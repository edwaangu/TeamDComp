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
// RollMotor            motor         1               
// RightDriveMotors     motor_group   2, 3            
// LeftDriveMotors      motor_group   4, 5            
// FingerMotor          motor         6               
// ConveyorMotor        motor         7               
// FlywheelMotor        motor         8               
// Controller1          controller                    
// ---- END VEXCODE CONFIGURED DEVICES ----

/** IMPORTANT INFORMATION

Controls:

Axis 3 - Forward and back
Axis 1 - Turning left and right

Button A - Toggle Conveyor
Button X - Toggle Flywheel
Button Y - Start Finger Sequence

R1 and R2 - Roll Roller

*/

#include "vex.h"

using namespace vex;

/** Global instance of competition */
competition Competition;

/** VARIABLES **/
float minimumStick = 5;  // Minimum output from controller sticks
bool refreshScreenEveryX = false; // If true, it will refresh every "nextScreenRef" x 20 milliseconds.
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
int screenRefCount = 0; // Counter that counts up to the nextScreenRef amount
int nextScreenRef = 100; // The amount of 20 msec intervals to the next refresh

int lastBatteryAmt = 0; // Holds the value of the battery without needing to check the battery each time

// Button related
bool conveyorButtonPressed = false;
bool fingerButtonPressed = false;
bool flywheelButtonPressed = false;

/** SCREEN FUNCTIONS **/
void refreshScreen(){
  // Clear screen
  Controller1.Screen.clearScreen();

  // Set cursor to row 1, column 1
  Controller1.Screen.setCursor(1, 1);

  // Print whether or not the conveyor is on or off
  Controller1.Screen.print("CONV:");
  Controller1.Screen.print(conveyorOn ? "ON" : "OFF");

  // Print whether finger is active
  Controller1.Screen.print("FLY:");
  Controller1.Screen.print(flywheelOn ? "ON" : "OFF");

  // Set cursor to row 2, column 1
  Controller1.Screen.setCursor(2, 1);

  // Print brain battery amount
  Controller1.Screen.print("BATT: ");
  Controller1.Screen.print(lastBatteryAmt);
  Controller1.Screen.print("%");
  
  // Set cursor to row 3, column 1
  Controller1.Screen.setCursor(3, 1);

  // Print whether finger is busy
  Controller1.Screen.print("FINGER:");
  Controller1.Screen.print(fingerMode == 0 ? "LAZY" : "BUSY");
}

void setupScreen(){
  // Clear screen in-case something happens to be on it somehow
  Controller1.Screen.clearScreen();

  // Set-up battery capacity
  lastBatteryAmt = Brain.Battery.capacity();

  // Refresh screen
  refreshScreen();
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
  Controller1.Screen.print("AUTO STARTED");

  StartAutonomous();

  Controller1.Screen.clearScreen();
  Controller1.Screen.setCursor(1, 1);
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
  refreshScreen();
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
  refreshScreen();
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
    }
  }
}

/** USER CONTROL / RC **/

void usercontrol(void) {
  // User control code here, inside the loop
  while (1) {
    // Left/Right Motors
    if(abs(Controller1.Axis3.position(percent)) + abs(Controller1.Axis1.position(percent)) >= minimumStick){
      // Only move if controller sticks are more than the deadband (In case of controller drift)
      // Use dual stick mode, easier to go straight or control turns if necessary
      fwdBackSpd = Controller1.Axis3.position(percent);
      turnSpd = Controller1.Axis1.position(percent);

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
      if(refreshScreenEveryX){ // If the mode is to refresh every x amount of times
        refreshScreen(); // Run refresh screen function
      }
      else{ // If the mode is to refresh if the battery percentage changes
        if(lastBatteryAmt != Brain.Battery.capacity()){ // Check if battery % changed from last refresh
          refreshScreen(); // Refresh screen
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