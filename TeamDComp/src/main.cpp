/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       C:\Users\Kepples                                          */
/*    Created:      Wed Jan 18 2023                                           */
/*    Description:  Main code for 29295D                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// RollMotor            motor         6               
// RightDriveMotors     motor_group   2, 3            
// LeftDriveMotors      motor_group   4, 5            
// FingerMotor          motor         20              
// ConveyorMotor        motor         1               
// FlywheelMotor        motor         12              
// Controller1          controller                    
// Pneumatic            digital_out   A               
// InertialSensor       inertial      13              
// ---- END VEXCODE CONFIGURED DEVICES ----

bool testingAutonomous = false; // IMPORTANT: CHANGE TO FALSE WHEN RUNNING COMPETITION
int autonomousMode = 5; // 1: THREE SQUARE, 2: TWO SQUARE
bool twoStickMode = true;
double maxSpeed = 0.9;

#pragma region INFORMATION

/** IMPORTANT INFORMATION

Controls:

Axis 3 - Forward and back
Axis 1 - Turning left and right

Button A - Toggle Intake
Button X - Toggle Flywheel
Button Y - Start Finger Sequence

Button UP - Increase Flywheel Speed by 5
Button DOWN - Decrease Flywheel Speed by 5

Button LEFT - Reverse Intake
Button RIGHT - Un-reverse Intake

R1 and R2 - Roll Roller


Controller Messages:

COOKING STARTED - Autonomous started
COOKING FINISHED - Autonomous finished with remaining time

MIX:
  R- at beginning - Intake is reversed
  MIXING - Intake is mixing disks
  MIXED - Intake has currently stopped mixing disk

TOAST:
  TOASTING - Flywheel is running
  TOASTED - Flywheel is not running

JUICE: Battery power

FINGER: Disk pusher
  LAZY - Disk pusher is not active
  BUSY - Disk pusher is in the middle of pushing or returning

TOASTER SPEED: Speed that flywheel is turning at (when on)


*/

#pragma endregion INFORMATION

#pragma region VARIABLES
#include "vex.h"

using namespace vex;

competition Competition;

float minimumStick = 5;  // Minimum output from controller sticks
bool refreshScreenEveryX = true; // If true, it will refresh every "nextScreenRef" x 20 milliseconds.
                                  // If false, it will refresh every battery % change (Checked every "nextScreenRef")
double turningCap = 1; // 0 to 1

// Temporary Variables for motors
double fwdBackSpd = 0;
double turnSpd = 0;
double leftSpd = 0;
double rightSpd = 0;

// Math Functions
int abs(int val){ // Convert integers to their absolute value
  return val < 0 ? -val : val;
}
double absDouble(double val){ // Convert integers to their absolute value
  return val < 0 ? -val : val;
}

bool conveyorOn = false; // Default to conveyor OFF, boolean that controls whether the conveyor is running
bool flywheelOn = false; // Default to flywheel OFF, boolean that controls whether the flywheeel is running
bool conveyorReversed = false;

int fingerMode = 0; // Finger modes:
/**
0 - Finger is not in motion
1 - Finger is moving to push the flywheel
2 - Finger is moving back to it's original position
*/

// Screen related
int screenRefCount = 300; // Counter that counts up to the nextScreenRef amount
int nextScreenRef = 200; // The amount of 20 msec intervals to the next refresh

int lastBatteryAmt = 0; // Holds the value of the battery without needing to check the battery each time
int screenInformationMode = 3;
/*
0 - Battery (JUICE) percentage
1 - Movement Motor (TRLR-L, TRLR-R) TEMPS
2 - Conveyor Motor (MIX) TEMPS + Flywheel Motor (TST) TEMPS
3 - Pusher Motor (FINGER) TEMPS + Roller Motor (ROLL) temps

*/


// Button related
bool conveyorButtonPressed = false;
bool fingerButtonPressed = false;
bool flywheelButtonPressed = false;

bool flywheelAdjustUpPressed = false;
bool flywheelAdjustDownPressed = false;

bool conveyorUnReverseButtonPressed = false;
bool conveyorReverseButtonPressed = false;

int flywheelAdjustedSpeed = 60;

#pragma endregion VARIABLES

