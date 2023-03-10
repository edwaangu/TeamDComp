#include "vex.h"

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 Brain screen
brain  Brain;

// VEXcode device constructors
motor RightDriveMotorsMotorA = motor(PORT18, ratio18_1, true);
motor RightDriveMotorsMotorB = motor(PORT17, ratio18_1, true);
motor_group RightDriveMotors = motor_group(RightDriveMotorsMotorA, RightDriveMotorsMotorB);
motor LeftDriveMotorsMotorA = motor(PORT20, ratio18_1, false);
motor LeftDriveMotorsMotorB = motor(PORT19, ratio18_1, false);
motor_group LeftDriveMotors = motor_group(LeftDriveMotorsMotorA, LeftDriveMotorsMotorB);
motor FingerMotor = motor(PORT11, ratio18_1, false);
motor ConveyorMotor = motor(PORT1, ratio36_1, false);
controller Controller1 = controller(primary);
inertial InertialSensor = inertial(PORT15);
/*vex-vision-config:begin*/
signature VisionSensor__RED_GOAL = signature (1, 1707, 10321, 6014, -1591, 85, -753, 0.7, 0);
signature VisionSensor__BLUE_GOAL = signature (2, -2979, -1157, -2068, 2883, 11253, 7068, 0.8, 0);
vision VisionSensor = vision (PORT21, 55, VisionSensor__RED_GOAL, VisionSensor__BLUE_GOAL);
/*vex-vision-config:end*/
digital_out Pneumatic = digital_out(Brain.ThreeWirePort.A);
motor FlywheelMotorMotorA = motor(PORT8, ratio6_1, false);
motor FlywheelMotorMotorB = motor(PORT9, ratio6_1, true);
motor_group FlywheelMotor = motor_group(FlywheelMotorMotorA, FlywheelMotorMotorB);

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