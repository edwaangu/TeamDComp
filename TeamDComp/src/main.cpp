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
// RollMotor            motor         21              
// RightDriveMotors     motor_group   2, 3            
// LeftDriveMotors      motor_group   4, 5            
// FingerMotor          motor         20              
// ConveyorMotor        motor         1               
// FlywheelMotor        motor         12              
// Controller1          controller                    
// ---- END VEXCODE CONFIGURED DEVICES ----

bool testingAutonomous = false; // IMPORTANT: CHANGE TO FALSE WHEN RUNNING COMPETITION

/** IMPORTANT INFORMATION

Controls:

Axis 3 - Forward and back
Axis 1 - Turning left and right

Button A - Toggle Conveyor
Button X - Toggle Flywheel
Button Y - Start Finger Sequence

R1 and R2 - Roll Roller


Controller Messages:

COOKING STARTED - Autonomous started
COOKING FINISHED - Autonomous finished with remaining time

MIX:
  MIXING - Conveyor is mixing disks
  MIXED - Conveyor has currently stopped mixing disk

TOAST:
  TOASTING - Flywheel is running
  TOASTED - Flywheel is not running

JUICE: Battery power

FINGER: Disk pusher
  LAZY - Disk pusher is not active
  BUSY - Disk pusher is in the middle of pushing or returning

Motor Temperature Guide:
 0 - 25% - BREAD
 25 - 50% - TOAST
 50 - 75% - TOASTED
 75 - 100% - BURNTED


*/

#include "vex.h"

using namespace vex;

/** Global instance of competition */
competition Competition;

/** VARIABLES **/
float minimumStick = 5;  // Minimum output from controller sticks
bool refreshScreenEveryX = true; // If true, it will refresh every "nextScreenRef" x 20 milliseconds.
                                  // If false, it will refresh every battery % change (Checked every "nextScreenRef")
double turningCap = 1; // 0 to 1

// Temporary Variables for motors
double fwdBackSpd = 0;
double turnSpd = 0;
double leftSpd = 0;
double rightSpd = 0;

// Math Function
int abs(int val){ // Convert integers to their absolute value
  return val;
}

bool conveyorOn = false; // Default to conveyor OFF, boolean that controls whether the conveyor is running
bool flywheelOn = false; // Default to flywheel OFF, boolean that controls whether the flywheeel is running

int fingerMode = 0; // Finger modes:
/**
0 - Finger is not in motion
1 - Finger is moving to push the flywheel
2 - Finger is moving back to it's original position
*/

// Screen related
int screenRefCount = 300; // Counter that counts up to the nextScreenRef amount
int nextScreenRef = 300; // The amount of 20 msec intervals to the next refresh

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