#pragma region SCREEN_FUNCTIONS
void refreshScreen(bool updateRow1, bool updateRow2, bool updateRow3){
  
  // Clear screen
  //Controller1.Screen.clearScreen();

  if(updateRow1){
    // Set cursor to row 1, column 1
    Controller1.Screen.clearLine(1);
    Controller1.Screen.setCursor(1, 1);

    // Print whether or not the conveyor is on or off
    if(conveyorReversed){
      Controller1.Screen.print("R-");
    }
    Controller1.Screen.print("MIX");
    Controller1.Screen.print(conveyorOn ? "ING | " : "ED | ");

    // Print whether finger is active
    Controller1.Screen.print("TOAST");
    Controller1.Screen.print(flywheelOn ? "ING" : "ED");
  }

  if(updateRow2){
    // Set cursor to row 2, column 1
    Controller1.Screen.clearLine(2);
    Controller1.Screen.setCursor(2, 1);

    // Print brain battery amount
    if(screenInformationMode == 0){
      Controller1.Screen.print("JUICE: %d", lastBatteryAmt);
    }
    else if(screenInformationMode == 1){
      Controller1.Screen.print("TRLR-L: %.0f", LeftDriveMotors.temperature(percent));
      Controller1.Screen.print(" TRLR-R: %.0f", RightDriveMotors.temperature(percent));
    }
    else if(screenInformationMode == 2){
      Controller1.Screen.print("MIXER: %.0f", ConveyorMotor.temperature(percent));
      Controller1.Screen.print(" TOAST: %.0f", FlywheelMotor.temperature(percent));
    }
    else if(screenInformationMode == 3){
      Controller1.Screen.print("FINGER: %.0f", FingerMotor.temperature(percent));
      Controller1.Screen.print(" ROLL: %.0f", RollMotor.temperature(percent));
    }
  }
  
  if(updateRow3){
    // Set cursor to row 3, column 1
    Controller1.Screen.clearLine(3);
    Controller1.Screen.setCursor(3, 1);

    // Print whether finger is busy
    Controller1.Screen.print("FINGER:");
    Controller1.Screen.print(fingerMode == 0 ? "LAZY" : "BUSY");
  }
}

void setupScreen(){
  // Clear screen in-case something happens to be on it somehow
  Controller1.Screen.clearScreen();

  // Set-up battery capacity
  lastBatteryAmt = Brain.Battery.capacity();

  // Refresh screen
  refreshScreen(true, true, true);
}

#pragma endregion SCREEN_FUNCTIONS

#pragma region AUTONOMOUS_FUNCTIONS
void Move(double feet, int speed) { // Input in feet, speed in percent 0-100
  //input is feet, converts to inches, gets radians with arc length, converts to degrees
  float angle = ((feet * 12) / 2.125) * (180/3.14159);
  
  RightDriveMotors.setVelocity(speed, percent);
  RightDriveMotors.spinFor(forward, angle, deg, false);
  LeftDriveMotors.setVelocity(speed, percent);
  LeftDriveMotors.spinFor(forward, angle, deg, true);
}

