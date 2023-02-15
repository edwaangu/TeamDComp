#include "vex.h"

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 Brain screen
brain  Brain;

// VEXcode device constructors
motor RollMotor = motor(PORT2, ratio18_1, true);
motor RightDriveMotorsMotorA = motor(PORT17, ratio18_1, true);
motor RightDriveMotorsMotorB = motor(PORT20, ratio18_1, true);
motor_group RightDriveMotors = motor_group(RightDriveMotorsMotorA, RightDriveMotorsMotorB);
motor LeftDriveMotorsMotorA = motor(PORT18, ratio18_1, false);
motor LeftDriveMotorsMotorB = motor(PORT19, ratio18_1, false);
motor_group LeftDriveMotors = motor_group(LeftDriveMotorsMotorA, LeftDriveMotorsMotorB);
motor FingerMotor = motor(PORT5, ratio6_1, false);
motor ConveyorMotor = motor(PORT1, ratio36_1, false);
motor FlywheelMotor = motor(PORT11, ratio18_1, false);
controller Controller1 = controller(primary);
digital_out Pneumatic = digital_out(Brain.ThreeWirePort.H);
inertial InertialSensor = inertial(PORT9);
/*vex-vision-config:begin*/
signature VisionSensor__RED_GOAL = signature (1, 1707, 10321, 6014, -1591, 85, -753, 0.7, 0);
signature VisionSensor__BLUE_GOAL = signature (2, -2979, -1157, -2068, 2883, 11253, 7068, 0.8, 0);
vision VisionSensor = vision (PORT21, 55, VisionSensor__RED_GOAL, VisionSensor__BLUE_GOAL);
/*vex-vision-config:end*/

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