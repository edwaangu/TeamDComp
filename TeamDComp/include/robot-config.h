using namespace vex;

extern brain Brain;

using signature = vision::signature;

// VEXcode devices
extern motor_group RightDriveMotors;
extern motor_group LeftDriveMotors;
extern motor FingerMotor;
extern motor ConveyorMotor;
extern controller Controller1;
extern inertial InertialSensor;
extern signature VisionSensor__RED_GOAL;
extern signature VisionSensor__BLUE_GOAL;
extern signature VisionSensor__SIG_3;
extern signature VisionSensor__SIG_4;
extern signature VisionSensor__SIG_5;
extern signature VisionSensor__SIG_6;
extern signature VisionSensor__SIG_7;
extern vision VisionSensor;
extern digital_out Pneumatic;
extern motor_group FlywheelMotor;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 * 
 * This should be called at the start of your int main function.
 */
void  vexcodeInit( void );