void MovePID(double feet, int speed){ // Input in feet, speed in percent 0-100
  //input is feet, converts to inches, gets radians with arc length, converts to degrees
  float targetAngle = ((feet * 12) / 2.125) * (180/3.14159);
  RightDriveMotors.setPosition(0, degrees);
  LeftDriveMotors.setPosition(0, degrees);

  float Kp = 0;
  float Ki = 0;
  float Kd = 0;

  float error = 0;
  float lastError = 0;
  float integral = 0;
  float derivative = 0;

  float errors[480];
  int counter = 0;

  Brain.Screen.clearScreen();
  Brain.Screen.setPenColor(red);
  Brain.Screen.drawLine(0, 120, 479, 120);
  Brain.Screen.setPenColor(white);

  float targetSpeed = 0;

  // For some reason I thought PID was always for distance, turns out it apparently was only used for correcting motors
  while(LeftDriveMotors.position(degrees) < targetAngle){
    targetSpeed = targetAngle - LeftDriveMotors.position(degrees);
    if(targetSpeed > speed){
      targetSpeed = speed;
    }
    if(targetSpeed < -speed){
      targetSpeed = -speed;
    }
    if(targetSpeed < 5 && targetSpeed > -5){
      if(targetSpeed < 0){
        targetSpeed = -5;
      }
      else{
        targetSpeed = 5;
      }
    }

    error = LeftDriveMotors.position(degrees) - RightDriveMotors.position(degrees);

    integral = integral + error;
    derivative = error - lastError;

    LeftDriveMotors.setVelocity(targetSpeed, percent);
    RightDriveMotors.setVelocity(targetSpeed + (error * Kp) + (integral * Ki) + (derivative * Kd), percent);
    
    lastError = error + 0;

    errors[counter] = error;
    Brain.Screen.setPenColor(white);
    Brain.Screen.drawPixel(counter, 120+(error*5));
    Brain.Screen.setPenColor(blue);
    Brain.Screen.drawPixel(counter, 120+(integral));
    Brain.Screen.setPenColor(green);
    Brain.Screen.drawPixel(counter, 120+(derivative*5));
    counter ++;
    if(counter >= 480){
      counter = 479;
    }

    LeftDriveMotors.spin(forward);
    RightDriveMotors.spin(forward);

    counter ++;
    wait(20, msec);
  }

  LeftDriveMotors.stop();
  RightDriveMotors.stop();
}

void Turn(int angle, int speed) { // Positive angle spins clockwise?
  int angleAdjust = -angle * 3.6;
  RightDriveMotors.setVelocity(speed, percent);
  RightDriveMotors.spinFor(forward, angleAdjust, deg, false);
  LeftDriveMotors.setVelocity(speed, percent);
  LeftDriveMotors.spinFor(reverse, angleAdjust, deg, true);
}

void TurnA(int angle){ // Accepts any angle from 0 to 359.99, based on clockwise from starting position
  double turnToAngle = angle;
  if(turnToAngle >= 360){
    turnToAngle -= 360;
  }
  else if(turnToAngle < 0){
    turnToAngle += 360;
  }
  

  double turnSpeed = 0;
  double maxSpeed = 50;

  double InertialPlus = 0;
  double difference = turnToAngle - InertialSensor.heading(degrees);

  
  int counter = 0;
  while(counter < 20){
    difference = turnToAngle - (InertialSensor.heading(degrees) + InertialPlus);
    if(absDouble(difference - 360) < absDouble(difference)){
      InertialPlus += 360;
      difference = turnToAngle - (InertialSensor.heading(degrees) + InertialPlus);
    }
    if(absDouble(difference + 360) < absDouble(difference)){
      InertialPlus -= 360;
      difference = turnToAngle - (InertialSensor.heading(degrees) + InertialPlus);
    }

    turnSpeed = absDouble(difference) < 2 ? (difference < 0 ? -2 : 2) : difference;
    turnSpeed = turnSpeed > maxSpeed ? maxSpeed : (turnSpeed < -maxSpeed ? -maxSpeed : turnSpeed);
    
    RightDriveMotors.setVelocity(-turnSpeed, percent);
    RightDriveMotors.spin(forward);
    LeftDriveMotors.setVelocity(-turnSpeed, percent);
    LeftDriveMotors.spin(reverse);

    if(absDouble(difference) < 0.5){
      counter ++;
    }
    else{
      counter = 0;
    }

    wait(20, msec);
  }

  RightDriveMotors.stop();
  LeftDriveMotors.stop();
}

void TurnI(int angle){
  double turnToAngle = 180 + angle;
  if(turnToAngle >= 360){
    turnToAngle -= 360;
  }
  else if(turnToAngle < 0){
    turnToAngle += 360;
  }
  InertialSensor.setHeading(180, degrees);

  double turnSpeed = 0;
  double maxSpeed = 40;
  double difference = turnToAngle - InertialSensor.heading(degrees);

  int counter = 0;
  while(counter < 20){
    difference = turnToAngle - InertialSensor.heading(degrees);
    turnSpeed = absDouble(difference) < 2 ? (difference < 0 ? -2 : 2) : difference;
    turnSpeed = turnSpeed > maxSpeed ? maxSpeed : (turnSpeed < -maxSpeed ? -maxSpeed : turnSpeed);
    
    RightDriveMotors.setVelocity(-turnSpeed, percent);
    RightDriveMotors.spin(forward);
    LeftDriveMotors.setVelocity(-turnSpeed, percent);
    LeftDriveMotors.spin(reverse);

    if(absDouble(difference) < 0.5){
      counter ++;
    }
    else{
      counter = 0;
    }

    wait(20, msec);
  }

  RightDriveMotors.stop();
  LeftDriveMotors.stop();
}

