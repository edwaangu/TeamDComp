#include "vex.h"

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 Brain screen
brain  Brain;

// VEXcode device constructors
motor RollMotor = motor(PORT1, ratio18_1, true);
motor RightDriveMotorsMotorA = motor(PORT2, ratio18_1, true);
motor RightDriveMotorsMotorB = motor(PORT3, ratio18_1, true);
motor_group RightDriveMotors = motor_group(RightDriveMotorsMotorA, RightDriveMotorsMotorB);
motor LeftDriveMotorsMotorA = motor(PORT4, ratio18_1, false);
motor LeftDriveMotorsMotorB = motor(PORT5, ratio18_1, false);
motor_group LeftDriveMotors = motor_group(LeftDriveMotorsMotorA, LeftDriveMotorsMotorB);
motor FingerMotor = motor(PORT6, ratio18_1, false);
motor ConveyorMotor = motor(PORT7, ratio18_1, false);
motor FlywheelMotor = motor(PORT8, ratio18_1, false);
controller Controller1 = controller(primary);

// VEXcode generated functions
// define variable for remote controller enable/disable
bool RemoteControlCodeEnabled = true;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 * 
 * This should be called at the start of your int main function.
 */
void vexcodeInit( void ) {
  // nothing to initialize
}