/** SCREEN FUNCTIONS **/
void refreshScreen(bool updateRow1, bool updateRow2, bool updateRow3){
  
  // Clear screen
  //Controller1.Screen.clearScreen();

  if(updateRow1){
    // Set cursor to row 1, column 1
    Controller1.Screen.clearLine(1);
    Controller1.Screen.setCursor(1, 1);

    // Print whether or not the conveyor is on or off
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

/** AUTONOMOUS FUNCTIONS **/
void Move(int feet, int speed) { // Input in feet, speed in percent 0-100
  //input is feet, converts to inches, gets radians with arc length, converts to degrees
  float angle = ((feet * 12) / 2.125) * (180/3.14159);
  
  RightDriveMotors.setVelocity(speed, percent);
  RightDriveMotors.spinFor(forward, angle, deg, false);
  LeftDriveMotors.setVelocity(speed, percent);
  LeftDriveMotors.spinFor(forward, angle, deg, true);
}

void Turn(int angle, int speed) { // Positive angle spins clockwise?
  int angleAdjust = angle * 3.6;
  RightDriveMotors.setVelocity(speed, percent);
  RightDriveMotors.spinFor(forward, angleAdjust, deg, false);
  LeftDriveMotors.setVelocity(speed, percent);
  LeftDriveMotors.spinFor(reverse, angleAdjust, deg, true);

}

void AdjustRoller(float timeSpin) { // Spins roller at 50 speed for X seconds
  RollMotor.setVelocity(50, percent);
  RollMotor.spinFor(forward, timeSpin, seconds);
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
    ConveyorMotor.spin(forward);
  }
  else{
    ConveyorMotor.stop();
  }
}

void FingerActivate(int angle){ // Rotates Finger mechanism
  FingerMotor.setStopping(hold);
  FingerMotor.setTimeout(1, seconds);
  FingerMotor.setVelocity(40, percent);
  FingerMotor.spinFor(forward, angle, deg, true);
}

/** PRE AUTONOMOUS **/
void pre_auton(void) {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();
  //setupScreen();

  return;
}

/** AUTONOMOUS **/
void StartAutonomous(){ // All autonomous actions should happen here
  //turn(45, 20);
  //roller(3);
}

void autonomous(void) {

  Controller1.Screen.clearScreen();
  Controller1.Screen.setCursor(1, 1);
  Controller1.Screen.print("COOKING STARTED");

  StartAutonomous();

  Controller1.Screen.clearScreen();
  Controller1.Screen.setCursor(1, 1);
  Controller1.Screen.print("COOKING");
  Controller1.Screen.setCursor(2, 1);
  Controller1.Screen.print("FINISHED");
}

/** USER CONTROL / RC FUNCTIONS **/
void toggleConveyor(){ // To toggle the conveyor
  // Update the bool controlling whether or not the conveyor is on
  conveyorOn = !conveyorOn;
  
  // Update conveyor status in-function to prevent wasted computations
  if(conveyorOn){
    // Set conveyor motor to full power (Because why wouldn't you)
    ConveyorMotor.setVelocity(100, percent);
    ConveyorMotor.spin(reverse);
  }
  else{
    // Stop conveyor motor
    ConveyorMotor.stop();
  }

  // Update screen to refresh whether or not "Conveyor: " says "ON" or "OFF"
  refreshScreen(true, false, false);
}

void toggleFlywheel(){ // To toggle the flywheel
  // Update the bool controlling whether or not the conveyor is on
  flywheelOn = !flywheelOn;
  
  // Update conveyor status in-function to prevent wasted computations
  if(flywheelOn){
    // Set conveyor motor to full power (Because why wouldn't you)
    FlywheelMotor.setVelocity(100, percent);
    FlywheelMotor.spin(reverse);
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
    FingerMotor.setVelocity(30, percent);
    FingerMotor.spin(forward);
    if(FingerMotor.position(turns) >= 1){
      FingerMotor.stop();
      fingerMode = 2;
    }
  }
  else if(fingerMode == 2){ // FINGER RETURN!
    FingerMotor.setVelocity(30, percent);
    FingerMotor.spin(reverse);
    if(FingerMotor.position(turns) <= 0.1){
      FingerMotor.stop();
      fingerMode = 0;
      refreshScreen(false, false, true);
    }
  }
}

/** USER CONTROL / RC **/

void usercontrol(void) {
  // User control code here, inside the loop
  setupScreen();
  while (1) {
    // Left/Right Motors
    if(abs(Controller1.Axis3.position(percent)) + abs(Controller1.Axis1.position(percent)) >= minimumStick){
      // Only move if controller sticks are more than the deadband (In case of controller drift)
      // Use dual stick mode, easier to go straight or control turns if necessary
      fwdBackSpd = Controller1.Axis3.position(percent);
      turnSpd = -Controller1.Axis1.position(percent);

      leftSpd = fwdBackSpd - turnSpd;
      rightSpd = fwdBackSpd + turnSpd;
      
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

    // Flywheel - Run the "toggleFlywheel" function every time the A button is pressed
    if(!flywheelButtonPressed && Controller1.ButtonX.pressing()){
      toggleFlywheel();
      flywheelButtonPressed = true;
    }
    if(flywheelButtonPressed && !Controller1.ButtonX.pressing()){
      flywheelButtonPressed = false;
    }

    // Finger - Update "fingerMode" to 1 to start finger sequence
    if(!fingerButtonPressed && Controller1.ButtonY.pressing()){
      if(fingerMode == 0){ // Only start finger sequence when finger sequence is not running
        fingerMode = 1;
        refreshScreen(false, false, true);
        FingerMotor.setPosition(0, degrees);
      }
      fingerButtonPressed = true;
    }
    if(fingerButtonPressed && !Controller1.ButtonY.pressing()){
      fingerButtonPressed = false;
    }

    updateFinger();

    // Screen Refresh every x msec (Not every check unless you want to REALLY lag the brain/controller)
    screenRefCount +=1;
    if(screenRefCount >= nextScreenRef){ // Have we reached the next screen refresh?
      screenRefCount = 0; // Reset counter
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

    // Wait
    wait(20, msec); // We are anti-wasters.
  }
}

//
// Main will set up the competition functions and callbacks.
//
int main() {
  if(testingAutonomous){
    autonomous();
  }
  else{
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
}