void AdjustRoller(float angleAmount) { // Spins roller at 50 speed for X seconds
  RightDriveMotors.setVelocity(10, percent);
  RightDriveMotors.spin(reverse);
  LeftDriveMotors.setVelocity(10, percent);
  RightDriveMotors.spin(reverse);
  wait(0.3, seconds);
  RollMotor.setVelocity(100, percent);
  RollMotor.spinFor(reverse, angleAmount, degrees, true);
  RightDriveMotors.stop();
  LeftDriveMotors.stop();
}

void AdjustFlywheel(int speed){ // Flywheel motor will spin clockwise with a positive speed
  if(speed != 0){
    FlywheelMotor.setVelocity(speed, percent);
    FlywheelMotor.spin(forward);
  }
  else{
    FlywheelMotor.stop();
  }
}

void AdjustConveyor(int speed){ // Conveyor motor will spin clockwise with a positive speed
  if(speed != 0){
    ConveyorMotor.setVelocity(speed, percent);
    ConveyorMotor.spin(reverse);
  }
  else{
    ConveyorMotor.stop();
  }
}

void FingerActivate(){ // FINGER ACTIVATE
  
  FingerMotor.setStopping(hold);
  FingerMotor.setTimeout(1, seconds); // Do not hurt kid named finger :(
  //FingerMotor.setPosition(0, turns);

  while(FingerMotor.position(turns) < 0.03) {
    FingerMotor.setVelocity(50, percent);
    FingerMotor.spin(forward);
    wait(20, msec);
  }
  FingerMotor.stop();
  while(FingerMotor.position(turns) >= 0){
    FingerMotor.setVelocity(50, percent);
    FingerMotor.spin(reverse);
    wait(20, msec);
  }
  FingerMotor.stop();
  //wait(300, msec); // Give finger a moment
}

#pragma endregion AUTONOMOUS_FUNCTIONS

#pragma region AUTONOMOUS
/** PRE AUTONOMOUS **/
void pre_auton(void) {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();
  FingerMotor.setPosition(0, turns); 
  //setupScreen();

  return;
}

