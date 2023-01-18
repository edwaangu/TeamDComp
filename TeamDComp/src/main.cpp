/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       C:\Users\Kepples                                          */
/*    Created:      Wed Jan 18 2023                                           */
/*    Description:  Main code for 29295D                                                */
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

int abs(int val){ // Convert integers to their absolute value
  return val;
}

bool conveyorOn = false; // Default to conveyor OFF, boolean that controls whether the conveyor is running

int screenRefCount = 0; // Counter that counts up to the nextScreenRef amount
int nextScreenRef = 100; // The amount of 20 msec intervals to the next refresh

int lastBatteryAmt = 0; // Holds the value of the battery without needing to check the battery each time

/** FUNCTIONS **/
void refreshScreen(){
  // Clear screen
  Controller1.Screen.clearScreen();

  // Set cursor to row 1, column 1
  Controller1.Screen.setCursor(1, 1);

  // Print whether or not the conveyor is on or off
  Controller1.Screen.print("Conveyor:");
  Controller1.Screen.print(conveyorOn ? "ON" : "OFF");

  // Set cursor to row 2, column 2
  Controller1.Screen.setCursor(2, 1);

  // Print brain battery amount
  Controller1.Screen.print("Battery: ");
  Controller1.Screen.print(lastBatteryAmt);
}

void setupScreen(){
  // Clear screen in-case something happens to be on it somehow
  Controller1.Screen.clearScreen();

  // Set-up battery capacity
  lastBatteryAmt = Brain.Battery.capacity();

  // Refresh screen
  refreshScreen();
}

void toggleConveyor(){
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

/** PRE AUTONOMOUS **/
void pre_auton(void) {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();
  setupScreen();

  return;
}

/** AUTONOMOUS **/
void autonomous(void) {

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
      FlywheelMotor.setVelocity(100, percent);
    }
    else{
      // Stop motors
      LeftDriveMotors.stop();
      RightDriveMotors.stop();
      FlywheelMotor.stop();
    }


    // Conveyor
    Controller1.ButtonA.pressed(toggleConveyor); // Run the "toggleConveyor" function every time the A button is pressed

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