/** AUTONOMOUS **/
void StartAutonomous(int mode){ // All autonomous actions should happen here
  LeftDriveMotors.setTimeout(2, seconds);
  RightDriveMotors.setTimeout(2, seconds);
  if(mode == 1){ // Two-Square Start Plan (TWO DISKS SHOT)
    AdjustFlywheel(75);

    // Two square start
    Move(0.65, 40);
    Turn(90, 40);
    Move(1.9, 40);
    Turn(-85, 40);

    wait(1, seconds);

    FingerActivate();
    FingerActivate();

    // Roll roller
    Move(-0.35, 40);
    AdjustRoller(90);

    AdjustFlywheel(0);
    AdjustConveyor(0);

  }
  if(mode == 2){ // Two-Square Boosted Plan (FIVE DISKS SHOT)
    AdjustFlywheel(74);

    // Two square start
    Move(0.65, 40);
    Turn(90, 40);
    Move(1.9, 40);
    Turn(-85, 40);

    wait(1, seconds);

    FingerActivate();
    FingerActivate();

    // Roll roller
    Move(-0.35, 40);
    AdjustRoller(90);

    // Move towards other discs and pick them up
    Move(0.35, 40);
    Turn(90, 40);
    Move(-0.25, 40);
    Turn(45, 40);
    AdjustConveyor(100);
    Move(-7.5, 50);

    // Turn towards goal and shoot again
    AdjustFlywheel(60);
    wait(2, seconds);
    Turn(-90, 40);
    FingerActivate();
    FingerActivate();
    FingerActivate();

    AdjustFlywheel(0);
    AdjustConveyor(0);

  }
  if(mode == 3){ // Three-Square Start Plan (TWO DISKS SHOT)
    Move(-0.1, 40); 
    AdjustRoller(90);
    AdjustFlywheel(69);
    wait(4, seconds);
    FingerActivate();
    wait(1, seconds);
    FingerActivate();
    AdjustFlywheel(0);
  }
  if(mode == 4){ // Skills Plan (TEN DISKS SHOT)
    // Move backwards into roller slowly and spin roller
    Move(-0.1, 40); 
    AdjustRoller(180);
    AdjustFlywheel(69);

    // Pick up disc that is on autonomous line
    Move(1.5, 40);
    Turn(135, 40);
    AdjustConveyor(100);
    Move(-1.5, 40);
    Turn(-45, 40);

    // Move backwards into 2nd roller slowly and spin roller
    AdjustConveyor(0);
    Move(-1.5, 40);
    AdjustRoller(180);

    // Move forwards slightly and shoot three disks into the blue basket
    Move(0.5, 40);
    FingerActivate();
    FingerActivate();
    FingerActivate();

    // Progress towards picking up next three discs
    Turn(90, 40);
    Move(-1, 40);
    Turn(45, 40);
    AdjustConveyor(100);
    AdjustFlywheel(40);

    // Pick up three disks
    Move(-5.65, 40);

    // Turn towards red basket and shoot three disks into there
    Turn(90, 40);
    AdjustConveyor(0);
    FingerActivate();
    FingerActivate();
    FingerActivate();

    // Move backwards into next set of three discs
    AdjustConveyor(100);
    Move(-2.82, 40);
    Turn(-90, 40);
    Move(-5.65, 40);
    
    // Move towards next roller
    Turn(-45, 40);
    Move(-0.75, 40);
    Turn(90, 40);
    
    // Move backwards into 3rd roller slowly and spin roller
    AdjustConveyor(0);
    AdjustFlywheel(69);
    Move(-1.5, 40);
    AdjustRoller(180);

    // Move forwards slightly and shoot three disks into the red basket
    Move(1, 40);
    FingerActivate();
    FingerActivate();
    FingerActivate();

    // Move towards final disc and shoot it
    AdjustConveyor(100);
    Turn(-135, 40);
    Move(-1.5, 40);
    Turn(135, 40);
    FingerActivate();
    AdjustFlywheel(0);
    AdjustConveyor(0);
    Turn(-90, 40);

    // Move backwards into final roller slowly and spin roller
    Move(-1.5, 40);
    AdjustRoller(180);

  }
}


void autonomous(void) {

  Controller1.Screen.clearScreen();
  Controller1.Screen.setCursor(1, 1);
  Controller1.Screen.print("COOKING STARTED");

  StartAutonomous(autonomousMode);

  Controller1.Screen.clearScreen();
  Controller1.Screen.setCursor(1, 1);
  Controller1.Screen.print("COOKING");
  Controller1.Screen.setCursor(2, 1);
  Controller1.Screen.print("FINISHED");
}

#pragma endregion AUTONOMOUS

#pragma region RC_FUNCTIONS

void updateConveyor(){ // To toggle the conveyor
  
  // Update conveyor status in-function to prevent wasted computations
  if(conveyorOn){
    // Set conveyor motor to full power (Because why wouldn't you)
    ConveyorMotor.setVelocity(100, percent);
    if(conveyorReversed){
      ConveyorMotor.spin(forward);
    }
    else{
      ConveyorMotor.spin(reverse);
    }
  }
  else{
    // Stop conveyor motor
    ConveyorMotor.stop();
  }

  // Update screen to refresh whether or not "Conveyor: " says "ON" or "OFF"
  refreshScreen(true, false, false);
}

void toggleConveyor(){ // To toggle the conveyor
  // Update the bool controlling whether or not the conveyor is on
  conveyorOn = !conveyorOn;
  
  updateConveyor();
}


void toggleFlywheel(int thespeed){ // To toggle the flywheel
  // Update the bool controlling whether or not the conveyor is on
  flywheelOn = !flywheelOn;
  
  // Update conveyor status in-function to prevent wasted computations
  if(flywheelOn){
    // Set conveyor motor to full power (Because why wouldn't you)
    FlywheelMotor.setVelocity(thespeed, percent);
    FlywheelMotor.spin(forward);
  }
  else{
    // Stop conveyor motor
    FlywheelMotor.stop();
  }

  // Update screen to refresh whether or not "Conveyor: " says "ON" or "OFF"
  refreshScreen(true, false, false);
}

void updateFinger(){
  FingerMotor.setStopping(hold);
  FingerMotor.setTimeout(1, seconds); // Do not hurt kid named finger :(
  if(fingerMode == 1){ // FINGER OUT!
    FingerMotor.setVelocity(50, percent);
    FingerMotor.spin(forward);
    if(FingerMotor.position(turns) >= 0.02){
      FingerMotor.stop();
      fingerMode = 2;
    }
  }
  else if(fingerMode == 2){ // FINGER RETURN!
    FingerMotor.setVelocity(50, percent);
    FingerMotor.spin(reverse);
    if(FingerMotor.position(turns) < 0){
      FingerMotor.stop();
      fingerMode = 0;
    }
  }
}

void updateFlywheelSpeed(int speedUpdate){
  flywheelAdjustedSpeed +=speedUpdate;
  if(flywheelAdjustedSpeed > 100){
    flywheelAdjustedSpeed = 100;
  }
  if(flywheelAdjustedSpeed < 5){
    flywheelAdjustedSpeed = 5;
  }
  if(flywheelOn){
    // Set conveyor motor to full power (Because why wouldn't you)
    FlywheelMotor.setVelocity(flywheelAdjustedSpeed, percent);
    FlywheelMotor.spin(forward);
  }
  else{
    // Stop conveyor motor
    FlywheelMotor.stop();
  }

  Controller1.Screen.clearScreen();
  Controller1.Screen.setCursor(1, 1);
  Controller1.Screen.print("TOAST SPEED: %d", flywheelAdjustedSpeed);

  screenRefCount = 0;
  nextScreenRef = 100;

}

#pragma endregion RC_FUNCTIONS

#pragma region RC

void usercontrol(void) {
  // User control code here, inside the loop
  setupScreen();
  while (1) {
    int turnAxis = twoStickMode ? Controller1.Axis1.position(percent) : Controller1.Axis4.position(percent);
    // Left/Right Motors
    if(abs(Controller1.Axis3.position(percent)) + abs(turnAxis) >= minimumStick){
      // Only move if controller sticks are more than the deadband (In case of controller drift)
      // Use dual stick mode, easier to go straight or control turns if necessary
      fwdBackSpd = Controller1.Axis3.position(percent);
      turnSpd = -turnAxis * 0.85;

      leftSpd = fwdBackSpd - turnSpd;
      rightSpd = fwdBackSpd + turnSpd;

      leftSpd *= maxSpeed;
      rightSpd *= maxSpeed;
      
      // Set velocities of motors
      LeftDriveMotors.setVelocity(leftSpd, percent);
      RightDriveMotors.setVelocity(rightSpd, percent);
      LeftDriveMotors.spin(forward);
      RightDriveMotors.spin(forward);
      //FlywheelMotor.setVelocity(100, percent);
    }
    else{
      // Stop motors
      LeftDriveMotors.stop();
      RightDriveMotors.stop();
      //FlywheelMotor.stop();
    }


    // Conveyor - Run the "toggleConveyor" function every time the A button is pressed
    if(!conveyorButtonPressed && Controller1.ButtonA.pressing()){
      toggleConveyor();
      conveyorButtonPressed = true;
    }
    if(conveyorButtonPressed && !Controller1.ButtonA.pressing()){
      conveyorButtonPressed = false;
    }

    // Conveyor - Update whether conveyor is reversed or not
    if(!conveyorReverseButtonPressed && Controller1.ButtonLeft.pressing()){
      conveyorReversed = true;
      updateConveyor();
      conveyorReverseButtonPressed = true;
    }
    if(!conveyorUnReverseButtonPressed && Controller1.ButtonRight.pressing()){
      conveyorReversed = false;
      updateConveyor();
      conveyorUnReverseButtonPressed = true;
    }
    
    if(conveyorReverseButtonPressed && !Controller1.ButtonLeft.pressing()){
      conveyorReverseButtonPressed = false;
    }
    if(conveyorUnReverseButtonPressed && !Controller1.ButtonRight.pressing()){
      conveyorUnReverseButtonPressed = false;
    }

    // Flywheel - Run the "toggleFlywheel" function every time the A button is pressed
    if(!flywheelButtonPressed && Controller1.ButtonX.pressing()){
      toggleFlywheel(flywheelAdjustedSpeed);
      flywheelButtonPressed = true;
    }
    if(flywheelButtonPressed && !Controller1.ButtonX.pressing()){
      flywheelButtonPressed = false;
    }
    
    // Flywheel - increase speed when the UP button is pressed
    if(!flywheelAdjustUpPressed && Controller1.ButtonUp.pressing()){
      updateFlywheelSpeed(5);
      flywheelAdjustUpPressed = true;
    }
    if(flywheelAdjustUpPressed && !Controller1.ButtonUp.pressing()){
      flywheelAdjustUpPressed = false;
    }
    
    // Flywheel - decrease speed when the DOWN button is pressed
    if(!flywheelAdjustDownPressed && Controller1.ButtonDown.pressing()){
      updateFlywheelSpeed(-5);
      flywheelAdjustDownPressed = true;
    }
    if(flywheelAdjustDownPressed && !Controller1.ButtonDown.pressing()){
      flywheelAdjustDownPressed = false;
    }

    // Roller - roll based on L1 and R1
    if(Controller1.ButtonL1.pressing()){
      RollMotor.setVelocity(100, percent);
      RollMotor.spin(reverse);
    }
    else if(Controller1.ButtonR1.pressing()){
      RollMotor.setVelocity(100, percent);
      RollMotor.spin(forward);
    }
    else{
      RollMotor.stop();
    }

    // Finger - Update "fingerMode" to 1 to start finger sequence
    
    if(Controller1.ButtonY.pressing()){
      if(fingerMode == 0){ // Only start finger sequence when finger sequence is not running
        fingerMode = 1;
        refreshScreen(false, false, true);
      }
      fingerButtonPressed = true;
    }
    if(fingerButtonPressed && !Controller1.ButtonY.pressing()){
      fingerButtonPressed = false;
      refreshScreen(false, false, true);
    }
    Brain.Screen.clearScreen();
    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print(FlywheelMotor.velocity(percent));
    updateFinger();

    // Expansion
    if(Controller1.ButtonB.pressing()) {
      Pneumatic.set(true);
    }
    else{
      Pneumatic.set(false);
    }

    // Screen Refresh every x msec (Not every check unless you want to REALLY lag the brain/controller)
    screenRefCount +=1;
    if(screenRefCount >= nextScreenRef){ // Have we reached the next screen refresh?
      screenRefCount = 0; // Reset counter
      
      if(nextScreenRef == 100){
        nextScreenRef = 200;
        refreshScreen(true, true, true); // Run refresh screen function
        lastBatteryAmt = Brain.Battery.capacity(); // Update "lastBatteryAmt" to remember what the battery % was last
      }
      else{
        screenInformationMode ++;
        if(screenInformationMode > 3){
          screenInformationMode = 0;
        }
        if(refreshScreenEveryX){ // If the mode is to refresh every x amount of times
          refreshScreen(false, true, false); // Run refresh screen function
          lastBatteryAmt = Brain.Battery.capacity(); // Update "lastBatteryAmt" to remember what the battery % was last
        }
        else{ // If the mode is to refresh if the battery percentage changes
          if(lastBatteryAmt != Brain.Battery.capacity()){ // Check if battery % changed from last refresh
            refreshScreen(false, true, false); // Refresh screen
            lastBatteryAmt = Brain.Battery.capacity(); // Update "lastBatteryAmt" to remember what the battery % was last
          }
        }
      }
    }
    //

    // Wait
    wait(20, msec); // We are anti-wasters.
  }
}

#pragma endregion RC

#pragma region MAIN
int main() {
  // Set up callbacks for autonomous and driver control periods.
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);

  // Run the pre-autonomous function.
  pre_auton();

  // Prevent main from exiting with an infinite loop.
  while (true) {
    wait(100, msec);
  }
}
#pragma endregion MAIN