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
// RightDriveMotors     motor_group   18, 17          
// LeftDriveMotors      motor_group   20, 19          
// FingerMotor          motor         11              
// ConveyorMotor        motor         1               
// Controller1          controller                    
// InertialSensor       inertial      15              
// VisionSensor         vision        21              
// Pneumatic            digital_out   A               
// FlywheelMotor        motor_group   8, 9            
// ---- END VEXCODE CONFIGURED DEVICES ----

bool testingAutonomous = false; // IMPORTANT: CHANGE TO FALSE WHEN RUNNING COMPETITION
int autonomousMode = 2; // 1: THREE SQUARE, 2: TWO SQUARE
bool twoStickMode = true;
double maxSpeed = 0.9;
bool runningSkills = false;

#pragma region INFORMATION

/** IMPORTANT INFORMATION

Controls:

Axis 3 - Forward and back
Axis 1 - Turning left and right

Button L1 - Toggle Intake
Button X - Toggle Flywheel
Button Y - Start Finger Sequence

Button UP - Increase Flywheel Speed by 5
Button DOWN - Decrease Flywheel Speed by 5

Button LEFT - Reverse Intake
Button RIGHT - Un-reverse Intake

L2 and R2 - Roll Roller in opposite directions

Button B - Launch Pneumatics
> Combine with L2 and R2 to launch before time is up


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
#include <string>
#include <list>
#include <array>
#include <iostream>

#pragma region FUNNY

const int amtOfImages = 26;

int waltMode = 0;

struct fullImage {
  std::string importedImage;
  std::array<color,35> colorsUsed = {};

  std::list<int> colorAmts;
  std::list<int> colorValues;
};

std::array<fullImage,amtOfImages> fullImages;





int findIndexInString(std::string _str, char _index)
{
    for(int i = 0;i < _str.length();i++)
    {
        if(_index == _str[i])
        {
            return i;
        }
    }
    return -1;
}

int hex2ToValue(std::string _x)
{
    std::string vals = "0123456789ABCDEF";

    int returnVal = 0;
    returnVal += findIndexInString(vals, _x[0]) * 16;
    returnVal += findIndexInString(vals, _x[1]);

    return returnVal;
}

int pow(int num, int p){
  int newNum = 1;
  for(int i = 0;i < p;i ++){
    newNum *=num;
  }
  return newNum;
}

int otherToValue(std::string _x, std::string otherString, int maxLength)
{
    std::string vals = otherString;

    int returnVal = 0;
    for (int i = 0; i < maxLength; i++)
    {
        returnVal += findIndexInString(vals, _x[i]) * pow(otherString.length(), maxLength - i - 1);
    }

    return returnVal;
}

color hexToColor(std::string _hex)
{
    std::string r = "";
    r += _hex[0];
    r += _hex[1];
    std::string g = "";
    g += _hex[2];
    g += _hex[3];
    std::string b = "";
    b += _hex[4];
    b += _hex[5];
    return color(hex2ToValue(r), hex2ToValue(g), hex2ToValue(b));
}

int chartoi(char thechar){
  switch(thechar){
    case '0':
      return 0;
      break;
    case '1':
      return 1;
      break;
    case '2':
      return 2;
      break;
    case '3':
      return 3;
      break;
    case '4':
      return 4;
      break;
    case '5':
      return 5;
      break;
    case '6':
      return 6;
      break;
    case '7':
      return 7;
      break;
    case '8':
      return 8;
      break;
    case '9':
      return 9;
      break;
  }
  return -1;
}

int stoi(std::string strToConvert){
  int lengthOfStr = strToConvert.length();
  int returnedNumber = 0;
  for(int i = 0;i < lengthOfStr;i ++){
    returnedNumber += chartoi(strToConvert[i]) * pow(10, lengthOfStr-i-1);
  }
  return returnedNumber;
}


void importToImage(int id){
  fullImages[id].colorAmts.clear();
  fullImages[id].colorValues.clear();

  bool isInColorMode = true;
  int z = 0; // index in imported Image

  int alphaPower = 0;
  std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  std::string theTemp = "";
  int theColorAt = 0;
  while (isInColorMode)
  {
      if(alphaPower == 0)
      {
          while (fullImages[id].importedImage[z] != '#')
          {
              z++;
          }
          alphaPower = z + 0;
          z++;
      }
      else
      {
          z += alphaPower + 1;
      }
      theTemp = fullImages[id].importedImage[z];
      for(int i = 1;i < 6;i++)
      {
          theTemp += fullImages[id].importedImage[z + i];
      }
      fullImages[id].colorsUsed.at(theColorAt) = hexToColor(theTemp);
      if (fullImages[id].importedImage[z+6] == ';')
      {
          isInColorMode = false;
      }
      z += 7;

      theColorAt ++;
  }


  while(z < fullImages[id].importedImage.length())
  {
      std::string foundColor = "";
      for(int i = 0;i < alphaPower;i++)
      {
          foundColor += fullImages[id].importedImage[z + i];
      }

      int foundColorVal = otherToValue(foundColor, alphabet, alphaPower);

      z += alphaPower + 1;
      std::string num = "";
      while (fullImages[id].importedImage[z] != '-' && fullImages[id].importedImage[z] != '.')
      {
          num += fullImages[id].importedImage[z];
          z++;
      }
      int theNum = stoi(num);

      fullImages[id].colorAmts.push_back(theNum);
      fullImages[id].colorValues.push_back(foundColorVal);

      z++;
  }
}

  int curImage = 0;

  void drawCurrentImage(){

    int width = 60;

    int x = 0;
    int y = 0;
    // std::list<int>::iterator it=mylist.begin(); it != mylist.end(); ++it
    std::list<int>::iterator itV=fullImages[curImage].colorValues.begin();
    for(std::list<int>::iterator it=fullImages[curImage].colorAmts.begin(); it != fullImages[curImage].colorAmts.end();it++)
    {
        Brain.Screen.setFillColor(fullImages[curImage].colorsUsed[*itV]);
        int addX = *it;
        if(x + addX >= width)
        {
            int cutoff = width - x;
            Brain.Screen.drawRectangle(x*8, y*8, cutoff*8, 8);
            addX -= cutoff;
            x = 0;
            y++;
        }
        Brain.Screen.drawRectangle(x*8, y*8, addX*8, 8);
        x += addX;

        itV ++;

        //wait(5, msec);
    }
  }

#pragma endregion FUNNY

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

double maxAccel = 5;
double accelLeftSpd = 0;
double accelRightSpd = 0;

// Math Functions
int abs(int val){ // Convert integers to their absolute value
  return val < 0 ? -val : val;
}

double absDouble(double val){ // Convert integers to their absolute value
  return val < 0 ? -val : val;
}

float absFloat(float val){ // Convert integers to their absolute value
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
int nextScreenRef = 100; // The amount of 20 msec intervals to the next refresh

int lastBatteryAmt = 0; // Holds the value of the battery without needing to check the battery each time
int screenInformationMode = 2;
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
      Controller1.Screen.print("TRLR: %.0f", LeftDriveMotors.temperature(percent));
      Controller1.Screen.print(" FINGER: %.0f", FingerMotor.temperature(percent));
    }
    else if(screenInformationMode == 1){
      Controller1.Screen.print("MIXER: %.0f", ConveyorMotor.temperature(percent));
      Controller1.Screen.print(" TOAST: %.0f", FlywheelMotor.temperature(percent));
    }
    else if(screenInformationMode == 2){
      Controller1.Screen.print("EST. TIME: %.0f", Brain.Timer.value());

    }
  }
  
  if(updateRow3){
    // Set cursor to row 3, column 1
    Controller1.Screen.clearLine(3);
    Controller1.Screen.setCursor(3, 1);

    // Print whether finger is busy
    Controller1.Screen.print("FINGER:");
    Controller1.Screen.print(fingerMode == 0 ? "LAZY " : "BUSY ");

    
    Controller1.Screen.print("%d", lastBatteryAmt);
    Controller1.Screen.print("%");
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

/** AUTONOMOUS FUNCTIONS **/
void Move(double feet, int speed) { // Input in feet, speed in percent 0-100
  //input is feet, converts to inches, gets radians with arc length, converts to degrees
  float angle = ((feet * 12) / 2.125) * (180/3.14159);
  
  RightDriveMotors.setVelocity(speed, percent);
  RightDriveMotors.spinFor(reverse, angle, deg, false);
  LeftDriveMotors.setVelocity(speed, percent);
  LeftDriveMotors.spinFor(reverse, angle, deg, true);
}

void MovePID(double feet, int speed){ // Input in feet, speed in percent 0-100
  //input is feet, converts to inches, gets radians with arc length, converts to degrees
  float targetAngle = ((feet * 12) / 2.125) * (180/3.14159) / 2;
  RightDriveMotors.setPosition(0, degrees);
  LeftDriveMotors.setPosition(0, degrees);

  double turnToAngle = InertialSensor.heading(degrees);
  if(turnToAngle >= 360){
    turnToAngle -= 360;
  }
  else if(turnToAngle < 0){
    turnToAngle += 360;
  }
  double InertialPlus = 0;
  double differenceAngle = turnToAngle - InertialSensor.heading(degrees);


  float Kp = 0.18;
  float Ki = 0;
  float Kd = 0.4;

  float error = 0;
  float lastError = 0;

  float integral = 0;
  float derivative = 0;

  int counter = 0;

  Brain.Screen.clearScreen();
  Brain.Screen.setPenColor(red);
  Brain.Screen.drawLine(0, 120, 479, 120);
  Brain.Screen.setPenColor(white);

  float targetSpeed = 0;
  float curTarget = 0;

  int integralThreshold = 40;


  // For some reason I thought PID was always for distance, turns out it apparently can be used for correcting motors

  int nearTarget = 0;
  while(nearTarget < 3){
    // PID
    lastError = error + 0;

    error = (targetAngle - LeftDriveMotors.position(degrees));
    
    if(absFloat(error) < integralThreshold){
      integral = integral + error;
    }
    else{
      integral = 0;
    }
    derivative = error - lastError;

    targetSpeed = (Kp * error) + (Ki * integral) + (Kd * derivative);
    
    if(targetSpeed > speed){
      targetSpeed = speed;
    }
    if(targetSpeed < -speed){
      targetSpeed = -speed;
    }

    // Angle stuff
    differenceAngle = turnToAngle - (InertialSensor.heading(degrees) + InertialPlus);
    if(absDouble(differenceAngle - 360) < absDouble(differenceAngle)){
      InertialPlus += 360;
      differenceAngle = turnToAngle - (InertialSensor.heading(degrees) + InertialPlus);
    }
    if(absDouble(differenceAngle + 360) < absDouble(differenceAngle)){
      InertialPlus -= 360;
      differenceAngle = turnToAngle - (InertialSensor.heading(degrees) + InertialPlus);
    }

    curTarget += (targetSpeed - curTarget) > 3 ? 3 : ((targetSpeed - curTarget) < -3 ? -3 : (targetSpeed-curTarget));


    LeftDriveMotors.setVelocity(curTarget, percent);
    RightDriveMotors.setVelocity(curTarget - differenceAngle, percent);
    

    Brain.Screen.clearScreen();
    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("Dist: %f", (targetAngle - LeftDriveMotors.position(degrees)));

    counter ++;
    if(counter >= 480){
      counter = 479;
    }

    if(absFloat(error) < 5){
      nearTarget ++;
    }
    else{
      nearTarget = 0;
    }

    LeftDriveMotors.spin(forward);
    RightDriveMotors.spin(forward);

    counter ++;
    wait(25, msec);
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
  double maxSpeed = 25;

  double InertialPlus = 0;
  double difference = turnToAngle - InertialSensor.heading(degrees);
  double lastDifference = difference + 0;

  double intervals = 0;

  double Kp = 0.4;
  double Ki = 0.025;
  double Kd = 0.2;

  double goToTurnSpeed = 0;
  
  while(absDouble(difference) > 0.5){
    lastDifference = difference + 0;
    difference = turnToAngle - (InertialSensor.heading(degrees) + InertialPlus);
    if(absDouble(difference - 360) < absDouble(difference)){
      InertialPlus += 360;
      difference = turnToAngle - (InertialSensor.heading(degrees) + InertialPlus);
    }
    if(absDouble(difference + 360) < absDouble(difference)){
      InertialPlus -= 360;
      difference = turnToAngle - (InertialSensor.heading(degrees) + InertialPlus);
    }

    if(absDouble(difference) > 15){
      maxSpeed = 25;
    }
    else{
      maxSpeed = 7;
    }

    //turnSpeed = absDouble(difference) < 3 ? (difference < 0 ? -3 : 3) : difference;
    if(absDouble(difference) > 25 || (Ki * intervals) > 10){
      intervals = 0;
    }
    else{
      intervals +=(difference * Kp);
    }
    turnSpeed = (difference * Kp) + (Ki * intervals) + (Kd * (difference - lastDifference));
    turnSpeed = turnSpeed > maxSpeed ? maxSpeed : (turnSpeed < -maxSpeed ? -maxSpeed : turnSpeed);

    goToTurnSpeed += turnSpeed-goToTurnSpeed > 2 ? 2 : (turnSpeed-goToTurnSpeed < -2 ? -2 : turnSpeed-goToTurnSpeed);
    
    //turnSpeed *= 0.55;
    RightDriveMotors.setVelocity(-goToTurnSpeed, percent);
    RightDriveMotors.spin(forward);
    LeftDriveMotors.setVelocity(-goToTurnSpeed, percent);
    LeftDriveMotors.spin(reverse);

    Brain.Screen.clearLine(1);
    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("DIFF: %f", difference);
    Brain.Screen.setCursor(2, 1);
    Brain.Screen.print("PROPORTIONAL %.2f", (difference * Kp));
    Brain.Screen.setCursor(3, 1);
    Brain.Screen.print("INTEGRAL %.2f", (Ki * intervals));
    Brain.Screen.setCursor(4, 1);
    Brain.Screen.print("DERVATIVE: %.2f", (Kd * (difference - lastDifference)));
    Brain.Screen.setCursor(5, 1);
    Brain.Screen.print("TURN SPEED: %.2f", turnSpeed);

    wait(25, msec);
  }

  RightDriveMotors.stop();
  LeftDriveMotors.stop();
}

void AdjustRoller(float angleAmount) { // Spins roller at 50 speed for X seconds
  wait(0.3, seconds);
  ConveyorMotor.setVelocity(100, percent);
  ConveyorMotor.spinFor(reverse, angleAmount, degrees, true);
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

void AdjustFinger(int speed){
  if(speed != 0){
    FingerMotor.setVelocity(speed, percent);
    FingerMotor.spin(reverse);
  }
  else{
    FingerMotor.stop();
  }
}
#pragma endregion AUTONOMOUS_FUNCTIONS

#pragma region AUTONOMOUS
/** PRE AUTONOMOUS **/
void pre_auton(void) {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();
  FingerMotor.setPosition(0, turns); 
  InertialSensor.calibrate();
  Controller1.Screen.clearScreen();
  Controller1.Screen.setCursor(1, 1);
  Controller1.Screen.print("INERTIAL READY");
  //setupScreen();
  
  std::string theFullImage = "";
  for(int i = 0;i < amtOfImages;i ++){
    switch(i){

  #pragma region images
      case 0:
        theFullImage = "AA#D4D7D6,AB#B4ACA4,AC#74594E,AD#542A1A,AE#6B3529,AF#B19585,AG#C5BEB7,AH#6E6A67,AI#514037,AJ#3D3529,AK#986F5E,AL#8D7D76,AM#CDA890,AN#DDB89E,AO#F4D7B4,AP#959697,AQ#FCF7D7,AR#2A2017,AS#965349,AT#7C4939,AU#CC8C6D,AV#AE6350,AW#AC8368,AX#3F1008,AY#333859,AZ#1B0910,BA#252643,BB#161530,BC#484C6B,BD#525B81;AAx19-ABx1-ACx2-ADx12-AEx2-ACx1-AEx1-ACx2-AFx1-AAx1-AFx1-AGx1-AFx1-ABx2-AAx3-AHx1-ACx1-AIx3-ADx1-AJx1-ADx3-AAx18-AGx1-ACx3-ADx4-AEx1-ACx2-AKx1-AFx2-AKx1-ACx1-ADx2-ACx1-AEx1-ACx1-ADx1-ACx8-AHx1-ACx1-ADx1-AIx4-ADx5-AAx18-ALx1-ACx2-AEx1-ADx2-AEx1-ACx1-AHx2-ALx1-AFx1-AMx1-ANx2-AOx1-ANx1-AKx1-ACx3-ADx1-ACx3-AEx4-AIx1-ADx1-AIx1-ADx6-AJx1-AIx2-AJx1-ABx1-AFx1-APx1-ALx1-ABx2-ALx1-ACx1-AHx1-ALx2-ACx1-ALx1-ACx1-ALx1-AFx2-AHx1-ACx3-ADx1-AEx1-ACx1-AHx1-ACx6-AKx1-AMx1-AOx2-AQx1-AGx1-ALx1-AEx1-ADx1-AEx1-ACx3-AEx2-ADx1-AIx1-AEx1-AIx1-ADx3-ARx1-ADx3-AIx4-AJx1-AIx1-AJx4-ADx1-AEx1-AIx1-ADx1-AIx2-ADx1-AEx1-ADx2-ACx2-AEx1-ACx1-AEx1-ACx1-AHx2-ACx7-AKx1-AMx1-AOx1-AQx2-AGx1-ADx3-AEx1-ACx2-AEx1-ADx1-AEx1-AIx2-ADx3-ARx1-ADx5-AJx1-AIx3-AJx2-ADx1-AEx1-ADx5-ACx1-ADx6-ACx1-ADx1-ACx1-AEx1-AHx2-AKx1-ACx8-AKx1-AMx1-AQx3-ACx3-ADx5-AEx1-AIx2-ADx3-ARx2-ADx3-ARx1-AJx1-ADx4-AJx2-ADx6-AIx1-ACx1-AEx1-ADx3-ARx1-ACx1-AEx1-ACx2-AKx4-ACx1-ASx4-ACx1-AKx3-AMx1-AOx2-AFx1-ACx1-AKx1-ACx4-AIx2-ADx1-ARx3-ADx1-ARx4-ADx1-ARx3-AJx1-AIx1-AJx3-ADx2-ACx6-ADx5-ACx4-AKx5-ASx6-AKx3-AMx1-ANx1-AMx1-ACx1-AHx1-ACx4-AIx1-ADx1-ARx3-AJx2-ADx4-ARx1-ADx1-ARx1-ADx1-AJx4-AIx1-AEx1-ACx6-AIx1-ADx1-AEx1-ADx2-AEx1-ACx1-AHx1-ACx2-AKx3-ASx11-AFx1-ANx1-AMx1-ACx3-AIx4-ARx5-ADx1-AIx1-ARx2-AJx2-ADx2-ARx1-AJx2-AIx4-ACx1-AIx1-ACx8-AEx2-ACx3-ATx1-AKx2-ASx11-AKx1-AMx1-AOx1-AMx1-AFx1-ACx2-AIx4-ADx1-ARx7-ADx1-AIx1-AJx1-ADx1-AIx1-AJx1-ACx1-AIx1-ACx10-AKx1-ACx3-ADx2-AEx1-ACx2-AKx2-ACx1-ASx1-ATx1-AEx2-ATx1-ASx7-AKx1-ANx1-AMx1-AKx2-ACx1-AIx6-ADx3-ARx2-ADx2-AIx1-ACx2-AIx2-AKx1-ACx4-AHx4-ACx1-AKx4-ACx3-AEx2-ACx1-AKx1-ASx1-AKx1-ASx3-AEx3-ASx3-AEx2-ADx1-ASx2-AKx1-AUx1-AKx2-AIx7-AEx1-ADx1-AIx1-AEx1-AIx3-ACx1-AEx1-ADx1-AIx2-AKx1-ALx1-ACx7-AKx7-ACx2-ADx1-AEx1-ACx1-ASx1-AKx1-AVx1-ASx1-AVx1-ASx3-ATx1-ASx3-ATx1-ASx3-AUx1-AMx1-ACx1-AHx1-ACx2-AIx9-ACx1-AEx1-ADx5-ARx1-AMx1-AFx1-AMx8-AFx8-AMx1-AFx1-ACx1-AKx3-ASx9-AVx1-ASx1-AVx1-ASx1-AUx1-ANx1-ACx1-AKx1-AIx1-ACx1-AIx8-ADx5-ARx1-ADx2-AJx1-AMx2-AFx1-ANx5-AMx3-AFx3-AMx4-AWx1-AFx1-AKx1-AWx1-AKx2-AVx2-ASx11-AUx1-AFx1-ACx2-AIx11-AEx1-ADx2-AIx1-ADx3-AJx1-ANx2-AFx1-ANx3-AMx3-AWx1-ACx4-AKx2-ACx1-AKx1-AFx2-ALx1-AKx3-AVx3-ASx1-ATx1-AEx1-ASx1-ATx1-AEx1-ASx3-AKx1-AMx1-ACx1-AKx1-ACx2-AIx11-AEx1-AIx2-AEx1-ADx1-AIx2-ANx2-AMx1-AFx1-ANx1-AMx1-AFx1-ACx1-AEx2-ADx1-AEx2-ADx1-AKx1-AFx1-ABx2-AKx1-AHx2-AKx1-AIx1-AKx1-AVx2-ASx1-ADx2-AXx4-ADx1-AEx1-ASx1-AUx1-AOx1-ANx1-AFx1-ACx3-AIx5-ACx2-AIx6-AEx1-ADx2-AEx1-AMx3-AFx1-AMx1-AKx1-ACx2-AEx2-ACx2-AHx1-AFx1-AKx1-AHx3-AKx3-ACx1-AJx1-ACx1-ASx2-AEx1-ADx6-AEx1-ASx1-AKx1-AFx1-AOx1-AQx1-AOx2-ABx1-AKx1-ACx1-AIx2-ACx1-AIx1-ACx1-AIx7-AEx1-ADx3-AMx3-AFx2-ACx1-ADx1-ACx2-AHx1-AKx5-AHx2-AKx3-AHx1-AJx1-AYx1-ADx1-ASx2-ADx1-AEx2-ADx4-AEx1-ASx1-AKx1-ACx1-ANx1-AQx1-ANx1-AMx1-ANx1-AOx2-ABx1-AHx1-AIx3-ACx1-AIx5-AEx2-AIx1-ADx1-AEx1-AMx2-AFx1-ACx2-AKx1-ADx1-ACx1-AKx3-ALx1-AKx8-ACx1-ARx1-AYx1-AZx1-ADx1-ATx1-ADx2-AEx5-ADx1-AWx1-AHx1-ACx1-AMx1-AQx1-ANx1-AFx2-AMx2-ANx1-AOx1-ANx1-AFx1-ACx1-AIx1-AEx1-AIx9-AFx1-ACx1-AEx1-ACx2-AFx1-AKx4-ALx3-AKx7-ACx1-BAx1-AYx1-AZx1-ARx1-ADx5-AXx2-ARx1-ADx1-AKx1-ACx2-AFx1-AOx1-ANx1-AFx3-ALx2-AFx1-AGx1-AOx2-ABx1-ACx1-AIx5-AEx1-AIx3-AKx2-AFx2-ACx1-AHx1-AKx5-ALx2-AKx6-AHx1-ACx1-BAx3-BBx1-AIx1-AEx1-ADx7-AIx1-BCx1-ACx1-ALx1-AMx1-AFx1-ALx5-AFx1-ALx1-AFx1-AGx1-AOx2-ABx1-AIx1-ADx3-AJx3-AIx1-AFx3-ACx1-AIx1-AKx2-AFx1-ALx1-AKx1-ALx2-AKx6-AHx1-ACx2-BAx2-AYx1-BBx1-AYx1-BCx1-ACx1-AKx2-ACx2-AIx1-AJx1-AYx2-AHx1-AKx4-ALx1-AKx3-ALx3-AFx1-ABx1-AGx1-ABx1-AAx1-ACx1-ADx1-AJx5-ADx4-ACx1-AKx2-AFx1-AKx2-ALx1-AFx1-AKx5-ACx2-AKx1-ACx1-AYx7-BCx1-AHx1-BCx1-AYx1-BAx1-AYx1-BAx1-AIx1-AHx1-ALx1-AKx7-ALx5-AFx2-ALx1-AOx1-ADx1-AJx1-AIx1-AJx3-ADx4-AEx1-AKx1-AWx1-AFx1-AKx2-ALx1-AKx5-ACx3-AHx2-AYx1-BCx4-AYx4-BCx1-AYx3-BBx1-AHx2-AKx1-AMx1-ABx1-AKx2-ALx1-AKx2-ALx4-AFx3-AKx1-AFx1-AIx1-AJx5-ADx4-ACx2-AUx1-AFx1-AKx2-ALx1-AKx5-ACx3-AHx2-AYx3-BCx1-AYx2-BCx6-AYx1-BCx1-AKx1-ACx2-AEx1-AKx4-AFx2-ALx4-AFx1-ALx1-AFx1-AKx2-AIx1-AJx5-AIx4-ACx2-AKx1-AUx1-AKx2-ALx1-AKx5-ACx4-AHx1-AYx2-BCx2-AYx2-BCx3-BDx1-BCx1-AYx2-AHx1-ACx3-AKx6-ALx5-AFx1-ALx1-AFx1-AKx2-AIx1-AJx5-AHx1-ACx2-AIx2-ACx2-AFx1-AKx1-ALx2-AKx7-ACx3-AYx1-BCx5-BDx2-BCx5-AHx1-ACx1-AKx8-ALx1-AKx1-ALx1-AUx1-AKx1-ALx3-AKx2-ACx1-AIx1-ACx2-AIx2-ALx1-ACx3-AHx1-AKx1-ACx1-AFx1-AKx1-ALx2-AKx7-ACx1-AHx1-ACx1-AYx2-BCx4-BDx2-BCx3-AYx1-BCx1-ACx1-AKx7-ALx1-AKx1-ALx1-AKx2-ALx2-AUx1-ALx1-AKx1-ACx1-AKx1-AFx3-AMx1-ANx1-AMx1-AKx4-AHx1-AKx1-AEx1-AKx2-ALx1-AKx8-ACx1-AHx1-ACx1-BCx4-AYx1-BDx4-BCx4-ACx1-AKx4-ALx4-AKx1-ALx2-AKx1-ALx3-AKx1-ACx2-AKx1-AFx2-AKx1-AFx2.";
        break;
      case 1:
        theFullImage = "AA#D4D8D6,AB#7F675F,AC#572E1F,AD#6E4134,AE#A9806F,AF#BF9580,AG#865947,AH#D0C7BD,AI#9A9190,AJ#B4ACA7,AK#483D30,AL#CDA893,AM#DFB99D,AN#F3D5B2,AO#30281D,AP#FBF4D5,AQ#5C4E4D,AR#887D79,AS#A26155,AT#1D1712,AU#954839,AV#D18C6D,AW#803227,AX#BB7259,AY#401009,AZ#32354D,BA#22223A,BB#07071C,BC#454665,BD#4C557C;AAx20-ABx2-ACx3-ADx1-ABx2-AEx1-AFx2-AEx1-ABx1-ACx3-ADx2-AGx2-AEx1-AHx1-AIx1-AJx1-AIx1-AJx2-AAx3-ABx1-AGx1-ACx1-ADx1-AKx1-ACx5-AAx19-AEx1-AGx1-ABx1-ACx1-ABx4-AEx1-AFx1-ALx1-AMx2-ANx2-AEx1-ACx2-ADx1-ABx1-ACx1-ADx1-ABx1-ADx2-ABx5-ADx1-ACx2-ADx1-ACx3-AOx1-ACx2-AOx1-AAx18-AJx1-ADx1-ABx10-AEx1-ALx1-ANx2-APx1-AAx1-ABx3-ACx1-ADx1-ABx1-AGx1-ADx1-ACx2-ADx2-ACx1-ADx1-ACx3-AOx3-ACx1-ADx2-AKx1-AIx7-ABx1-AQx1-ABx1-ARx1-ABx4-AIx3-ABx3-AEx1-ABx10-ALx1-ANx1-APx2-AAx1-AEx1-ADx1-ACx2-AGx1-ABx1-AGx1-ADx1-ACx4-AKx1-ACx3-AOx3-ACx2-AKx8-AOx1-ACx5-ADx1-ACx5-ABx1-ADx1-ABx6-AGx6-AEx1-AMx1-APx3-AFx1-ADx1-ACx3-ADx1-ABx1-ADx1-ACx7-AOx2-ACx1-AOx2-ACx2-AKx1-ACx4-AOx1-ACx7-ADx1-ACx5-ADx2-ABx3-AGx1-ABx1-AGx8-AEx1-ALx1-APx2-AMx1-ADx1-ACx1-ABx1-ACx9-AOx6-ACx1-AOx1-ACx1-AOx1-AKx1-ACx2-AOx2-ACx7-ABx1-ACx6-ADx1-ABx3-ASx1-AGx10-AEx1-AFx1-AMx1-ANx1-ABx4-ADx2-AQx1-ADx2-AOx1-ATx1-AOx10-ACx1-AKx5-ACx2-ADx1-ABx2-ADx1-AGx1-ABx1-ACx2-AOx1-ACx2-ADx1-ABx2-ASx3-AGx11-AFx1-AMx1-ANx1-ABx2-ADx1-ABx1-AQx2-ADx2-AOx5-ACx3-AOx1-ACx3-AOx7-ADx1-ACx1-ADx1-ABx5-ACx2-ADx1-ACx2-ADx1-ABx2-ASx3-AGx11-AEx1-ANx2-AEx1-ABx1-ADx1-ABx1-ADx2-AKx1-ADx1-AOx3-ATx1-AOx2-AKx1-AOx2-AKx1-ACx2-AOx2-AKx1-AOx1-AKx1-ADx6-ABx1-AGx1-ADx1-ABx3-ADx4-AGx1-ADx1-ASx2-AGx10-ADx2-AEx1-ALx1-ANx1-AFx1-ABx1-AGx1-ABx1-ADx4-ACx1-AOx3-ATx2-AOx3-ACx1-ADx3-ACx1-AEx1-ADx3-AGx2-ABx2-ADx1-ABx5-ADx1-AGx1-ADx1-ACx2-ASx2-AGx3-ADx4-AUx1-AGx2-ADx3-AGx2-AEx1-AFx1-AVx1-ABx3-ADx6-ACx3-AOx2-ACx2-ADx1-ABx1-AGx1-ADx2-ABx2-ADx1-AGx1-ABx8-AEx2-ABx3-AGx1-ADx1-AGx2-ASx1-AGx3-ACx1-ADx1-AWx1-ADx2-AGx1-ADx1-ACx2-ADx1-AGx1-ASx1-AFx2-ABx3-ADx6-ACx4-ADx1-ACx2-AQx1-ADx1-ACx2-AOx1-AEx3-ABx1-AEx1-ABx5-AEx1-ABx2-AEx2-ABx3-ADx1-AGx2-ASx2-AGx1-ASx1-AGx1-AUx1-ADx2-AGx3-ASx1-AGx3-ASx1-ALx1-AFx1-ABx2-AGx1-ADx8-ACx1-ADx1-AGx1-ACx2-AOx5-ALx1-AFx1-ALx2-AMx3-ALx4-AFx2-AEx1-AFx1-ALx4-AFx1-ASx1-AXx1-ASx2-AGx4-AUx1-AGx6-ASx2-AMx1-AEx1-ABx2-ADx9-ACx6-AOx2-ACx1-AOx1-ALx2-AFx1-AMx6-ALx1-AFx2-AEx2-AFx4-AEx4-ASx5-ADx1-ASx2-AGx2-AUx1-AGx2-ASx1-AXx1-ALx1-ADx1-ABx2-ADx11-ACx7-AKx1-AMx2-AFx1-ALx1-AMx3-ALx1-AFx1-AEx1-ADx1-ACx1-ABx1-ADx1-ABx3-AEx3-ABx2-ASx4-AUx1-ADx1-ACx1-AUx1-ADx1-ACx1-ADx2-AGx1-AXx1-AFx1-ALx1-ABx1-AEx1-ABx1-ADx12-ACx1-ADx2-ACx4-AMx3-AFx1-ALx1-AFx1-ABx2-ACx4-ADx1-ABx1-AEx1-AFx1-AEx1-ABx1-AEx1-ABx2-AKx1-ASx3-AGx1-ACx2-AYx4-ATx1-ACx1-AGx1-AEx1-ALx1-APx2-ANx1-AFx1-ABx1-ADx12-ACx6-AMx1-ALx1-AMx1-AFx1-ALx1-AEx1-ABx5-AEx2-ABx4-AEx2-ABx1-AKx1-AZx1-AGx2-ASx1-ADx1-ACx5-ATx1-ACx1-ADx1-ASx1-AEx2-APx2-ANx3-AAx1-AJx1-ABx1-ADx10-ACx5-ALx3-AFx1-AEx1-ABx1-ACx1-AGx1-ABx1-AEx4-ABx3-AEx3-ABx1-AOx1-AZx1-ACx1-AGx2-ACx2-ADx1-ACx4-ADx2-AEx2-ABx1-ANx1-APx1-AMx1-AEx1-AFx1-ALx1-AHx1-ANx1-AHx1-AFx1-ABx1-ADx6-ACx6-ALx2-AFx1-ABx1-ACx1-AEx1-ABx1-AEx6-ABx1-AEx5-AQx1-BAx1-AZx1-ATx1-ACx1-ADx1-ACx3-ADx1-ACx2-ADx1-ACx1-ADx1-AEx1-AQx1-ABx1-ALx1-APx1-AHx1-AEx2-AFx3-ALx1-AHx1-APx1-AHx1-AEx1-ACx1-ADx7-ACx2-AEx1-AGx1-ADx1-ABx2-AEx8-ABx1-AEx4-ABx1-ADx1-AZx2-BBx1-AOx1-ACx2-AYx5-ATx1-AYx1-AGx1-ABx1-BCx1-ABx1-AFx1-ANx1-ALx1-AEx7-AFx1-AHx1-ANx2-AEx1-ACx5-ADx1-ACx2-AEx1-AFx1-ALx1-AFx1-AEx14-ABx1-AGx1-BAx3-BBx1-AZx1-ADx1-ACx5-AOx1-ACx1-ADx1-AQx1-AZx1-ABx1-AEx1-AFx1-AEx10-AJx1-AHx3-ABx1-ACx6-ABx3-AEx14-ABx3-AOx1-BAx1-AZx1-BAx2-AZx1-AQx1-ABx3-AQx1-AKx1-AZx2-BAx1-AKx1-ABx1-AEx7-ABx1-AEx5-AFx3-AHx1-AEx1-ACx1-AKx1-ACx1-AOx2-ACx3-ABx1-AEx7-ABx1-AEx3-ABx5-AZx8-AQx1-BCx1-AZx4-BBx1-ABx2-AFx1-AEx6-ABx1-AEx9-AJx1-ACx1-AKx2-AOx2-ACx3-ABx1-AEx7-ABx4-ADx2-ABx3-AZx1-BCx7-AZx1-BCx2-AZx2-BAx1-AZx1-ABx3-AFx1-AJx1-AEx15-AKx1-AOx4-ACx3-ABx2-AEx6-ABx2-AEx1-ABx3-AGx1-ABx2-AZx1-BCx4-AZx1-BCx7-AZx1-ABx2-ADx3-ABx3-AEx2-AFx1-AEx10-AKx1-AOx4-ABx1-ADx2-ABx2-AEx9-ABx6-AZx2-BCx3-AZx1-BDx1-BCx3-BDx1-BCx1-AZx2-ABx5-AEx1-ABx2-AEx1-ABx1-AEx9-ABx1-AEx1-AKx2-ACx1-AOx2-AEx2-ABx2-ADx1-AEx9-ABx6-AZx1-BCx2-BDx1-BCx2-BDx4-BCx3-AQx1-ABx5-AEx14-ABx1-AEx1-ABx1-AEx2-ABx2-AEx1-ABx2-AEx1-ADx1-ABx1-AEx5-ABx1-AEx1-ABx7-AZx1-BCx5-BDx4-BCx2-AZx1-AQx1-ABx4-AEx14-ABx2-AEx3-ALx1-AMx1-ALx1-AFx1-AEx2-ABx2-ADx1-AEx7-ABx7-AZx1-BCx3-AZx1-BCx1-BDx4-BCx3-AQx1-ABx4-AEx14-AGx1-ABx1-AEx1-AFx1-ABx2-";
        break;
      case 2:
        theFullImage = "AA#D2D3D1,AB#A69B97,AC#7C665C,AD#644132,AE#734F45,AF#957670,AG#B4907E,AH#D1AA8F,AI#CEBFB0,AJ#8E8989,AK#AFADAB,AL#5C5656,AM#3E3529,AN#EDCEA8,AO#5F2D1E,AP#F9ECC9,AQ#FBFBE7,AR#281F15,AS#474142,AT#925652,AU#A96858,AV#7B3528,AW#904134,AX#CC886D,AY#4A1810,AZ#2E3655,BA#1D1E37,BB#080514,BC#3F496E;AAx21-ABx1-ACx1-ADx1-AEx1-ACx1-AFx1-AGx1-AHx3-AGx1-AFx1-ADx5-AEx1-ACx1-AFx1-AIx1-AAx1-ABx1-AAx1-AJx1-AKx1-AAx4-ALx1-ACx1-ADx2-AMx4-AEx1-AAx21-AFx1-ACx4-AFx2-AGx1-AHx1-AIx1-ANx3-AFx1-ADx1-AOx1-ADx1-AEx1-ACx1-ADx1-ACx1-AFx1-ACx2-AFx3-ABx2-ACx1-ADx4-AMx5-AAx20-ABx1-ACx9-AGx1-AIx1-ANx1-APx2-AAx1-ADx2-ACx2-ADx1-AEx1-ACx1-AEx1-ADx7-AMx1-ADx1-AMx7-AAx9-AKx1-ABx1-AIx2-ABx1-AIx1-ABx1-AIx1-AAx2-AKx1-ACx11-AFx1-AIx1-APx2-AQx1-AIx1-ADx1-ACx2-AMx1-ADx1-ACx2-ADx7-AMx3-ARx1-AMx1-ARx1-AMx1-ADx2-ACx1-ADx1-AEx1-ADx1-ASx1-ALx1-AMx1-ASx1-AMx2-ADx3-AEx1-ADx4-AMx1-ACx1-AFx1-ACx1-ATx2-AEx5-ATx2-ACx1-AFx1-ANx1-APx1-AQx1-APx1-ACx1-ADx1-ACx1-AMx1-ADx2-ACx2-ADx2-AMx6-ARx1-AMx2-ARx1-AMx3-AFx1-AMx7-ADx3-AMx2-ACx1-AMx5-ACx1-AFx1-ACx3-AEx5-ATx4-AGx1-AIx1-APx2-AGx1-ADx3-ACx1-ADx1-AMx1-ADx3-AMx1-ADx1-AMx2-ARx1-AMx1-ARx2-AMx2-ARx2-AMx1-AGx1-ADx1-AMx9-ADx1-AMx1-ACx1-AMx1-ADx1-AMx3-ACx3-ATx4-AEx2-ATx6-AGx1-AHx1-ANx2-ADx1-ACx4-ADx2-AEx1-ADx2-ARx4-AMx1-ARx4-AMx1-ARx1-AMx1-ACx1-AMx8-ADx1-AEx1-ACx1-ADx1-ACx1-ADx1-AMx4-ACx1-AUx1-ATx14-AHx1-ANx2-ACx3-ADx1-ACx1-ADx4-ARx4-AMx4-ARx1-AMx2-ARx1-AMx1-ADx2-AMx5-ADx2-ACx5-ADx1-AMx1-ADx1-AMx2-ACx1-AUx2-ATx13-AGx1-ANx2-AGx1-ACx2-ADx1-ACx1-ADx4-ARx5-AMx2-ARx1-AMx1-ADx1-AMx3-ADx1-ACx1-AMx3-ADx6-ACx1-AEx1-ACx1-AEx1-ACx3-AEx2-AUx1-ATx11-AEx3-AGx1-ANx2-AGx1-AEx1-ACx1-ADx1-ACx1-ADx4-AMx1-ARx7-AMx1-ADx2-AMx1-ADx1-AGx1-ACx2-ADx2-AEx3-ACx1-AEx2-ACx5-ADx1-AEx1-AUx2-AEx2-ADx3-AVx2-ATx4-AVx1-AWx1-AEx2-ATx1-AGx1-AHx1-AXx1-ADx1-ACx1-ADx1-ACx1-ADx6-AMx3-ARx2-AMx2-ADx1-ACx2-ADx1-AHx1-AFx2-AEx1-ADx1-ACx9-AFx2-ACx2-ATx5-AEx1-ADx2-AOx1-AVx1-AWx2-ADx1-AOx3-ATx2-AUx1-AGx1-AXx1-ADx1-ACx1-AEx1-ACx1-ADx7-AMx1-ADx6-AMx3-AGx2-AFx1-AGx1-AFx1-ACx1-AFx1-ACx5-AFx1-ACx1-AFx3-ACx1-ATx1-AUx2-ATx2-AUx1-AEx1-AWx1-ADx2-AWx1-ATx3-ADx1-AWx1-ATx1-AEx1-AUx1-AHx1-AGx1-ADx1-ACx1-AEx1-ACx1-ADx11-AMx3-ARx1-AMx1-ARx1-AHx3-AGx1-AHx2-ANx2-AHx6-AGx2-AHx3-ATx1-AUx2-ATx6-AEx1-ATx8-ANx1-AGx1-ADx1-ACx3-ADx11-AMx3-ARx1-AMx2-AHx5-ANx4-AHx3-AGx2-AFx1-AGx2-AHx1-AGx1-ATx1-AUx3-ATx3-AWx1-ATx1-AEx3-ATx1-AWx1-ATx3-AXx1-AHx1-ACx1-ADx1-AFx1-ACx1-ADx12-AMx1-ADx2-AMx3-AHx1-ANx3-AGx1-ANx3-AHx2-AGx1-ACx1-ADx3-ACx2-AEx1-ACx1-AFx1-AUx5-ATx1-AVx1-AWx1-ATx3-AVx1-ATx1-AWx1-ATx1-AUx1-AGx2-ACx4-ADx18-AHx7-AGx1-ACx2-ADx4-ACx1-AFx3-ACx1-AFx1-ADx1-AUx3-ATx1-AWx1-AVx1-AOx2-AVx1-AOx1-AYx1-AOx2-ATx1-AUx1-AHx1-AIx1-AHx1-AFx1-ACx2-ADx16-AMx1-ADx1-ANx2-AHx3-AGx1-AHx1-AFx1-ACx3-AFx1-ABx2-AFx1-ACx1-AFx3-ACx1-AMx1-ATx1-AUx1-ATx2-AOx2-AYx4-ARx2-AOx1-ATx1-AUx1-AHx1-APx4-AHx1-AFx1-ADx15-AMx2-ANx2-AHx3-AGx1-AFx1-AEx1-ACx1-AFx2-ACx4-AFx3-ACx1-AMx1-ASx1-ADx1-ATx2-AWx1-AOx2-ARx1-AOx3-ARx1-AOx2-ATx1-AGx1-ACx1-APx2-ANx1-AHx1-AIx1-APx2-AIx1-AFx1-ADx14-AHx4-AFx1-ACx1-AFx6-ACx2-AFx4-ACx1-ARx1-AZx1-ARx1-ADx1-ATx1-ADx1-AOx1-AVx1-AOx5-AVx1-ADx1-AGx1-ACx2-ANx1-APx1-ANx1-AGx2-AHx3-ANx1-APx1-ABx1-ACx1-ADx11-AHx1-AGx1-ACx1-AEx1-ACx1-AFx7-ACx1-AFx5-ADx1-BAx1-AZx1-BBx1-ARx1-ADx2-AOx6-ADx1-AOx1-AEx1-AGx1-ALx1-ACx1-AHx1-APx1-ANx1-AFx2-AGx1-AFx2-AGx2-ANx1-APx1-ANx1-AGx1-ADx2-AMx3-ADx4-ACx1-AFx1-AGx1-AFx14-ACx1-ADx1-AZx2-BBx1-ARx1-AMx1-AOx1-ARx2-AYx5-ARx1-AEx1-ACx1-ASx1-ACx1-AGx1-ANx2-AFx8-ABx1-AIx1-APx2-ABx1-ADx1-AMx6-AHx1-AFx16-ACx1-AEx1-ARx1-BAx1-AZx1-BBx1-BAx1-AMx1-ADx1-ARx1-AYx1-AOx3-ARx1-AMx1-ADx1-ALx1-ASx1-ACx1-AFx1-AHx2-AFx10-AHx1-ANx1-AHx1-AAx1-ACx1-AMx5-AGx1-ACx1-AFx8-ACx1-AFx5-ACx3-ARx1-BAx1-AZx1-BAx2-AZx1-ASx1-ALx1-ACx2-ADx1-AMx1-ARx1-BAx1-AZx1-BAx1-AMx1-ACx1-AFx1-ABx1-AFx5-ACx1-AFx6-AHx1-ABx2-AHx1-ACx1-AMx4-ADx1-ACx1-AFx2-AGx1-AFx9-ACx5-ASx1-AZx7-ASx3-AZx4-BBx1-ACx2-ABx1-AHx1-AFx5-ACx1-AFx7-ABx1-AFx2-ABx1-AMx4-ACx2-AFx2-AGx1-AFx4-ACx5-ADx2-ACx3-ASx1-AZx2-BCx4-AZx3-BCx1-AZx3-BAx1-AMx1-ACx3-AGx1-AIx1-ABx1-AFx13-ACx1-AFx1-AMx4-ABx1-ACx2-AFx1-AGx1-AFx4-ACx3-AFx1-ACx3-ADx1-ACx2-AZx8-BCx6-AZx1-ACx2-ADx4-ACx2-AFx2-AGx2-AFx8-ACx1-AFx1-ADx2-AMx2-AFx2-ACx1-AFx10-ACx3-AEx1-ACx2-AZx6-BCx6-AZx3-ACx1-AEx1-ACx1-AEx1-ACx7-AFx9-ACx1-AFx2-AGx4-AFx1-ADx1-AFx9-ACx4-AEx1-ACx2-AZx3-BCx2-AZx1-BCx8-ASx1-ACx8-AFx1-ACx2-AFx8-ACx2-AFx1-AGx2-AHx1-ANx1-AGx2-ADx1-ACx1-AFx8-ACx7-AZx2-BCx11-AZx1-ALx1-ACx5-AFx5-ACx1-AFx8-ACx2-AFx1-AGx1-ACx2.";
        break;
      case 3:
        theFullImage = "AA#D1D2D2,AB#8E8080,AC#806C6A,AD#C09C87,AE#D6AF96,AF#754737,AG#8A594A,AH#AF8D7A,AI#C1BEBA,AJ#AAA5A6,AK#4A3F32,AL#685D5A,AM#98959B,AN#E8C6A3,AO#F7E0BE,AP#5F3221,AQ#2F271C,AR#FAF5DC,AS#A97865,AT#574D47,AU#1A1212,AV#C78669,AW#AF624D,AX#9B4A3C,AY#621811,AZ#3F130B,BA#2E304A,BB#3B4062,BC#151A38,BD#060824,BE#4C557B;AAx22-ABx1-ACx1-ABx2-ADx1-AEx3-ADx1-ACx1-AFx6-AGx1-AFx1-AHx1-AEx1-AAx1-AIx2-AJx2-AIx1-AAx3-ACx2-AFx1-AKx5-ALx1-AAx20-AMx1-ACx4-ABx1-ADx2-AEx1-ANx2-AOx1-AEx1-ACx1-APx4-AFx2-ACx1-AFx1-ABx2-ACx1-ABx1-ADx1-ABx1-ADx2-ABx1-AFx3-AKx2-AQx2-AKx1-AQx1-AAx18-AIx1-ABx1-ACx3-AGx2-AFx1-ACx2-ADx1-AEx1-ANx1-AOx2-ARx1-ADx1-AFx1-APx1-AKx1-AFx1-ACx1-AFx2-ACx1-AFx8-AKx3-AQx3-AKx3-AAx11-AJx1-AAx2-AJx1-AIx3-ABx1-ACx3-AGx7-AHx1-AEx1-ANx1-ARx3-ABx1-APx2-AFx1-ACx1-AKx2-AFx1-AGx1-AFx2-AKx2-AFx1-AKx4-AQx5-AKx1-AFx1-AHx1-ABx1-ALx1-ACx1-ALx3-AKx1-ALx1-AKx3-AFx4-AKx1-AFx1-ABx1-ACx1-AGx10-AHx1-ANx1-ARx3-AOx1-APx3-ACx1-AKx2-AFx2-AGx1-AFx1-AKx4-AQx9-AKx1-AGx1-AHx1-AFx1-AKx11-AGx1-AQx1-AKx1-ACx3-AGx11-ASx1-ANx1-ARx2-AOx1-AFx1-APx2-AFx3-ACx1-AKx7-AQx10-ADx1-AHx1-AGx1-AQx1-AKx3-AQx2-AKx5-ACx1-AKx2-ACx2-ASx1-AGx12-ASx1-AEx1-ANx1-AOx1-ADx1-AKx2-AFx1-ACx3-AFx5-AQx12-AGx1-ACx1-AKx9-AFx3-ACx1-AFx1-AKx1-ACx1-ASx2-AGx13-ADx1-ANx2-AEx1-AGx1-ACx1-AFx1-ACx1-AFx1-AGx2-AFx3-AQx13-ASx1-AFx1-ATx1-AQx3-AKx1-AQx1-AKx2-AFx1-ACx2-AGx1-ACx1-AFx1-APx1-ACx1-ASx2-AGx13-ASx1-AEx1-ANx2-ADx1-ACx1-AFx1-ACx1-AFx2-AGx1-AFx1-AKx2-AQx6-AKx1-AQx2-AKx1-AQx3-AHx1-ASx1-ACx1-AQx2-AKx4-AFx3-ACx1-AFx1-ACx1-AFx2-AGx1-ASx3-AGx12-ASx1-AEx1-AOx1-AEx1-AHx1-AFx2-ACx1-AFx1-AGx1-AFx2-AKx3-AQx3-AUx1-AQx4-AKx3-AFx1-AHx1-ADx1-ACx2-AFx5-ACx1-AGx1-AFx2-ACx3-ASx1-AGx2-ASx1-AGx9-AFx4-AGx1-ADx1-ANx1-AEx1-AVx1-AFx2-ACx3-AFx4-AKx4-AQx3-AKx2-AFx1-AGx2-AFx1-ADx3-ABx1-ACx1-AFx2-AGx1-ACx7-ASx3-AGx2-AFx7-AGx1-AFx5-AGx2-AHx1-ADx1-ASx1-AGx1-AFx1-AGx1-ACx2-AFx5-AKx8-AFx1-AKx1-AQx1-AKx1-ADx3-AHx1-ADx1-AHx1-ASx3-ACx4-ASx1-ACx1-ABx1-ASx1-AGx5-AFx1-APx2-AFx5-APx2-AFx1-AGx2-AHx1-AEx1-ADx1-AGx1-AFx2-ACx2-AFx11-AKx1-AQx5-AEx4-ADx1-AEx1-ADx1-AEx6-ADx2-AHx1-ADx1-AGx1-ASx1-AGx5-AFx3-AGx4-AFx1-AGx3-ASx1-AEx1-ADx1-AFx3-ACx2-AFx9-AKx4-APx1-AQx3-AEx4-ADx2-AEx7-ADx1-AHx1-ASx1-ADx1-ASx3-AGx5-AFx1-AGx4-AFx1-AGx4-ASx1-AEx1-AFx2-ACx1-AFx1-ACx2-AFx10-AKx7-AEx5-ADx1-ANx1-AEx3-ADx1-AHx1-AGx1-AKx1-AFx3-ACx1-ASx3-AWx1-AGx2-AFx2-AXx1-AGx2-AFx3-AGx2-ASx1-AVx1-ADx1-AFx2-ACx1-AFx1-ACx2-AFx13-AKx4-AEx6-ADx1-AEx1-ADx1-ACx1-AFx1-AKx2-AFx1-ACx2-ABx2-ACx1-ASx2-AWx2-AGx1-AFx2-AXx3-AFx4-AGx1-ASx1-ADx1-AEx1-AHx1-AFx1-ACx1-AFx1-ABx1-AGx1-AFx14-AKx3-AEx6-ADx1-AEx1-ABx1-AFx1-ABx1-ADx3-ABx4-AKx1-AFx1-ASx1-AWx1-AGx1-AFx2-APx1-AYx1-APx2-AYx1-AZx1-AQx1-APx1-AGx1-ASx1-AEx1-ARx2-AOx1-ANx1-ADx2-AFx13-AKx5-AEx6-ADx1-ABx4-ACx3-ABx3-ACx1-AQx2-AGx3-APx1-AYx2-AZx2-AYx1-AZx1-AYx1-APx2-AGx1-ASx2-AOx2-ANx2-AOx1-ARx1-AOx1-ADx1-ACx1-AFx12-AKx3-AEx4-ADx1-AHx1-ABx5-ACx2-ABx4-ALx1-BAx1-AUx1-APx1-AGx2-APx2-AYx6-AFx1-APx1-ASx1-ACx1-AFx1-ANx1-ARx1-AOx1-AHx1-ADx2-AEx1-ANx1-AOx1-AAx1-ADx1-AGx1-AFx1-AKx3-AFx5-AKx1-AFx1-AKx1-ADx2-AHx1-ASx1-ABx7-ACx1-ABx4-ACx1-AKx1-BAx1-AUx2-AFx2-AYx1-APx1-AFx2-APx3-AFx1-APx1-AFx1-ADx1-ALx1-AGx1-AEx1-AOx2-AHx2-ADx4-AEx1-AOx2-AIx1-ABx1-AKx4-APx1-AKx5-ADx1-ASx1-ABx8-ACx1-ABx5-ACx1-AQx1-BBx1-BCx1-AUx1-AQx1-APx2-AYx1-APx2-AYx1-AUx1-AYx3-AFx1-AHx1-BBx1-AFx1-ADx1-AOx1-ANx1-AHx1-ABx1-ASx1-AHx1-ABx3-AHx1-ADx1-ANx1-AOx2-ABx1-AKx3-AQx1-AKx1-AQx3-ADx1-ASx1-ACx4-ASx1-ABx3-ACx1-ABx5-AGx1-AKx1-BAx1-BCx1-AUx2-AKx1-AQx2-AYx2-APx1-AYx4-AFx1-ALx1-BAx1-ACx1-AHx1-AEx2-ABx4-ACx1-ABx4-AHx1-AEx1-AOx1-ANx1-AIx1-ABx1-AKx4-AQx1-AKx1-AHx1-ASx6-ABx2-ACx1-ABx5-ACx1-AFx1-AKx1-BDx1-BCx1-BAx1-BDx1-BCx1-BAx1-APx1-AQx1-AYx2-AQx4-AKx1-BBx1-BAx1-ACx1-ABx1-ADx1-ABx5-ACx2-ABx5-ADx1-AEx1-ADx1-AEx2-AKx2-AQx3-ASx2-AHx1-ASx1-ACx1-ASx1-ABx2-ACx1-ABx5-ACx3-ALx1-BCx1-BAx2-BCx1-BAx1-BBx1-BAx1-AKx1-ACx1-ALx1-AKx1-BAx4-AUx1-AKx1-ACx1-ABx8-ACx1-ABx6-ADx1-AHx1-ABx1-AEx1-AFx1-AQx4-AGx1-ASx1-AHx2-ACx1-ASx1-AHx1-ACx2-ABx3-ACx1-AFx1-ACx1-AGx1-ACx1-ALx1-BAx2-BBx3-BAx5-BBx1-BAx3-BCx1-AUx1-ACx2-ABx1-AEx1-ABx6-ACx1-ABx8-ACx3-AQx1-AKx1-AQx2-AGx1-ASx1-AHx1-ASx3-AHx1-ACx5-AFx4-AGx1-ALx1-BAx1-BBx11-BAx3-AKx1-ABx1-ACx3-ADx2-ABx5-ACx2-ABx1-ASx2-ABx3-ACx2-ABx1-AKx1-AFx1-AKx2-AFx1-ASx1-AHx1-ASx4-ACx3-ABx1-ACx2-AFx3-AGx1-ALx1-BAx3-BBx2-BAx1-BBx8-BAx1-ACx2-AFx5-ACx3-ABx1-ADx1-ACx2-ASx3-ABx1-ASx3-ACx1-AHx2-ADx1-AEx2-AGx1-ASx1-AHx1-ASx3-ACx4-ABx1-ACx4-AFx1-ACx1-ALx1-BAx2-BBx3-BAx1-BBx4-BEx2-BAx3-ACx1-AFx3-ACx8-ASx1-ACx1-ASx6-ACx1-AGx1-AHx1-ADx1-AHx1-ADx2-AGx1-AFx1-AHx1-ASx2-ABx1-ACx1-ABx4-ACx4-AFx1-ACx1-ALx1-BAx1-BBx2-BEx2-BBx1-BEx3-BBx1-BEx4-BBx1-ACx1-AGx2-ACx5-ABx2-ACx1-ASx2-ACx1-ASx6-AFx1-ACx1-AHx1-ACx1-AFx2.";
        break;
      case 4:
        theFullImage = "AA#DED3DB,AB#F4E3E6,AC#FBFCF4,AD#E7C4C5,AE#D5B5B6,AF#C6A5A5;AAx18-ABx7-AAx3-ABx8-ACx20-ABx4-AAx18-ABx7-AAx3-ABx7-ACx21-ABx4-AAx18-ABx7-AAx1-ABx9-ACx21-ABx4-AAx16-ABx18-ACx22-ABx4-AAx15-ABx20-ACx21-ABx4-AAx15-ABx1-AAx12-ABx7-ACx21-ABx4-AAx28-ABx8-ACx19-ABx5-AAx16-ADx1-AAx13-ABx6-ACx18-ABx6-AAx13-ADx4-AAx15-ABx5-ACx16-ABx7-AAx12-ADx5-AAx15-ABx6-ACx13-ABx9-AAx11-ADx5-AAx17-ABx9-ACx5-ABx13-AAx11-ADx5-AAx18-ABx11-ACx1-ABx14-AAx11-ADx6-AAx20-ABx23-AAx10-ADx6-AAx23-ABx21-AAx10-ADx7-AAx23-ABx20-AAx10-ADx8-AAx22-ABx20-AAx2-ADx1-AAx6-ADx8-AAx1-ADx1-AAx21-ABx20-AEx2-AAx1-ADx2-AAx4-ADx8-AAx23-ABx20-AEx4-ADx1-AAx4-ADx10-AAx22-ABx19-AEx4-ADx1-AAx4-ADx4-AEx1-ADx6-AAx21-ABx19-AEx5-ADx1-AAx1-ADx12-AAx23-ABx5-AAx2-ABx11-AEx3-ADx1-AAx2-AEx3-ADx11-AAx24-ABx2-AAx11-ABx3-AEx1-ADx1-AEx5-AAx1-ADx12-AAx9-ADx1-AAx1-ADx10-AAx3-ABx1-AAx5-ADx1-AAx2-ADx4-ABx3-AEx6-ADx1-AAx1-ADx11-AAx10-ADx14-AAx2-ADx1-AAx3-ADx5-AEx3-AAx3-AEx6-ADx1-AEx1-ADx10-AAx8-ADx1-AAx2-ADx4-AEx4-ADx6-AAx2-ADx2-AAx1-ADx2-AEx1-ADx2-AEx10-ADx1-AEx1-ADx1-AEx3-ADx9-AAx7-ADx1-AAx1-ADx3-AEx8-ADx6-AAx1-AEx20-ADx1-AEx3-ADx7-AAx7-ADx7-AEx9-ADx6-AEx15-AFx1-AEx3-AFx1-AEx4-ADx6-AAx2-ABx2-AAx1-ADx10-AEx11-ADx4-AEx15-AFx1-AEx1-AFx1-AEx1-AFx2-AEx4-ADx5-AAx2-ABx1-AAx1-ADx11-AEx12-ADx2-AEx18-AFx1-AEx1-AFx1-AEx3-ADx1-AEx1-AAx1-ADx4-AAx4-ADx11-AEx12-ADx2-AEx10-AFx1-AEx2-AFx1-";
        break;
      case 5:
        theFullImage = "AA#DACEDC,AB#EEE0E4,AC#FBF8F2,AD#DDBFC0,AE#BCA1A3,AF#CBB3B1,AG#A69292;AAx20-ABx14-ACx23-ABx3-AAx19-ABx17-ACx22-ABx2-AAx19-ABx17-ACx23-ABx1-AAx18-ABx1-AAx1-ABx6-ACx33-ABx1-AAx19-ABx17-ACx24-AAx19-ABx12-AAx1-ABx6-ACx20-ABx2-AAx19-ABx12-AAx1-ABx7-ACx18-ABx3-AAx19-ABx12-AAx2-ABx6-ACx18-ABx3-AAx18-ABx14-AAx2-ABx10-ACx11-ABx5-AAx18-ABx14-AAx3-ABx10-ACx9-ABx6-AAx18-ABx13-AAx6-ABx8-ACx8-ABx7-AAx18-ABx9-AAx15-ABx3-ACx3-ABx12-AAx18-ABx4-AAx21-ABx3-ACx1-ABx13-AAx17-ABx1-AAx4-ADx4-AAx18-ABx16-AAx16-ABx1-AAx2-ADx8-AAx19-ABx14-AAx18-ADx9-AAx22-ABx11-ADx1-AAx16-ADx11-AAx23-ABx9-AEx3-ADx1-AEx1-ADx1-AAx10-ADx14-AAx15-ADx1-AAx6-ABx8-AEx5-ADx2-AAx9-ADx11-AAx3-ADx1-AAx1-ADx2-AAx7-ADx5-AAx6-ABx8-AEx5-AAx11-ADx10-AAx1-ADx1-AAx2-ADx5-AAx1-ADx7-AAx1-ADx2-AAx2-ADx3-AAx2-ABx7-AEx5-ADx1-AAx1-ADx2-AAx7-ADx30-AAx2-ADx4-AAx1-ABx7-AEx1-AFx1-AEx2-ADx1-AAx1-ADx1-AEx3-ADx1-AEx1-AAx4-ADx7-AAx2-ADx27-AAx5-ABx3-AEx1-ADx1-AEx5-ADx3-AAx1-AEx1-AAx3-ADx8-AAx3-ADx26-AAx2-ADx3-AAx1-ABx2-AEx7-AAx1-ADx2-AEx2-ADx1-AAx2-ADx8-AAx1-ADx9-AEx6-ADx2-AEx2-ADx12-AFx1-AEx1-AAx3-AEx12-ADx1-AEx1-ADx18-AEx12-ADx9-AFx1-AEx9-ADx1-AEx10-ADx5-AAx1-ADx12-AEx14-ADx6-AEx22-ADx4-AAx2-ADx12-AEx15-ADx5-AEx19-ADx1-AEx1-ADx2-AAx3-ADx10-AFx1-ADx2-AEx17-AFx4-AEx10-AGx1-AEx8-ADx4-AAx2-ADx10-AFx4-AEx17-AFx3-AEx11-AGx1-AEx7-ADx1-AAx1-ADx15-AEx21-AFx4-";
        break;
      case 6:
        theFullImage = "AA#BDBCCB,AB#D2CAD7,AC#E8D9E1,AD#C4AFAE,AE#AD9292,AF#A08081,AG#8B6467,AH#FAF5F0,AI#905449,AJ#763F33,AK#5C2720,AL#3A110F,AM#5C515A,AN#3A415F,AO#533841,AP#222D4B,AQ#12183A,AR#160710,AS#332831,AT#A29FAD,AU#767476,AV#B3866D,AW#A0775D,AX#716454,AY#1D251C;AAx21-ABx11-ACx6-ABx1-ADx1-AEx1-AFx2-AGx3-AEx1-ACx1-AHx3-ACx9-AAx20-ABx12-ACx5-AEx1-AGx5-AIx5-ADx1-AHx3-ACx8-AAx20-ABx11-ACx5-AEx1-AGx4-AIx8-AEx1-AHx2-ACx8-AAx18-ABx13-ACx4-ABx1-AGx4-AIx4-AJx5-AIx1-ABx1-ACx9-AAx17-ABx14-ACx4-AAx1-AGx1-AIx7-AJx1-AKx1-AJx1-AIx1-AJx2-AEx1-AHx1-ACx8-AAx17-ABx14-ACx4-ABx1-AGx1-AIx8-AJx3-AIx2-ABx2-ACx8-AAx15-ABx16-ACx4-ABx1-AIx10-AJx1-AIx3-AAx1-ACx9-AAx14-ABx17-ACx5-AFx1-AIx8-AJx1-AKx4-ADx1-ACx9-AAx13-ABx20-ACx3-AEx1-AIx7-AJx1-ALx3-AKx2-ABx1-ACx9-AAx13-ABx22-ADx1-AGx1-AIx7-ALx1-AKx2-ALx1-AKx1-AGx1-ACx10-AAx12-ABx19-AAx2-AEx1-AFx1-AMx1-ANx1-AOx1-AJx2-AIx3-AJx1-ALx5-AAx1-ACx10-AAx12-ABx14-AAx2-AEx2-AFx1-AGx4-AMx1-ANx2-AOx1-AJx4-AKx1-ALx4-AOx1-ACx11-AAx12-ABx11-AAx1-AEx1-AFx1-AGx9-AMx1-ANx2-APx1-AOx1-AJx1-AKx2-ALx4-AJx1-ADx1-ACx9-ABx2-AAx8-ABx13-AAx1-AEx1-AFx2-AGx10-AMx1-APx1-AQx1-APx1-ANx1-AKx4-ALx1-ARx2-AOx1-ADx1-ABx1-ACx9-ABx1-AAx8-ABx12-AAx1-AFx4-AGx10-AMx1-ANx1-APx4-AIx2-AJx2-APx1-AQx1-ASx1-AGx2-AEx1-AAx1-ACx5-ABx3-AEx1-AAx7-ABx12-AEx1-AFx4-AGx8-AJx1-AKx1-AJx1-AMx1-ANx3-APx2-AMx2-ANx1-APx1-AQx1-AMx1-AGx1-AIx1-AJx1-AGx1-AFx1-AAx1-ACx2-ABx4-AEx1-AAx2-AEx1-ATx1-AAx1-ABx13-AAx1-AEx1-AFx5-AGx4-AIx1-AGx2-AIx3-AGx1-APx1-ANx3-APx5-AQx1-AMx1-AGx1-AMx1-AKx1-AJx1-AGx2-ADx1-ABx5-AGx3-AEx1-AUx1-AEx2-ABx12-ADx1-AEx1-AGx2-AFx5-AGx8-AFx1-ANx1-APx1-ANx2-APx1-ANx4-APx1-AGx2-AMx2-AIx1-AGx3-AAx1-ABx4-AEx1-AGx1-AMx2-AGx1-ADx1-AFx1-AAx1-ABx11-AEx1-AFx1-AGx2-AFx5-AGx8-AFx1-APx1-ANx9-AMx1-AGx1-AIx1-AGx5-AFx1-ABx4-AMx1-AGx1-AMx1-AGx1-AMx1-ADx1-AAx4-ABx8-AAx1-AEx1-AFx1-AGx1-AIx1-AGx1-AFx4-AGx9-APx1-ANx9-AMx1-AGx1-AMx1-AGx6-ABx4-AGx1-AMx4-AUx1-ADx1-AEx2-AAx2-ADx1-AAx1-ABx5-AAx1-AEx1-AFx1-AGx1-AJx1-AGx1-AFx4-AGx9-APx1-ANx9-AMx1-AGx1-AJx1-AGx5-AFx1-ABx4-AMx1-AGx2-AMx1-AEx1-AAx1-AEx1-AGx2-AUx1-AFx1-AEx2-ABx3-ADx1-AEx2-AFx1-AVx1-AGx1-AJx1-AGx1-AFx3-AGx1-AFx1-AGx8-ANx5-APx1-ANx5-AGx1-AJx1-AIx1-AGx4-AFx1-AEx1-AAx1-ABx2-AMx1-AGx1-AUx1-AMx4-AFx1-AEx1-ATx2-AUx2-ABx2-AAx1-AMx1-AEx2-AFx1-AVx1-AWx1-AKx1-AGx1-AFx2-AGx2-AFx1-AGx8-ANx11-AGx1-AJx1-AIx1-AGx5-AOx1-AFx1-ABx2-AIx1-AGx1-AMx2-ASx2-AMx1-AEx1-ATx1-AUx1-AGx1-AMx1-AUx1-ADx3-AMx1-AGx1-AFx1-AVx2-AFx1-AKx1-AGx1-AFx1-AGx2-AFx2-AGx8-AQx1-APx1-ANx1-APx1-ANx6-APx1-AGx1-AJx1-AIx1-AGx4-AMx1-ASx1-AOx1-AEx2-AIx1-AMx2-AGx2-AMx1-AUx2-AMx4-AGx1-AMx2-AEx2-AFx1-AVx1-AFx1-AVx2-AIx1-AFx2-AGx11-AIx1-AQx1-APx2-ANx1-APx1-ANx2-APx4-AMx2-AIx1-AGx4-AMx1-ASx3-ARx1-AMx2-AGx1-AUx2-AMx1-AFx1-AMx1-ASx1-AMx2-ANx1-AMx2-AOx1-AFx1-AUx1-AEx1-AVx1-AFx2-AWx1-AGx1-AFx2-AGx11-AMx1-AQx1-APx3-ANx1-APx4-AQx2-AMx2-AIx1-AGx4-AMx1-ASx3-AKx1-AFx1-AMx3-ANx1-AOx1-AMx1-APx2-ANx1-AUx1-AMx2-AOx2-AGx1-ADx1-AFx1-AVx2-AFx1-AGx1-AFx2-AGx12-AOx1-APx1-ANx9-APx1-ANx1-AIx1-AJx1-AGx4-AMx1-ASx3-AOx1-AEx1-AMx4-APx1-AMx1-APx1-ASx1-AMx1-ANx1-AMx2-ANx1-AUx1-AEx1-ADx1-AFx1-AVx2-AGx2-AFx2-AGx12-ASx1-APx1-ANx9-APx1-AQx1-AMx1-AJx1-AIx1-AGx4-ASx3-AOx1-AEx1-AMx1-AOx1-APx1-AMx1-APx2-ANx1-AMx1-AOx1-ASx1-AMx2-AGx1-AFx1-AEx2-AFx2-AVx1-AGx1-AJx1-AGx14-ASx1-APx1-ANx5-APx1-ANx2-APx1-AQx1-ARx1-AMx1-AJx1-AIx1-AGx3-AXx1-ASx1-ARx1-ASx2-AMx1-AIx1-AMx1-ANx1-AMx1-ASx1-APx1-AYx1-ASx1-AMx1-ASx1-ATx1-AMx4-AEx1-AFx2-AVx1-AWx1-AKx1-AGx1-AFx1-AGx12-AQx2-ANx5-APx1-ANx2-APx2-AQx1-AOx1-AJx1-AIx1-AGx4-ARx1-AQx2.";
        break;
      case 7:
        theFullImage = "AA#BBBED4,AB#D5CDD9,AC#EBDDE2,AD#FBF4EF,AE#D0AEAE,AF#ECC7C0,AG#C4999A,AH#BB8279,AI#9E7670,AJ#A98E91,AK#84686A,AL#837B84,AM#646778,AN#A09EA6,AO#62565D,AP#B2B0BE,AQ#4B5146,AR#414760,AS#3E4033;AAx20-ABx10-ACx9-ADx15-ACx6-AAx19-ABx10-ACx10-ADx17-ACx4-AAx19-ABx10-ACx9-ABx1-ACx2-ADx15-ACx4-AAx17-ABx11-ACx7-ABx1-AEx3-AFx1-ACx2-ADx14-ACx4-AAx18-ABx10-ACx6-ABx1-AGx1-AEx3-AFx2-ACx2-ADx10-ACx7-AAx17-ABx12-ACx5-AEx1-AGx2-AEx3-AFx2-ACx6-ADx4-ACx8-AAx15-ABx15-ACx4-AGx4-AEx5-AFx2-AEx3-ABx1-ACx1-ADx1-ACx9-AAx15-ABx15-ACx4-AGx4-AEx10-ACx12-AAx16-ABx15-ACx3-AGx1-AHx1-AGx12-ABx1-ACx11-AAx16-ABx16-ACx2-AEx1-AGx8-AHx5-ABx1-ACx11-AAx16-ABx16-ACx2-AEx1-AHx8-AIx5-ABx1-ACx11-AAx12-ABx3-AAx1-ABx17-AEx1-AJx1-AHx7-AIx5-AEx1-ACx12-AAx12-ABx3-AAx1-ABx14-AEx2-AGx1-AJx1-AIx3-AHx4-AIx1-AKx3-AIx2-ABx1-ACx12-AAx11-ABx14-AEx2-AGx2-AHx5-ALx2-AIx6-AKx4-AIx1-ACx13-AAx11-ABx10-AAx1-AEx1-AJx1-AHx10-AKx1-ALx1-AMx1-AKx1-AIx1-AKx6-AGx1-ABx1-ACx2-ABx2-ACx5-ABx4-ANx1-AAx9-ABx10-AEx1-AJx1-AHx12-AKx1-AMx2-ALx1-AKx5-AOx2-AKx1-AEx1-ABx1-ACx6-ABx6-ANx1-AAx2-ANx1-APx1-AAx3-ABx11-AEx1-AHx12-AIx3-AKx1-AMx2-AKx1-AIx1-AHx2-AIx1-AMx1-AOx1-AKx1-AHx2-AGx1-AEx1-ABx2-ACx2-ABx6-ALx1-AIx1-AJx1-ANx1-ALx1-AEx1-APx1-AAx1-ABx10-AEx1-AHx9-AIx2-AHx1-AIx1-AKx2-AIx1-AKx1-AMx4-AKx2-AMx2-AOx1-AIx1-AHx1-AIx1-AOx1-AIx1-AJx1-ABx9-AJx1-AIx1-AKx1-AIx1-AKx1-AEx1-AJx1-AAx1-ABx10-AGx1-AHx12-AIx3-AHx1-AIx1-AMx9-AIx2-AKx3-AIx2-AEx1-ABx7-AMx1-AIx1-AKx1-AIx2-AAx3-ABx9-AAx1-AGx1-AHx1-AIx1-AHx10-AIx3-AHx1-AIx1-AMx9-AIx8-ABx7-AIx1-AKx3-AMx1-ALx1-ANx3-AAx1-ABx1-AAx2-ABx4-AAx1-AHx2-AIx1-AHx10-AIx3-AHx1-AIx1-AMx9-AIx8-AGx1-ABx6-AIx2-AKx2-AJx1-AAx1-ANx1-AKx2-ALx1-AJx3-ABx3-AAx1-AGx2-AHx1-AIx1-AHx10-AIx3-AHx1-AKx1-AMx9-AKx1-AIx1-AKx1-AIx5-AJx1-AAx4-ABx2-AKx1-AIx1-AJx1-AKx1-AIx1-AKx2-AJx1-ANx3-AJx1-ALx1-ABx2-AAx1-AKx1-AGx2-AHx1-AIx2-AHx8-AIx4-AHx1-AKx1-AMx9-AKx1-AIx1-AKx1-AIx5-AGx1-AKx1-ALx1-AKx1-AJx1-ABx2-AKx1-AIx1-AMx1-AKx1-AOx2-AKx1-ANx1-AEx1-ALx1-AJx1-AIx1-ALx1-AAx3-AIx1-AGx3-AHx2-AGx4-AHx4-AIx4-AHx1-AKx1-AMx4-AOx1-AMx5-AIx1-AKx1-AIx5-AKx1-AOx3-AKx1-AAx1-AEx1-AKx6-AIx1-AJx1-AIx1-AKx3-ALx1-AIx2-AGx5-AHx1-AIx1-AGx4-AHx4-AIx3-AHx2-AMx9-AOx2-AIx1-AKx1-AIx5-AKx1-AOx1-AQx1-AOx1-AQx3-AKx2-AIx2-AJx1-AKx1-AJx1-AKx1-AOx1-AKx3-AIx1-AKx2-AJx1-AGx4-AHx1-AIx1-AGx1-AHx1-AGx2-AHx4-AIx3-AHx2-AOx2-AMx2-AOx1-AMx3-AOx3-AKx3-AIx4-AKx1-AOx4-AQx2-AIx1-AKx3-AMx1-AKx1-AJx1-AOx2-AKx1-ALx1-AKx1-AIx1-AKx2-AHx1-AGx4-AHx10-AIx3-AHx2-AOx1-ARx1-AOx9-AKx3-AIx4-AKx1-AOx6-ANx1-AKx4-AOx1-AIx1-AOx2-AKx2-AIx2-AKx1-AIx1-AGx4-AHx10-AIx4-AHx1-AIx1-AOx7-AQx1-AOx1-AQx1-ARx1-AOx1-AKx2-AIx4-AKx1-AOx1-AQx3-AOx2-AJx1-AKx1-AOx1-AQx1-AKx1-AQx1-AOx1-AKx1-AIx1-AOx2-AKx1-AIx2-AGx1-AEx1-AGx3-AHx5-AIx1-AHx4-AIx4-AHx1-AIx1-AOx1-AMx9-AOx2-AIx1-AKx1-AIx4-AKx1-AQx1-ASx1-AQx2-AOx2-AKx2-AMx1-AOx1-AKx1-AOx3-AMx1-AKx1-AOx1-AEx1-AIx1-AHx2-AGx3-AHx1-AIx1-AHx9-AIx4-AHx1-AIx1-AOx1-AMx2-AKx2-AMx1-AOx1-AMx1-AKx1-AMx1-AOx1-AQx1-AKx2-AIx4-AKx1-AQx3-ASx1-AQx1.";
        break;
      case 8:
        theFullImage = "AA#DDD1DD,AB#F3E3E2,AC#FAF9F3,AD#CCB8B9,AE#E7C4C6,AF#C3A7A3,AG#E3ADAD;AAx18-ABx14-ACx26-ABx2-AAx18-ABx13-ACx28-ABx1-AAx16-ABx13-ACx30-ABx1-AAx16-ABx12-ACx30-ABx2-AAx16-ABx20-ACx21-ABx3-AAx16-ABx22-ACx19-ABx3-AAx16-ABx23-ACx17-ABx4-AAx16-ABx23-ACx16-ABx5-AAx16-ABx13-AAx3-ABx11-ACx12-ABx5-AAx15-ABx14-AAx3-ABx11-ACx9-ABx8-AAx14-ABx15-AAx3-ABx12-ACx3-ABx13-AAx14-ABx16-AAx2-ABx28-AAx14-ABx14-AAx4-ABx1-AAx8-ABx19-AAx14-ABx12-AAx15-ABx19-AAx15-ABx6-AAx20-ABx19-AAx15-ABx2-AAx24-ABx19-ADx1-AAx3-ADx1-AAx12-AEx4-AAx23-ABx16-ADx7-AAx9-AEx5-AAx25-ABx14-ADx5-AAx1-ADx1-AAx8-ADx3-AEx3-AAx27-ABx12-ADx5-AAx9-ADx4-AEx4-AAx20-AEx3-AAx4-ABx11-ADx6-AAx1-ADx2-AAx2-ADx1-AAx2-ADx3-AEx5-AAx20-ADx1-AEx3-AAx4-ABx1-AAx3-ABx6-ADx4-AAx2-ADx6-AAx2-ADx3-AEx4-AAx18-ADx2-AEx1-ADx1-AEx1-AAx1-AEx1-AAx2-AEx1-AAx2-ADx1-AAx6-ABx2-AFx1-ADx3-AFx2-ADx2-AAx1-ADx1-AAx1-ADx6-AEx4-AAx8-AEx1-ADx1-AAx4-ADx12-AAx1-ADx4-AAx2-ADx3-AAx1-ABx2-ADx4-AFx1-ADx2-AAx2-ADx8-AEx1-AAx7-AEx5-ADx17-AEx1-ADx12-AFx1-ADx16-AEx2-AAx5-AEx4-ADx32-AFx1-ADx16-AEx1-AAx6-AEx3-ADx34-AFx1-ADx13-AEx1-ADx1-AEx1-AAx2-AEx6-ADx35-AFx1-ADx1-AFx1-ADx1-AFx1-ADx6-AEx5-AAx3-AEx5-ADx36-AFx3-ADx1-AFx1-ADx6-AEx2-AAx2-ABx2-AAx1-AEx1-ADx40-AFx1-ADx1-AFx3-ADx6-AAx1-AEx2-AAx5-ADx22-AFx1-ADx1-AGx2-ADx12-AFx1-";
        break;
      case 9:
        theFullImage = "AA#D2C4D0,AB#E2D3E0,AC#F9E9E7,AD#F2CBC8,AE#DCB8B7,AF#C6ABA8,AG#E7A3A0,AH#B79C98;AAx2-ABx21-ACx37-AAx3-ABx1-AAx1-ABx23-ACx32-AAx3-ABx18-AAx3-ABx10-ACx26-AAx5-ABx16-AAx4-ABx10-ACx25-AAx5-ABx16-AAx4-ABx10-ACx25-AAx6-ABx15-AAx5-ABx9-ACx25-AAx4-ABx17-AAx6-ABx9-ACx24-AAx3-ABx18-AAx6-ABx9-ACx24-AAx3-ABx19-AAx5-ADx2-ABx7-ACx24-AAx2-ABx20-AAx5-ADx2-ABx1-AAx4-ABx2-ACx24-ABx3-AAx2-ABx17-AAx12-ABx1-ACx25-ABx3-AAx2-ABx15-AAx14-ABx1-ACx25-ABx4-AAx2-ABx12-AAx16-ABx1-ACx25-ABx4-AAx2-ABx10-AAx2-ADx1-AAx16-ABx4-ACx21-ABx4-AAx3-ABx1-AAx2-ABx3-AAx2-AEx3-ADx2-AAx20-ABx2-ACx18-ABx4-AEx4-AAx5-AEx5-ADx3-AAx22-ACx17-ABx4-AAx2-AEx1-AFx1-AEx2-AAx1-AEx7-ADx2-AAx24-ABx1-ACx8-ABx1-ACx6-ABx1-AAx2-ABx1-AAx1-AFx1-AEx1-AFx1-AEx10-ADx1-AAx1-AEx2-AAx23-ACx7-ABx3-ACx2-ABx1-ACx2-AAx1-AFx2-AAx2-AEx1-AFx2-AEx14-AAx23-ACx3-ABx1-ACx2-ABx1-AAx1-ABx3-AAx3-ABx1-AFx1-AAx2-AFx1-AEx1-AAx1-AEx1-AAx1-AEx13-AAx24-ABx2-AAx2-ABx1-ACx1-ABx3-AAx4-ABx1-AAx1-AFx2-AEx1-AFx3-AEx15-AAx24-ABx1-AAx14-AFx2-AAx2-AFx3-AEx12-ADx1-AAx18-AEx1-AAx6-ABx3-AAx8-AEx1-AAx2-AEx1-AFx2-AEx1-AAx2-AFx2-AEx11-AAx17-AEx1-AAx1-AEx6-AAx2-ABx1-AAx8-AEx6-AFx3-AAx2-AFx2-AEx8-ADx3-AAx12-AEx14-AAx1-ABx2-AAx2-AEx11-AFx5-AEx10-ADx1-AAx9-AEx21-AAx2-AEx7-AFx2-AEx3-AFx5-AEx6-AFx1-AEx3-ADx1-AAx9-AEx11-AFx2-AEx17-AFx10-AEx8-ADx3-AAx5-ADx3-AAx1-AEx11-AFx2-AEx18-AFx11-AEx6-ADx2-AAx5-ADx2-AEx6-AFx2-AEx1-AFx1-AEx5-AFx2-AEx7-AFx1-AGx5-AFx1-AEx2-AFx7-AHx1-AFx4-AEx6-ADx3-AAx2-ADx3-AEx5-AFx2-AEx4-AFx1-AEx3-AFx3-AEx5-AFx1-AGx10-AFx12-AEx2-ADx1-AEx3-AAx3-ADx4-AEx6-AFx3-AEx3-AFx2-AEx2-AFx4-AEx3-AFx1-AGx13-";
        break;
      case 10:
        theFullImage = "AA#B8BACE,AB#A8ABBE,AC#CCC8DC,AD#E3D6D4,AE#F4E4E6,AF#9190A3,AG#8D7C89,AH#7F6471,AI#8D595A,AJ#AA6252,AK#9B4F43,AL#DAC2C1,AM#793E34,AN#AE8784,AO#A27372,AP#B89C93,AQ#CEB0A7,AR#644A41,AS#413E41,AT#5E5A67,AU#444A5F;AAx15-ABx1-AAx13-ACx18-ADx11-AEx2-AAx15-AFx1-AAx2-AFx1-AGx2-AHx2-AGx1-AFx1-AAx5-ACx17-ADx12-AEx1-AAx15-AFx2-AHx2-AIx7-AGx1-ACx2-AAx1-ACx17-ADx13-AAx15-AHx2-AIx11-ACx19-ADx1-ACx2-ADx10-AAx14-AGx1-AHx1-AIx1-AJx5-AIx1-AKx4-AIx2-ALx1-ACx1-AAx1-ACx15-ADx1-ACx3-ADx9-AAx12-ABx1-AAx1-AGx1-AIx1-AJx6-AIx2-AKx2-AIx1-AKx1-AIx1-ACx1-ADx1-AAx1-ACx19-ADx9-AAx12-ABx1-AAx1-AGx1-AIx1-AJx5-AIx2-AKx1-AMx2-AIx3-ANx1-ADx1-ACx16-ADx13-AAx11-AFx1-AAx2-ANx1-AJx5-AIx4-AMx1-AIx4-AJx1-AOx1-ACx16-ADx5-ACx1-ADx7-AAx14-ABx1-ANx1-AJx1-AIx1-AJx4-AIx9-ANx1-ACx14-ADx8-ACx1-ADx5-AAx2-ABx1-AAx8-ABx1-AAx2-AFx1-AGx1-AJx9-AIx1-AJx1-AKx1-AMx2-AIx1-ALx1-ACx15-ADx3-ACx1-ADx9-AAx15-AFx1-AJx1-AIx1-AJx7-AIx1-AKx1-AMx4-ANx1-ACx14-ADx4-ACx1-ADx2-ACx1-ADx6-AAx10-ACx1-AAx1-AFx1-AAx2-ABx1-AAx1-ANx1-AJx7-AIx1-AMx5-ANx1-ADx1-ACx14-ADx5-ACx1-ADx1-ACx1-ADx5-AAx12-AGx1-ABx1-AFx1-AAx3-ANx1-AIx1-AJx5-AIx1-AMx1-AKx1-AMx3-AHx1-ALx1-ANx2-APx1-AQx1-ALx1-ADx4-ACx5-ADx2-ACx1-ADx2-ACx1-ADx1-ACx1-ADx5-AAx4-ACx1-AAx6-AFx1-AGx3-AAx3-AHx2-AJx5-AIx1-AMx3-ARx3-ANx1-AGx4-ANx4-AQx1-ADx3-ACx1-ADx5-ACx3-ADx6-AAx8-ABx1-AFx1-AAx1-AFx3-AGx1-AAx1-ABx1-ANx1-AHx3-AIx2-AJx1-AIx2-AMx3-ARx4-AHx1-AGx5-ANx4-APx1-AQx1-ADx1-ACx2-ADx6-ACx1-ADx5-AAx5-ACx1-AAx3-ABx1-AAx2-AFx1-AGx3-ANx2-AGx1-AHx2-AIx4-AKx1-AMx2-ARx3-ASx2-AHx1-AOx1-AGx1-AHx1-AGx3-ANx5-ALx1-ADx1-ACx1-ADx4-ACx1-ADx1-ACx1-ADx5-AAx5-ACx1-AAx2-ACx1-ABx2-AAx2-AGx2-ANx3-AGx1-ATx1-AHx2-ARx1-AKx4-AMx2-AIx1-ARx1-ASx2-AIx1-AOx1-AHx1-AIx1-AHx2-AGx1-ANx6-ADx1-ACx1-ADx12-ACx1-AAx2-ABx1-ACx2-AAx2-ACx1-AAx3-APx1-ANx6-AHx3-ASx1-ARx1-AIx4-ANx1-AHx1-ATx1-ASx2-AGx1-AOx1-AHx2-AGx1-ANx8-AQx1-ADx5-ACx1-AAx1-ACx3-ADx3-ACx3-AAx1-ACx2-AAx1-ACx3-AAx1-AFx1-ANx7-AGx1-ATx1-AUx1-ASx1-ATx3-AHx1-AGx2-ATx4-AHx1-AOx1-AHx1-AIx1-ANx8-AOx1-ANx1-ACx4-ALx1-AHx6-AGx1-AFx1-ACx3-AAx2-ACx1-AAx1-ACx2-AAx1-ANx10-AHx1-ATx2-AHx2-ATx6-AHx3-AGx1-AHx1-AIx1-AOx1-ANx7-AOx2-ALx1-AAx1-ABx2-AGx1-AHx8-ACx1-AAx1-ABx2-AFx1-AAx2-ACx1-AAx1-ANx11-AGx1-AHx7-ATx1-AHx5-AGx1-AIx2-AOx1-ANx8-AOx1-ANx1-AIx1-ARx5-AIx3-AHx3-ABx2-AFx1-ABx2-AFx1-ABx1-AAx1-ANx11-AHx3-ATx1-AHx5-ATx2-AHx2-ATx2-AGx1-AHx1-AIx1-AOx1-ANx8-AOx2-AHx1-ARx11-AFx1-ABx1-AAx2-ABx1-AFx3-ANx9-AOx1-AIx3-AOx1-AHx1-ATx3-AHx7-ATx1-AHx1-AIx2-AOx1-ANx9-AOx1-AHx1-ARx11-AAx4-AGx1-ATx1-AHx1-AGx1-ANx7-AJx1-AIx2-ANx1-AOx2-ANx2-ATx4-AHx3-ATx3-AHx1-ATx1-AHx1-AIx1-AOx1-ANx10-AGx1-ARx11-ACx2-AAx1-ABx1-AGx1-AHx1-AGx1-AFx1-ANx8-AOx1-ANx2-AOx1-AJx1-AOx1-ANx1-AHx1-ATx3-AHx2-ATx6-AHx1-AIx1-AOx1-ANx11-ARx11-AFx1-AGx1-AHx2-ATx2-ARx1-AHx1-ANx8-AOx1-ANx3-AOx2-ANx2-ARx1-ATx11-AHx1-AOx2-ANx10-ARx11-AIx1-AHx3-ARx3-ASx1-AJx2-ANx10-AOx2-ANx2-AHx1-ATx7-AHx1-ATx2-AUx1-AIx1-AOx2-ANx10-ARx11-AGx1-AHx1-AIx1-AHx1-ARx4-AHx1-AJx1-AIx1-AJx1-ANx8-AOx3-ANx1-AOx1-ATx1-ASx1-ATx8-ASx1-ARx1-AGx1-AOx1-ANx10-ARx11-AIx1-ARx2-AHx1-AIx1-ARx4-ANx1-AIx2-AJx1-ANx4-AJx1-ANx2-AOx2-AJx1-ANx2-AHx1-AUx1-ATx2-AUx1-ASx7-AHx1-AOx1-ANx10-ARx11-AIx1-ARx2-AHx2-AIx4-ANx2-AHx2-AOx1-ANx6-AOx3-ANx2-AGx1-ATx1-ASx7-AUx1-ASx1-ATx2-AGx1-ANx10-AHx1-ARx2-ASx2-ARx3-ASx1-";
        break;
      case 11:
        theFullImage = "AA#B8B7CD,AB#9F9EB0,AC#CCC7D9,AD#E5DADF,AE#898693,AF#716A6E,AG#6B5B56,AH#4F4D4D,AI#423E32,AJ#9C7671,AK#B48D89,AL#D6B2AC,AM#87554B,AN#2E2F27,AO#AA6452,AP#964935,AQ#C0846F,AR#1F2114,AS#EFC5B5,AT#7B3926,AU#DAA58E,AV#0C0F07,AW#5E1E0E,AX#F9E0C7,AY#714E3A,AZ#8D6F59;AAx27-ABx1-AAx8-ACx23-ADx1-AAx27-ABx1-AAx8-ACx24-AAx24-ABx1-AAx2-ABx1-AAx5-ABx1-AAx1-ACx25-AAx24-ABx1-AAx2-ABx1-AAx3-ACx2-AAx3-ACx24-AAx27-ABx2-ACx1-AAx1-ACx2-AAx1-AEx1-AAx4-ACx21-AAx25-ABx1-AAx2-ABx1-AAx1-ACx3-AAx1-AEx1-ABx1-AAx1-AEx2-ABx1-AAx1-ACx18-ADx1-AAx25-AEx1-AAx2-ABx1-AAx1-ACx2-AAx1-ABx1-AEx1-AFx1-AEx1-AFx1-AEx1-ABx1-AAx1-ACx19-AAx24-ABx1-AFx1-ABx1-AAx1-ABx1-AAx2-ACx2-AAx1-AFx3-AGx1-AEx1-AAx2-ACx1-AAx1-ACx17-AAx24-AFx1-AEx3-ABx1-AAx2-ABx1-AAx2-AFx3-AGx2-AEx1-AAx1-AEx1-ABx1-AAx2-ABx1-AAx1-ACx3-AAx2-ACx8-AAx18-ACx1-AAx3-ABx1-AAx1-AEx5-ABx1-AFx2-ABx4-AFx1-AGx1-AEx1-ABx2-AFx1-AEx2-ABx1-AEx1-ABx1-ACx3-AFx1-ABx2-ACx7-AAx17-ACx3-AAx2-ABx1-AAx2-AEx1-AFx1-AEx1-AFx1-AEx1-AFx1-ABx1-AFx1-AHx1-AEx2-AFx3-AGx2-AEx1-ABx3-AEx2-AAx3-AFx3-ABx1-AAx1-ACx5-AAx19-ACx1-AAx2-ABx2-AAx2-AEx1-AFx3-AGx1-AFx1-AEx2-AGx2-AFx2-AGx1-AHx1-AGx1-AEx1-ABx1-AEx1-AFx1-AGx1-AFx3-ABx2-AEx1-AFx1-ABx1-AAx1-ACx2-ADx3-AAx12-ACx1-AAx2-ACx1-AAx3-ACx1-AAx6-AEx1-AFx1-AGx1-AFx3-ABx2-AFx2-AGx1-AFx2-AGx2-AFx1-AGx1-AFx1-AGx1-AFx2-AGx1-AHx1-AFx1-AEx1-ABx3-AAx3-ADx3-AAx7-ACx9-AAx2-ACx2-AAx1-ACx2-AAx2-ABx1-AEx1-AFx1-AGx1-AFx1-AGx2-AEx1-AAx1-ABx1-AFx4-AGx1-AFx1-AEx1-AGx1-AHx2-AGx2-AFx1-AHx1-AGx1-ABx1-AAx1-AEx1-ABx1-AAx1-AEx1-ABx1-ADx3-AAx6-ACx11-AAx2-ACx1-AAx1-ACx2-AAx1-ABx1-AAx1-AFx3-AGx1-AIx1-AGx2-AEx2-AFx1-AEx1-AFx2-AGx3-AIx2-AGx1-AFx2-AGx2-AFx1-ABx1-AAx1-AFx1-ABx2-AFx1-AEx1-ABx1-ACx1-ADx1-ABx3-AAx4-ACx8-AAx2-ABx2-AAx3-ABx1-AEx1-AFx2-AEx2-AFx1-AGx1-AIx1-AGx4-ABx2-AFx3-AHx1-AGx1-AHx3-AIx1-AHx1-AGx1-AFx1-AEx1-ABx1-AAx1-AEx1-AFx1-AGx1-AEx1-AFx1-AEx1-ACx1-ADx1-AGx4-AFx1-AJx1-AKx1-ALx1-ACx6-ABx2-AEx1-ABx1-AEx1-ABx1-AAx2-AEx1-ABx1-AEx1-AFx1-AGx1-AHx2-AFx1-AHx2-AGx2-AHx1-AEx1-ABx1-AFx1-AGx2-AHx1-AIx2-AGx2-AIx1-AEx2-AFx3-AEx1-AGx3-AFx1-ABx1-ACx1-ADx1-ACx1-AGx3-AMx5-ALx1-ADx3-ACx2-ABx2-AAx2-ABx3-AAx1-ABx1-AGx1-AHx1-AIx3-AHx1-AFx1-AIx2-AHx1-AGx1-AHx2-AGx1-AFx1-AGx2-AIx1-AHx1-AIx2-AHx2-AFx2-AGx2-AFx5-AEx1-AFx1-AAx2-AHx1-AMx9-AKx1-ALx1-ADx2-AAx5-AEx1-AGx1-AFx2-ABx1-AFx1-AIx1-ANx3-AHx1-AGx1-AHx1-ANx1-AHx2-ANx1-AIx2-AGx3-AIx2-ANx1-AIx1-AGx1-AIx1-AGx2-AFx2-AIx1-AGx1-AIx1-AFx4-AEx1-AHx1-AIx1-AOx5-AMx4-APx1-AMx1-AQx1-ALx3-AAx3-AFx2-AEx1-AFx1-AGx1-AIx5-AHx1-AGx2-AIx1-AHx2-AIx1-AHx3-AIx1-AHx1-ANx2-ARx1-ANx1-AHx1-AIx3-AGx2-AHx1-AIx2-AHx1-AGx1-AEx1-AFx1-AHx1-AIx2-AOx6-APx5-AMx1-AJx1-ALx1-ADx1-AKx1-AEx2-AKx1-ALx3-ADx2-ALx1-AHx1-AIx3-AGx2-AIx2-AHx1-ANx1-AIx1-AHx2-AIx1-AHx1-ANx1-ARx1-ANx5-AIx3-ANx2-AIx2-AGx1-AFx1-AGx1-AHx1-AIx2-AOx6-APx5-AMx2-AKx1-ALx1-ASx1-AKx1-AJx6-ALx1-ADx2-AKx1-AIx1-ANx1-AIx1-AGx1-AHx1-ANx1-AIx1-ANx1-ARx1-AIx1-AHx1-AIx1-AHx1-ANx6-ARx1-ANx6-AHx1-AGx1-AHx1-AGx3-AHx1-AOx4-APx6-ATx2-AMx1-AUx1-ALx1-AUx1-AJx8-ALx4-AGx1-AIx2-AGx1-AHx1-AIx2-ARx1-AIx1-AGx1-AIx1-AHx1-ANx4-ARx6-ANx2-AIx2-AHx2-AGx3-AHx1-AOx5-APx4-ATx3-APx1-AMx1-AQx1-AUx1-AKx1-AJx1-AMx1-AFx3-AJx2-ALx2-AUx2-ALx1-AJx1-AIx1-AHx1-AGx1-AIx2-ARx1-ANx1-AGx1-AHx2-ANx5-ARx3-AVx1-ARx1-ANx1-AIx2-AHx2-AGx3-AHx1-AIx1-AOx5-APx10-AMx1-AKx1-AJx1-AGx1-AFx4-AMx1-AKx1-ASx1-ALx1-AKx2-ALx1-AJx1-AIx1-AHx2-AIx1-ARx1-ANx1-AHx2-AIx1-ARx7-AVx2-ARx2-ANx1-AIx1-AHx2-AIx1-AHx4-APx2-AOx2-APx9-ATx1-AWx1-ATx1-AUx1-AJx1-AGx2-AFx2-AMx1-AJx2-ASx1-AXx1-ASx1-ALx1-AKx1-AUx1-AJx1-AHx1-AGx1-AIx1-ANx2-AHx2-ANx1-ARx4-AVx1-ARx1-AVx2-ARx3-ANx1-AIx3-AHx3-AIx1-AHx1-APx2-AOx5-APx5-ATx1-AWx1-ATx1-AMx1-AKx1-AJx1-AMx1-AGx2-AMx1-AOx1-AJx2-AKx1-ASx2-AXx1-ALx1-AKx1-ALx2-AGx1-AHx1-ANx1-AHx1-AGx1-AIx1-ARx1-ANx1-ARx7-ANx2-ARx1-ANx1-AIx4-AHx4-APx2-AOx3-APx7-AWx1-ATx2-AWx1-AYx2-AIx1-AGx2-AMx1-AJx4-ASx1-AXx4-ADx1-ALx1-AJx1-AYx1-AIx1-AGx2-AIx1-ANx2-ARx4-ANx5-ARx1-ANx4-AIx1-AHx4-APx1-AMx1-APx1-AOx1-APx8-ATx1-AWx2-ARx1-ANx2-AHx1-AGx2-AMx2-AJx3-ALx3-AXx2-ADx1-AXx1-AKx1-AGx2-AMx1-AIx2-ANx1-AIx2-AHx1-AIx4-ANx5-AIx1-ANx2-AIx1-AHx1-AIx2-AHx1-AGx1-AHx1-AIx1-APx1-AMx1-APx2-AMx4-APx1-ATx1-AWx1-ARx1-ANx2-AIx2-AGx1-AMx2-AZx1-AJx3-ALx1-ASx2-ADx1-AXx2-ASx1-AXx1-AKx1-AMx1-AJx1-AGx1-AFx3-AJx1-AFx1-AJx2-AKx1-AJx3-AFx2-AGx1-AHx1-AIx3-AHx1-";
        break;
      case 12:
        theFullImage = "AA#C1C1D6,AB#AAA9BA,AC#898590,AD#797682,AE#5D513E,AF#696258,AG#9C9AAA,AH#41413D,AI#2A2F28,AJ#896861,AK#69402D,AL#D4D1E1,AM#563021,AN#1B1E12,AO#B39893,AP#977D79,AQ#CCAA91,AR#E5C3AD,AS#F7D9C0,AT#AE856A,AU#7E5647,AV#90492E,AW#451F10,AX#474B5E,AY#0B0B06,AZ#303852;AAx36-ABx1-AAx4-ABx1-ACx2-ADx1-ACx2-ADx1-AEx3-AFx2-ADx1-AAx1-ACx1-AEx1-AFx1-AEx2-AAx10-AGx1-ACx1-AAx22-ABx2-ACx1-ABx1-AAx2-ABx1-ACx2-ADx1-ACx1-AFx1-AEx5-AFx1-AEx1-ADx1-ABx1-AGx1-AFx1-AEx2-AHx1-AAx10-AFx1-AEx1-ABx1-AAx20-ABx1-ACx1-ABx2-AGx1-ABx2-AGx1-ACx1-AFx1-AHx4-AEx1-AFx1-AHx1-AEx4-ADx1-ACx1-AEx3-AHx1-AAx9-ABx1-AEx3-AFx2-AEx1-AFx1-ADx1-ACx1-ABx1-AAx1-ABx1-AAx10-ABx3-AAx2-AGx1-ACx2-ABx1-ACx1-AEx1-AIx3-AHx1-AFx2-AHx2-AEx2-AIx2-AHx1-AEx4-AAx9-AJx1-AEx4-AKx1-AEx7-ACx1-AAx6-ALx2-AAx4-ABx2-AFx2-ADx1-AFx2-AIx5-AEx1-AFx1-AEx1-AHx1-AEx1-AHx3-AEx4-AIx1-AAx8-ACx1-AEx1-AKx1-AEx12-ACx1-AAx4-ALx4-AAx4-ACx1-AFx1-ADx2-AHx1-AIx2-AEx1-AHx4-AEx2-AHx3-AIx1-AEx3-AHx1-AEx1-AIx1-AAx7-ABx1-AHx1-AMx1-AKx7-AEx6-AFx1-ACx1-AAx7-ABx1-ACx1-AFx1-AEx4-AHx1-AIx6-ANx1-AHx1-AEx3-AIx1-AHx1-AIx2-AHx1-AEx1-AHx1-AEx1-AIx1-AAx7-AFx1-AKx2-AMx1-AKx8-AMx3-AKx1-AEx2-ACx1-AGx2-AOx2-APx1-AFx1-AEx2-AFx1-AEx3-AHx1-AIx3-ANx2-AIx1-ANx1-AIx3-AEx2-AHx3-ANx1-AHx1-AEx1-AHx1-AEx1-AHx1-ALx4-AAx2-ACx1-AEx1-AKx11-AMx2-AKx1-AEx2-AFx1-AJx4-AFx1-AEx1-AFx4-AEx4-AHx3-AIx2-AHx1-AIx2-AHx2-AIx1-AEx2-AHx2-ANx1-AIx1-AEx3-AHx1-ALx5-AAx1-ADx1-AKx1-AMx1-AKx6-AMx1-AKx2-AMx3-AEx7-AFx1-AEx13-AHx4-AIx4-AHx1-AEx2-AHx1-ANx1-AIx1-AEx1-AHx1-AEx1-AIx1-ALx5-ABx1-AEx1-AKx1-AMx2-AKx2-AEx1-AKx2-AMx1-AKx1-AMx4-AKx1-AEx8-AFx2-AEx12-AHx4-AIx3-AEx2-AHx1-AIx2-AHx1-AEx1-AIx1-ANx1-ALx3-AAx2-AEx1-AIx1-AMx4-AIx2-AMx3-AEx1-APx1-AOx1-AQx1-APx1-AJx8-AFx4-AEx15-AHx4-AEx2-AIx2-AEx1-AHx1-ANx2-AAx2-AGx2-ABx1-AFx1-AEx1-AKx1-AEx4-AKx1-AIx2-AFx1-AOx2-ARx4-ASx1-ARx2-ASx5-ARx1-AQx1-AOx2-AFx1-AEx13-AHx4-AEx2-AIx1-AEx1-AFx1-AIx2-ANx1-AEx3-AFx2-AEx1-AFx1-AEx1-AFx5-AEx1-AJx1-APx7-AOx3-AQx1-ARx8-ASx1-AOx1-AEx12-AHx4-AEx3-AFx1-AEx1-AIx1-ANx1-AIx1-AHx2-AEx1-AFx2-AJx1-AFx1-AEx6-AJx1-APx8-AOx2-AQx1-ARx6-AQx3-ASx2-AQx1-AOx1-AFx1-AEx4-AHx9-AEx3-AFx1-AEx2-AFx1-AJx1-AEx3-AFx1-AJx1-AFx2-AEx2-AJx1-AFx1-AEx1-AFx1-APx10-AOx1-AQx1-ARx1-ASx2-ARx8-ASx2-ARx1-AQx1-AOx1-AEx2-AIx3-AEx3-AFx3-AEx4-AJx1-AFx2-AEx1-APx1-AJx3-AEx2-APx1-AOx1-AQx3-AOx1-APx3-ATx2-APx11-AOx5-AQx1-AOx1-APx1-AOx2-AQx1-ARx2-AOx1-AJx2-AOx1-ARx1-AOx2-ARx1-ASx2-AQx1-AOx1-AJx1-AEx6-AQx3-AJx1-AEx1-AFx1-AJx1-ATx1-AOx1-ATx1-APx2-ATx3-APx17-AOx1-AQx2-APx6-AOx2-AQx1-AOx3-ARx1-ASx4-AOx2-AQx2-AOx1-AFx1-AEx2-AOx1-APx3-AEx5-AFx1-APx7-AJx2-APx15-AOx1-APx2-AJx1-APx1-AJx1-APx3-AJx1-APx2-AOx1-AQx1-AOx2-APx1-ARx2-AQx1-AOx1-AQx1-ASx1-ARx1-AOx1-AFx1-AJx3-AEx6-APx7-AJx4-APx10-AOx1-AQx1-ARx1-AQx2-AOx1-APx2-AJx5-APx1-AOx2-APx2-AJx2-APx2-AJx2-APx2-AQx1-ASx2-ARx1-AQx2-ATx1-APx1-AEx4-AJx2-APx6-AJx4-APx4-ATx1-APx6-AOx1-AQx1-ASx3-ARx2-AOx3-APx1-AOx2-APx2-AJx10-APx1-AQx1-ARx1-AOx1-AQx1-ATx1-AJx1-AUx1-AEx4-AFx3-AJx2-APx3-AEx1-AUx1-AJx2-APx4-ATx1-APx9-AOx1-ARx1-ASx3-ARx1-AOx2-APx2-AJx14-AOx1-APx1-AUx2-AEx11-AFx1-AJx2-AEx1-AFx1-AJx2-APx16-AOx1-AQx1-AOx2-APx2-AJx18-AUx4-AEx5-AKx1-AMx2-AIx2-AHx1-AEx3-AFx1-AJx2-APx18-AJx20-AFx1-AVx3-AKx2-AMx3-AWx1-ANx4-AMx1-AKx1-AEx4-AFx2-AJx3-APx13-AJx15-AFx1-AEx1-AHx1-AIx2-ANx3-AKx1-AVx2-AKx1-AWx3-AMx1-AWx5-AMx1-AKx1-AFx2-AHx2-AEx4-AFx3-AJx7-APx1-AJx12-AFx2-AEx2-AIx1-ANx9-AVx2-AKx1-AMx1-AWx6-ANx1-AWx1-ANx1-AMx1-AIx1-AEx1-AHx3-AXx4-AHx3-AEx17-AHx4-AIx2-ANx1-AYx2-ANx7-AIx1-AVx2-AKx1-AWx2-AYx1-AWx4-ANx4-AIx3-AHx3-AXx8-AHx1-AZx1-AIx3-AHx2-AXx4-AHx5-AXx2-AIx1-AHx1-AIx1-ANx1-AYx1-ANx8-AIx1-AVx2-AKx1-AMx1-AWx4-AYx1-ANx4-AIx2-ANx1-AHx3-AXx6-AHx1-AXx1-AHx2-AZx3-AIx1-AZx2-AXx1-AHx9-AIx2-AHx1-ANx1-AYx2-ANx8-AIx1-AVx2-AKx5-AMx1-ANx2-AIx2-ANx1-AYx1-AIx1-AHx3-AXx7-AHx4-AZx2-AIx3-AHx11-AIx2-AHx1-ANx1-AYx1-ANx5-AIx1-ANx2-";
        break;
      case 13:
        theFullImage = "AA#865737,AB#966A48,AC#6A3D25,AD#A4856B,AE#BA997A,AF#DCBB9E,AG#403324,AH#D4A782,AI#ECCEB1,AJ#654F3D,AK#F9E4C8,AL#B2603E,AM#411B0B,AN#201511,AO#BD815B,AP#A6502D,AQ#4C4955,AR#5E2914,AS#232130,AT#79614F,AU#8F4322,AV#31354D,AW#D08F6A,AX#F0B38B,AY#0B0E2C,AZ#6A656B;AAx2-ABx1-AAx1-ACx1-ABx1-AAx1-ACx1-AAx3-ACx3-AAx5-ACx1-AAx2-ACx1-ABx2-AAx2-ACx2-AAx1-ACx4-AAx1-ADx14-ABx1-AEx2-AFx1-ADx1-AEx3-AFx1-ABx1-AGx1-AAx1-ABx1-ADx1-ABx1-AAx1-ADx1-ABx2-ADx3-AAx1-ACx3-AAx6-ACx1-AAx1-ABx1-AAx1-ACx1-ABx1-ADx1-AEx1-ADx2-ABx1-ACx4-ADx12-AEx2-ADx1-ABx1-ADx1-AFx1-AEx2-ADx1-AEx2-ADx1-ABx1-ADx1-ABx1-AEx1-ADx1-ABx1-ADx2-AEx1-ADx1-ABx1-ADx1-AEx4-AFx4-AHx2-ADx1-AAx1-ACx1-AAx1-ADx1-AEx2-ADx2-ABx4-ADx14-AEx1-AFx1-AIx1-AFx6-AHx1-AEx1-AAx1-AEx4-ADx2-ABx1-AAx2-ADx2-AEx2-AHx1-AEx1-ADx2-AEx5-ADx1-AAx7-AJx1-AAx2-ADx21-AHx1-AKx1-AIx1-AFx1-ADx1-AFx4-AHx1-AFx1-AHx1-ADx1-AAx1-ABx1-ADx1-AEx3-AHx1-AEx3-ADx3-ABx2-ADx1-AAx1-ABx2-AAx3-ACx2-AAx1-ABx1-ADx2-ABx1-ADx4-ABx3-ADx12-AIx2-AFx1-ABx1-ADx1-AEx1-ADx1-AEx4-ADx2-AAx2-ABx1-ADx2-ABx1-AAx3-ABx1-ADx1-AEx2-AFx2-AHx1-AFx3-AEx1-ABx1-AAx1-ABx6-ADx3-ABx1-AAx1-ABx2-ADx12-AEx1-AFx1-AEx1-ABx1-AAx1-ADx2-AEx1-AHx1-ADx2-AEx3-ABx1-ACx2-AAx9-ABx1-ADx2-AEx1-ABx2-AAx4-ACx1-AJx2-ABx5-AAx1-ABx1-ADx1-ABx2-ADx12-AEx1-ADx1-ABx1-AEx2-ADx3-AEx2-ADx2-AEx1-AHx1-ABx2-ADx3-ABx5-AAx2-ABx2-AAx6-ACx3-AGx2-AJx1-ABx2-AAx2-ADx18-ABx1-AFx15-AEx1-ABx3-ALx2-AAx1-ALx1-AAx2-ACx2-AAx3-ACx1-AMx1-AGx2-AMx2-ANx1-AMx1-AAx3-ADx17-ABx2-AFx8-AHx6-AEx1-AOx2-ABx1-ALx5-APx1-AAx5-ACx1-AMx3-AGx1-AMx4-ACx2-AAx2-ABx2-ADx13-ABx2-ADx1-AFx1-AHx2-AFx1-AHx2-AFx1-AHx2-AFx1-AHx2-AFx1-AHx3-AOx1-ABx1-ALx5-APx1-ACx3-APx2-ACx1-AMx5-AGx1-AMx2-AAx1-ACx1-AMx1-AGx1-AAx1-ABx2-ADx13-ABx1-ADx1-AHx3-AFx2-AEx1-AHx10-AEx1-AOx1-ABx1-ALx1-AAx1-APx3-ACx2-APx3-ACx1-AMx7-AGx1-ACx1-AGx1-ANx1-AGx1-AQx2-AJx2-ABx1-ADx11-ABx1-ADx1-AEx2-AHx15-AEx1-AOx1-ALx1-AAx1-APx2-AAx1-ACx2-APx3-ACx1-ARx1-AMx7-ANx1-ASx1-AGx1-AQx7-AJx2-ATx1-ABx2-ADx4-ABx3-AHx21-AOx2-AAx3-AUx1-ACx5-AGx2-AMx1-ANx3-ASx1-AQx11-AVx1-ASx1-ANx2-AVx1-AQx1-AJx4-AHx13-AEx8-AHx2-AEx2-AWx2-AOx1-ADx1-AAx1-AGx3-AJx2-ANx2-AVx1-AQx13-AVx1-ASx2-AQx6-AEx1-AHx4-AEx1-AHx1-AEx1-AHx1-AEx1-AHx2-AEx1-AWx1-AEx3-AHx3-AEx2-AHx6-AEx1-ABx1-AAx3-ACx1-AGx1-AQx1-AVx1-AQx13-AVx2-ASx2-AQx6-AHx4-AXx2-AHx8-AXx1-AFx1-AHx5-AEx7-AOx1-ADx2-ABx2-AAx1-AJx1-AGx1-AQx13-AVx1-ASx1-AVx1-ASx1-AQx1-AVx3-AQx3-AEx3-AHx8-AEx2-AHx1-AEx3-ADx1-AOx2-AEx9-AOx1-ADx1-ABx3-AAx1-AJx1-AQx1-AVx1-AQx10-AVx1-ASx1-AVx1-ASx1-AVx1-ASx1-AVx1-AQx4-ASx1-AEx4-AHx1-AEx1-AOx1-ADx2-AEx1-ADx1-AEx7-AHx2-AEx5-AHx3-AEx1-ADx1-ABx1-AAx3-ABx2-AJx1-AQx2-AVx1-AQx1-AVx2-ASx1-AVx2-ASx1-AVx1-ASx7-AVx1-ASx4-AEx4-AHx1-AEx2-AHx8-AEx1-AHx12-AEx2-AAx6-ABx3-AJx1-AGx1-ASx6-AYx5-ASx4-AQx4-AHx2-AXx1-AHx3-AEx1-AHx3-AOx1-AEx1-ADx1-AHx2-AFx1-AXx1-AHx1-AEx2-AHx1-AFx1-AHx1-AEx2-ADx1-ABx1-AAx2-ACx7-AAx5-ABx2-ATx1-AJx1-AQx1-AGx2-AQx1-AJx2-ATx2-AZx7-AHx5-AEx4-AFx5-AHx2-AEx1-ADx2-ABx1-AAx2-ACx13-AGx3-ACx3-AAx9-ABx1-ATx1-ABx8-AAx3-ACx5-AAx6-ACx46-AAx5-ACx1-AAx7-ACx20-AAx3-ACx4-AAx3-ACx17-AAx5-ACx1-AAx7-ACx2-AAx2-ACx7-AAx1-ACx14-AAx1-ACx1-AAx1-ACx18-AAx15-ABx1-ADx14-AEx3-ADx1-AEx1-AHx1-AFx2-AHx3-AEx1-ABx1-AAx6-ABx3-ADx2-ABx1-ADx1-AEx3-ADx1-AFx1-AEx1-ADx5-AEx2-AHx2-AFx9-AHx1-AFx2-AHx1-AFx2-AXx1-AFx4-AHx1-AXx2-AHx2-AFx16-AHx1-AFx1-AXx1-AHx2-AFx1-AHx3-AXx1-AFx1-AHx1-AXx1-AEx1-AFx34-AHx2-AFx3-AHx3-AFx4-AHx1-AXx1-AFx6-AOx1-AEx1-AHx1-AFx4-AHx1-AXx1-AHx2-AFx2-AHx1-AFx33-AXx1-AEx1-AFx3-AXx1-AFx7-AXx2-AHx2-AFx5-AHx1-AFx1-AHx1-AEx2-AHx1-AFx2-AXx1-AHx2-AFx22-AHx1-AFx4-AHx2-";
        break;
      case 14:
        theFullImage = "AA#906442,AB#805334,AC#61472D,AD#4D3422,AE#94775A,AF#A8876D,AG#BF9C84,AH#CDAF96,AI#ECD3BA,AJ#F6E6CC,AK#665A4A,AL#DDA37C,AM#C5785C,AN#EFB890,AO#2B2326,AP#AD714C,AQ#AA5536,AR#72351A,AS#3B1309,AT#D2906B,AU#581F0E,AV#0F0E1E,AW#3E3E48,AX#9D4624,AY#1F2B46,AZ#4B4F67,BA#78695D;AAx2-ABx1-AAx1-ACx1-ABx1-AAx2-ABx1-ADx1-ACx2-ABx2-AAx1-ABx5-AAx1-AEx1-AAx2-ACx2-ABx1-ACx1-ADx1-ACx3-AAx1-AFx3-AGx2-AHx2-AIx2-AHx2-AIx6-AJx1-AIx2-AJx1-AEx1-ACx1-ADx1-ABx1-ACx2-AEx1-AAx1-AEx2-AAx1-AFx2-AEx2-ABx4-AAx5-ABx2-AAx2-ABx1-ACx1-AAx2-AFx1-AAx2-ACx4-AEx1-AFx1-AEx2-AFx3-AGx1-AHx1-AGx2-AFx1-AEx4-AFx3-AEx1-AGx1-AFx1-AHx2-AKx1-ACx2-AFx1-AEx3-AAx1-AGx1-AFx1-AEx1-AFx1-AGx5-AHx5-AGx1-AEx1-ACx2-AFx1-AGx5-AFx1-AAx2-AEx1-AFx4-AEx6-AAx2-AFx2-AEx2-AFx4-AEx1-AFx2-AHx1-AIx3-AGx3-AFx1-AEx1-AAx3-AEx1-AFx1-AGx2-AHx1-AGx1-AFx2-AHx1-AGx4-AEx1-ABx2-AAx4-ABx2-AAx1-AEx1-AFx5-AEx8-AAx4-AEx2-AFx2-AEx1-AFx1-AEx2-AFx1-AGx1-AFx1-AHx3-AGx1-AFx1-AAx2-AFx1-AGx1-ALx3-AHx1-AGx1-AFx1-AGx2-AFx1-AAx3-ABx9-AEx1-AFx6-AEx10-AFx2-AGx3-AIx1-AHx1-AEx2-AGx1-AFx3-AGx6-AFx1-ABx1-AAx1-AFx3-AEx1-AAx2-ABx2-AAx3-ABx2-ACx2-ABx4-AAx1-AEx3-AFx5-AEx6-AFx1-AEx1-AFx2-AEx4-AGx1-AHx1-AEx1-AAx1-AEx2-AGx5-AFx1-AEx2-AFx2-AEx1-ABx3-ACx1-ABx2-AAx1-ABx1-AAx1-ABx4-ACx3-ABx4-AEx1-AFx7-AEx9-AFx3-AEx3-AGx2-AEx1-AAx1-AEx6-AFx2-AGx1-AFx3-AHx1-AFx1-AAx2-AEx3-AAx1-AEx1-AGx3-AEx1-ABx1-ACx5-ABx1-AEx19-AFx2-AEx3-AFx1-AEx2-AAx1-AEx5-AHx9-AGx2-AHx2-ALx1-AHx3-AGx1-AFx1-AGx2-AMx1-AFx1-AAx1-ABx1-AAx1-AEx9-AAx2-AEx8-AFx1-AEx2-AFx2-AEx4-AAx1-AEx5-ANx11-AGx1-AEx3-AFx1-ALx3-AHx2-AEx1-AAx1-ABx2-ACx1-ABx2-AEx7-AAx1-AEx18-AAx1-AEx5-ANx1-AHx8-ALx1-AGx1-AAx4-AEx1-ALx1-AMx1-AEx2-ABx2-AAx3-ABx3-ADx1-ACx1-AAx1-AEx2-AAx1-ABx1-AAx1-AEx6-AFx1-AEx5-AFx2-AEx4-AAx1-AEx5-AHx3-ALx1-AHx1-ALx1-AHx1-ALx1-AHx2-AEx1-AAx7-ABx5-AAx2-ABx3-ACx1-ADx1-AOx1-ACx1-AAx2-ACx1-AAx1-AEx18-AAx1-AEx5-AHx1-ALx9-APx1-AAx4-AQx1-ABx3-ARx1-ABx2-AQx2-ADx1-ASx1-ADx2-ASx1-AOx1-ASx2-AOx1-ABx2-AAx1-AEx24-ALx10-AMx1-AAx3-AQx3-ABx2-ARx1-ABx4-ADx1-ASx2-ADx1-ASx2-AOx1-ASx2-ACx1-ABx2-AAx1-AEx23-ALx1-AHx1-ALx8-ATx1-AMx1-AAx1-APx1-AQx4-ABx2-AAx3-ABx1-AUx1-ASx2-AUx1-ASx1-ADx1-ASx3-ACx1-ADx1-AOx1-ADx1-ACx1-AAx1-AEx21-ALx1-AGx1-ALx9-AMx1-APx2-AQx4-ARx4-AQx1-ARx1-ASx9-AOx1-AVx1-AOx2-AWx3-ADx1-ACx1-ABx1-AAx1-AEx11-AAx5-ALx7-AGx1-ALx4-AMx1-AAx1-AQx2-AXx1-ABx1-ARx2-AXx3-ARx1-AUx1-ASx5-AVx4-AOx1-AWx10-AOx2-ADx2-ACx1-ABx1-AAx9-ANx3-ALx8-ANx2-ALx1-AGx1-AMx2-AAx1-ARx2-ABx1-ARx2-ABx3-ADx1-AOx1-ADx2-AVx2-AOx3-AWx11-AYx2-AVx2-AWx5-AOx2-AWx3-ALx4-ANx1-ALx4-AGx1-ALx2-AGx2-AFx1-AEx1-AFx1-AMx1-AEx1-AFx2-AGx4-AEx1-AAx1-ABx1-ACx3-AOx2-AWx12-AYx3-AVx1-AWx10-AOx1-ALx1-AGx1-ALx1-AFx5-AGx1-AFx1-AGx3-AFx2-AGx1-ALx1-AGx1-ATx1-AGx3-ALx1-AHx1-ALx1-AEx2-AAx3-ABx1-ACx1-AOx3-AWx10-AYx1-AVx2-AWx1-AYx2-AWx6-AOx3-ALx4-AGx2-ALx1-AGx2-ALx2-AGx2-ALx12-AFx1-AAx6-ABx1-AWx2-AOx2-AYx2-AOx1-AYx1-AWx1-AYx1-AWx1-AVx3-AOx1-AVx2-AYx1-AWx4-AOx1-AYx1-AWx3-AGx1-ALx4-AGx1-ALx1-AFx1-AGx1-AMx1-ALx9-AHx1-ANx3-ALx1-AGx1-AFx1-AAx1-ABx1-ACx2-ABx1-AAx4-AKx1-ACx1-AWx1-AOx2-AVx8-AOx2-AWx3-AZx7-ALx1-AGx3-ALx3-AGx1-ALx2-ANx1-AHx1-ANx2-ALx2-AGx1-AFx2-AEx1-AAx2-ABx1-ACx4-ADx5-ABx1-AAx8-AKx2-ACx1-AKx2-AEx3-BAx2-AKx5-BAx1-AEx3-AFx2-AEx4-AFx1-AGx1-AHx2-AGx1-AFx1-AEx1-AAx2-ABx1-ACx14-ADx5-ACx3-ABx16-AAx1-ABx12-ACx5-ABx1-ACx1-ABx3-ACx26-ADx4-ACx1-ADx1-ACx2-ADx2-ACx7-ABx14-ACx3-ABx2-ACx4-ABx1-ACx6-ABx2-ACx28-ABx14-ACx1-ABx2-ACx1-ABx6-ACx1-ABx1-ACx34-ABx28-ACx3-ABx5-ACx9-ABx1-ACx10-ABx1-ACx2-ABx1-AAx1-ABx10-AAx2-AEx2-AFx5-AEx3-AFx1-AGx2-AHx2-ALx1-AGx2-AFx5-AAx1-ABx8-AAx2-AEx1-AFx1-AAx1-ABx2-AAx3-ABx5-AHx1-AGx1-AMx1-AGx4-AHx1-ANx29-AFx1-AEx2-AFx1-AGx2-ALx1-ANx1-AHx1-ALx1-ANx2-AHx1-ANx7-AHx1-";
        break;
      case 15:
        theFullImage = "AA#A27654,AB#765738,AC#493219,AD#57472D,AE#86674B,AF#B59278,AG#EDCBB3,AH#DFBCA2,AI#F4E0C8,AJ#D5A987,AK#BB825D,AL#F0B290,AM#D68D6F,AN#9E5A43,AO#7F4023,AP#42190A,AQ#17100F,AR#A34929,AS#22212B,AT#62250F,AU#2B323F,AV#5C5550;AAx3-ABx2-AAx2-ABx1-ACx1-ADx1-ACx1-ADx1-ABx8-AAx1-AEx1-ABx3-AAx1-ABx1-ACx1-ADx2-ACx1-ABx1-AAx1-AFx1-AGx12-AHx1-AGx1-AIx1-AGx3-AIx1-AAx1-AHx1-AIx1-AGx1-AIx2-ABx2-AEx1-AAx2-AFx2-AAx1-AFx1-AAx2-AFx1-AAx2-AFx4-AAx3-ABx1-ACx1-AEx1-AAx1-AFx1-AHx1-AJx1-AFx1-AAx1-ABx2-ACx1-AAx1-AHx1-AJx1-AHx1-AGx4-AHx7-AFx2-AAx3-AJx1-AFx1-AAx1-AJx1-AGx3-AIx1-AJx1-AFx1-AAx2-AFx1-AAx1-ABx1-AAx1-AFx3-AJx1-AHx1-AJx1-AHx1-AJx4-AAx1-ABx2-AAx1-AFx3-AAx1-AEx1-ABx2-AAx3-AFx6-AAx13-ABx1-AAx4-AJx2-AHx1-AJx1-AFx2-AAx1-ABx2-AAx1-AFx1-AJx3-AHx1-AFx1-AAx1-AFx1-AJx1-AFx2-AAx3-ABx9-AAx1-AFx3-AAx3-AFx5-AAx10-ACx1-ABx1-AAx6-AJx2-AHx1-AJx1-AFx1-AAx2-AFx7-AAx3-AEx1-AAx1-ABx10-AAx1-AFx4-AAx8-ABx5-AEx2-ABx2-AEx1-AAx2-AJx1-AFx1-AAx4-AFx2-AAx1-AFx1-AAx2-ABx2-AAx2-ABx4-ADx1-ACx1-ADx1-ABx4-ADx2-ABx5-AAx1-AFx4-AAx16-AGx1-AAx5-AJx1-AFx1-AAx3-AJx2-AFx4-AJx1-AEx1-ABx2-AAx2-ABx2-AAx1-AFx2-AAx2-ABx1-ADx1-ACx1-ADx3-ABx2-AEx1-AFx6-AAx15-AHx1-AJx1-AAx5-AFx2-AAx2-AFx6-AJx2-AAx2-AFx5-AJx2-AFx1-AAx1-ABx1-ADx5-ABx2-AAx1-AFx4-AAx1-AFx2-AAx23-ABx2-AHx6-AJx2-AHx4-AJx1-AFx2-AAx2-ABx1-ACx1-ADx4-ABx2-AEx1-AAx6-AKx1-AFx2-AAx18-AEx1-AAx3-ABx1-AAx2-AHx2-AJx8-AFx2-AAx2-AEx2-AAx2-AFx6-AJx1-AFx1-AAx7-AFx1-AAx19-AEx1-AAx6-AJx8-AHx1-AJx1-AHx1-AJx2-AHx2-AJx8-AFx1-AAx29-AEx1-AAx6-AHx1-AJx14-AHx1-ALx1-AJx1-AMx1-AJx1-AFx1-AAx1-ABx2-AAx10-ABx1-AAx25-AJx15-AAx2-AJx1-AAx1-ABx4-ADx2-ABx1-AAx8-ABx1-AAx25-AJx10-AAx3-AFx1-AJx1-AFx1-AAx2-ABx8-ADx1-ABx1-AAx5-ABx1-AAx26-AJx9-AAx1-AEx1-ABx3-AAx1-ABx1-ADx2-ABx9-ACx2-ABx1-AAx2-ABx2-AAx26-AJx8-AKx1-AAx2-ANx1-ABx3-AOx2-ABx1-AOx1-ANx3-ABx1-ACx3-APx3-AQx1-APx1-ADx1-ABx2-AAx26-AJx8-AAx2-ANx6-AOx5-ARx1-AOx1-ACx1-APx1-ACx2-APx3-AQx1-APx1-ACx2-ABx1-AAx25-ALx1-AJx7-AKx1-ANx7-AOx2-ACx1-AOx4-ACx1-APx2-ACx1-APx1-ACx1-APx1-AQx5-ACx1-ADx1-ABx1-AEx1-AAx18-AEx1-ABx2-AFx8-AKx1-AAx1-ANx3-ARx2-AOx4-ANx1-ABx1-ANx1-AOx1-APx4-AQx1-APx1-AQx8-ASx5-ACx1-ADx2-ABx11-AEx1-ABx3-AJx2-AFx1-AJx1-AFx1-AJx4-AKx1-ANx3-AOx4-ACx1-ATx1-AOx3-ATx1-APx4-AQx11-ASx5-AUx1-AQx5-ASx9-AQx2-AJx11-AAx1-AOx5-ACx2-ATx2-AOx2-ACx8-AQx11-ASx2-AQx4-ASx1-AQx1-ASx3-AUx4-ASx1-AUx2-AJx6-AKx1-AJx1-AFx1-AJx2-AHx1-AJx1-AAx2-ABx1-AAx1-ABx7-ADx1-ACx2-APx2-ACx2-ADx7-ACx7-ADx3-ABx1-ADx6-AVx2-ABx3-AJx3-AFx3-AJx1-AHx4-AJx1-AHx1-AFx2-AAx1-ABx2-ACx7-APx2-AQx3-APx1-ACx1-ADx1-ABx29-ADx1-ABx3-ADx6-ACx48-ABx9-ADx5-ACx2-ADx1-ACx8-ADx2-ACx1-ADx5-ACx26-ADx1-ABx13-ADx26-ACx3-ADx4-ACx2-ADx4-ACx1-ADx1-ACx3-ADx2-ACx1-ABx21-ADx36-ACx1-ADx2-ABx28-ADx12-ABx2-ADx9-ABx4-ADx5-ABx19-AEx2-AAx2-AEx1-AAx5-ABx1-AAx4-AEx1-ABx9-ADx1-ABx10-AAx3-ABx2-AEx1-ABx2-AEx1-AAx3-AFx5-AJx1-AHx1-AJx1-AFx1-AJx1-AHx18-AJx1-AFx2-AAx1-AFx4-AJx7-AHx1-ALx1-AHx5-";
        break;
      case 16:
        theFullImage = "AA#8E6847,AB#B08863,AC#C39973,AD#564024,AE#E5B18F,AF#6D573F,AG#3F311A,AH#231C0D,AI#0D0605,AJ#E6D1B8,AK#F6E1C9,AL#FCF5DC,AM#7D705F,AN#A57752,AO#C36C4D,AP#824830,AQ#9D5535,AR#572510,AS#70331A;AAx1-ABx1-AAx1-ABx9-ACx4-ABx3-AAx1-ADx1-AAx1-ABx1-ACx1-AEx3-ABx1-AAx1-AFx1-ADx2-AGx2-AHx3-AIx1-AHx2-ABx1-ACx1-ABx2-ACx1-AJx1-ACx1-ABx6-ACx1-AJx1-AKx1-ALx1-AKx3-ACx1-ABx3-AAx3-ABx3-AEx5-ACx2-AEx1-ABx1-AAx1-AFx1-AAx1-ABx3-AAx2-AFx2-AAx4-AFx1-ADx1-AGx1-AFx1-AMx1-ABx2-AJx1-ABx13-ACx1-AEx1-ACx1-AJx3-ACx2-ANx1-AAx2-ABx1-ACx1-AEx4-ABx1-AAx1-ABx1-AEx1-ACx1-ABx1-AAx1-ABx1-AAx1-AFx9-ADx1-AFx2-AAx2-ABx1-AJx2-AEx1-AMx1-AEx2-ABx14-ACx1-ABx2-ACx1-AEx4-ACx1-AAx2-ABx2-ACx3-ABx2-AAx5-AFx1-ADx7-AFx2-AAx3-AFx1-AAx1-ABx21-ACx1-ABx1-AAx4-ABx5-AAx4-AFx1-ADx2-AFx1-ADx13-AFx1-ABx2-AAx4-ABx18-AMx1-AAx1-AFx1-AGx1-AFx1-AAx4-AEx1-ACx2-ABx2-AEx1-AAx1-AFx1-AAx2-AMx1-AAx3-ABx1-ACx1-ABx2-AFx1-ADx7-AFx2-ABx2-AMx1-ABx17-AMx1-AAx4-ADx2-AAx5-ABx1-ACx4-AEx2-ABx3-ACx1-AEx2-ACx1-AEx2-ACx1-AAx1-ADx6-AFx3-AAx1-ACx3-ABx17-AAx4-AFx2-AAx2-AMx1-AAx2-AMx1-AEx11-ACx1-ABx2-ANx1-AAx1-ADx6-AFx1-AAx2-ABx1-ACx3-ABx11-ANx2-ABx2-AMx1-ABx1-ANx1-AAx7-ABx4-AAx2-AEx9-ACx1-ABx4-AAx1-ABx2-ACx5-AEx6-ABx13-AAx9-AFx1-AAx2-AMx1-ABx5-AAx1-AEx27-ABx12-ANx1-AAx7-ABx4-AAx2-ABx2-AAx1-ANx1-ABx3-AEx1-ACx1-AEx19-ACx1-AEx1-ACx2-AEx1-ABx16-AAx3-ABx8-AAx1-ABx1-AAx2-ABx2-AAx1-AEx17-ACx1-AEx6-ACx1-ABx16-AAx3-ABx9-AAx1-ABx1-AAx2-ABx1-AAx2-AEx20-ACx3-ABx5-AAx1-ABx9-ANx1-ABx1-ANx1-AAx1-ANx1-ABx9-ANx2-ABx1-AAx4-ABx1-AEx10-ACx2-AEx2-ACx1-ABx1-ACx2-ANx1-AOx1-ABx1-AAx1-AFx1-AAx1-AMx1-ABx4-AAx4-ABx4-ANx1-ABx2-AAx2-ABx1-ANx2-ABx9-AAx4-ANx1-ABx1-AEx1-ACx5-AEx1-ACx1-ABx6-AEx1-ABx2-ACx2-AAx2-AFx4-AAx1-ABx5-ANx1-AAx6-ANx2-AAx4-ABx6-AAx1-ABx2-AAx5-ABx2-AEx9-ABx1-AAx4-ABx2-ACx1-ABx1-AAx2-AFx7-AAx1-ABx3-ANx1-AAx2-AFx1-AAx7-ANx1-ABx4-AAx10-ANx2-ABx1-AEx9-AAx7-APx1-ADx1-APx1-AAx3-AFx2-APx1-ADx4-AAx1-ANx1-AAx4-ADx1-AAx2-ABx4-ANx1-AAx17-ACx1-ABx8-AAx4-AQx1-APx1-ADx3-APx2-AQx1-APx1-ADx1-AGx2-ARx1-AGx1-AHx3-ADx1-AAx3-ADx1-AFx1-AAx24-AEx1-ACx1-AEx1-ACx1-AEx2-ACx2-ABx1-AAx1-AOx1-AQx4-APx1-ADx1-ASx1-ARx1-APx4-AGx1-AHx1-ADx1-AGx1-AHx3-AIx1-AHx1-ADx1-AFx3-AAx24-AEx5-ACx4-AAx1-AOx1-AQx2-APx3-ARx3-APx4-ADx1-AGx2-AHx1-AGx1-AHx2-AIx2-AHx1-AGx1-ADx1-AFx1-AAx2-ABx2-AAx20-ACx1-AEx1-ACx2-AEx1-ACx2-AEx2-ANx1-AQx1-APx4-ASx1-ADx2-APx1-AAx2-ABx2-AAx1-ADx1-AGx1-AHx3-AIx3-AHx1-ADx3-AAx2-ABx1-ANx1-AAx16-AFx4-AEx10-APx6-ARx3-AAx1-ABx4-AAx1-ADx1-AHx2-AIx3-AHx1-ADx3-AAx1-ABx3-AAx2-ADx2-AFx2-ADx15-ACx1-ABx1-AAx1-AFx1-ADx1-AFx1-ADx10-AGx2-ADx1-AAx1-ABx5-AFx1-AGx1-AHx3-AGx1-ADx2-AFx2-ANx1-ABx1-ACx1-ABx1-ACx1-ABx1-AFx1-ADx3-AGx1-AHx4-AGx1-AHx6-AGx3-AAx2-AFx2-ADx1-AFx2-ADx7-AGx1-ADx2-AGx2-AAx1-ABx1-ACx2-ABx2-AAx1-AFx1-ADx3-AAx1-ABx5-ACx4-ABx1-AAx1-ADx18-AAx1-ABx2-AAx2-AFx4-ADx10-AAx1-ABx1-ACx3-ABx2-AAx2-ANx1-ABx1-ACx1-AEx3-ACx1-AEx3-ACx2-ABx1-AAx1-AFx1-ADx17-AEx1-ACx1-ABx2-AFx8-ADx5-AFx2-AAx1-ABx1-ACx3-ABx6-ACx1-AEx9-ABx1-AAx1-AFx1-ADx17-AEx2-ABx1-AFx11-ADx2-AFx4-ABx1-ACx4-ABx4-ACx1-AEx8-ACx1-ABx2-AAx1-AFx3-ADx15-AAx2-AFx18-ANx1-ABx1-ACx3-ABx4-ACx1-AEx8-ACx1-ABx2-AAx3-AFx4-ADx1-AFx1-ADx2-AFx2-AAx1-ABx2-AAx3-AFx2-AAx6-AFx12-AAx2-ABx5-ACx4-AEx10-ABx1-AAx1-ANx1-ABx2-ACx3-AEx11-ACx1-ABx3-ACx1-AEx3-ACx2-AEx1-ACx2-AAx1-ABx1-ACx3-ABx2-ACx1-AEx6-ACx3-AEx2-ACx1-";
        break;
      case 17:
        theFullImage = "AA#B38762,AB#9F7A55,AC#755C37,AD#DEA780,AE#82684F,AF#DABCA2,AG#ECD0B6,AH#C7AC92,AI#C69875,AJ#5F482E,AK#4D2F1A,AL#EFB58E,AM#C16F50,AN#A55537,AO#793B22,AP#361C08;AAx1-ABx2-ACx2-AAx4-ADx8-AAx1-ABx1-ACx3-ABx2-ACx6-AEx1-ABx1-AEx1-AAx1-AFx1-AGx1-AHx1-AAx1-AGx1-AFx1-AHx1-AAx6-ADx1-AGx4-AFx1-AGx1-ADx1-AAx1-AFx1-AGx3-ADx1-AAx1-ACx1-AEx1-AAx1-ADx5-AAx2-AIx1-ADx1-AAx1-ABx1-ACx16-AAx17-ADx4-AFx1-AAx1-ABx2-AAx1-ADx1-AFx1-ADx3-ABx1-ACx1-ABx1-AAx1-AIx1-AAx3-ABx1-ACx3-ABx2-ACx2-AJx2-ACx5-ABx4-ACx1-ABx1-AAx17-ABx3-AKx1-ACx2-AEx2-ABx1-AAx1-ADx1-ABx1-AAx3-AEx1-ACx3-AJx2-ACx8-AJx4-ACx2-AJx2-ABx1-AAx1-ACx2-ABx1-AAx16-ABx3-ACx2-AKx1-ACx1-AEx1-ABx4-AAx1-ADx1-AAx3-ADx1-ABx1-AEx1-ABx1-AAx2-ABx1-ACx1-ABx1-ADx3-AAx1-ACx1-AJx7-ACx2-AAx1-ADx1-AAx11-ABx10-ACx3-AAx3-ABx3-AAx1-ADx6-AIx1-ADx8-AAx1-ACx1-AJx6-ACx3-ABx1-AIx1-AAx11-ABx7-ACx5-ABx2-AAx4-ABx3-ALx7-AFx1-ADx2-AAx2-ABx1-ACx2-AJx2-ACx5-ABx1-AAx1-AIx1-ADx2-AAx12-ABx8-AAx1-AEx1-ACx1-ABx2-AAx2-ABx1-AAx3-ABx2-ADx11-AAx4-ADx12-AAx12-ABx7-AAx5-ABx1-AAx2-ABx1-AAx5-ADx23-ALx1-ADx2-AAx15-ABx3-AAx8-ABx1-AAx1-ABx2-AAx2-ABx1-ACx1-ADx25-AAx15-ABx3-AAx9-ABx1-AAx1-ABx2-AAx1-ABx1-ACx1-ABx1-ADx24-AAx13-ABx1-AAx2-ABx2-AAx12-ABx2-AAx1-AEx1-ABx1-AAx1-ADx21-AAx7-ABx4-AAx7-ABx2-AAx13-ABx5-AAx1-ADx7-AIx1-ADx6-AIx1-AAx2-ADx1-AMx1-ANx1-ACx2-AJx1-ACx1-ABx1-AAx6-ABx1-AEx1-ACx1-ABx2-AAx1-ABx5-AAx11-ABx5-AAx2-ADx7-AIx3-AAx4-ADx1-AIx1-ADx1-AAx1-ACx8-ABx1-AAx6-ABx1-ACx2-ABx5-AAx5-ABx3-AAx1-ABx8-AAx2-ADx9-AAx1-ACx3-AEx1-AAx2-ACx12-ABx2-AAx4-ACx2-AEx1-ABx2-AAx3-ABx18-ADx8-AAx1-ABx1-ANx1-ACx4-AJx1-ACx1-AOx2-ANx4-ACx2-AJx1-AKx3-AJx1-ABx2-AAx1-ABx1-ACx2-AEx1-ABx23-AAx5-AIx1-AAx1-AIx1-AAx1-AMx1-ANx5-AOx1-AJx1-AOx2-ANx3-AOx1-AJx2-ACx1-AJx2-AKx3-ACx1-AEx1-ACx4-AEx2-ABx4-AEx5-ACx2-ABx2-AEx1-ABx7-AIx1-ADx7-AAx1-AMx2-ANx4-AOx1-AJx1-AKx1-AOx1-ANx3-AJx3-ACx7-AJx2-ACx2-AEx1-ABx12-AAx3-ABx6-ACx2-ADx8-AAx1-AMx2-ANx3-AOx1-AKx3-AOx1-ANx1-AOx2-AKx2-AJx1-ACx1-AJx1-ACx5-AKx3-AJx1-ACx1-ABx2-AAx4-ABx4-AAx4-ABx2-AEx1-ACx6-ADx9-ANx4-AOx3-AKx2-AOx1-AJx1-ANx1-AOx1-AKx2-AJx3-ACx5-AJx3-AKx1-AJx2-ACx4-AEx3-ABx7-AEx1-ACx7-ALx2-ADx6-AIx1-AAx1-AJx1-AOx5-AKx7-APx1-AKx1-AJx3-ACx24-AJx3-AKx2-AJx3-AAx1-ABx1-ACx4-AJx5-AKx5-APx3-AKx6-AJx1-ACx26-AJx1-AKx6-AJx1-ABx1-ACx5-AJx7-AKx6-APx1-AKx2-AJx1-ACx3-ABx10-AEx2-ACx7-ABx5-ACx2-AJx8-AAx3-ACx8-AJx9-ACx3-AEx1-ABx2-AAx3-ABx5-AAx1-ABx2-AAx1-ABx5-AAx6-ABx2-ACx2-AJx7-ADx1-AAx1-ACx11-AJx5-ACx4-AEx1-ABx1-AAx8-ABx2-AAx15-ABx2-ACx3-AJx6-AAx1-ACx20-AEx1-ABx1-AAx27-ABx2-ACx4-AJx4-ACx21-ABx1-AAx3-ADx10-AIx1-ADx5-AIx1-ADx2-AIx4-ADx5-AFx1-ADx6-ACx1-ABx1-ACx2-ABx1-AAx16-ADx3-ALx9-AFx1-ALx9-ADx1-AFx1-ALx10-ADx1-ALx1-ADx12-ALx1-AFx1-ALx1-ADx2-AFx2-ALx2-ADx3-ALx1-ADx1-ALx8-ADx1-ALx2-ADx3-ALx5-ADx1-ALx4-ADx1-ALx2-ADx4-ALx5-ADx1-ALx1-ADx1-ALx1-AFx1-ALx1-ADx3-AAx1-ADx1-AFx1-ALx3-ADx4-ALx1-ADx5-ALx1-ADx1-ALx3-ADx15-ALx1-ADx9-ALx2-ADx1-";
        break;
      case 18:
        theFullImage = "AA#B38768,AB#815E40,AC#9D7451,AD#6B5136,AE#4E381C,AF#C49679,AG#DFAA86,AH#F3B990,AI#DF906D,AJ#804228,AK#A15131,AL#3E1B09,AM#B16543,AN#6F2F19,AO#1E0906;AAx2-ABx2-ACx1-AAx1-ACx1-ABx3-ADx3-ABx3-ADx8-ABx1-AAx2-ACx1-ABx1-ACx1-AAx17-ACx3-ABx2-AEx1-ADx1-ABx1-ACx4-AAx1-AFx1-AAx1-AFx1-ACx1-ADx2-ABx5-ACx1-AAx1-ACx1-ABx2-ADx1-AEx2-ADx6-ACx1-AAx1-ABx1-AAx14-ACx6-ABx6-ACx5-AAx3-AFx1-AGx1-AAx1-ACx1-AAx6-AFx3-ABx1-ADx2-AEx2-ADx5-ABx1-AAx11-ACx10-ABx3-ACx3-AAx3-ACx4-AGx11-AFx1-AAx1-ACx1-ADx1-AEx4-ADx3-ABx1-AAx16-ACx3-ABx5-ACx1-ABx1-ACx2-AAx6-ACx3-AGx7-AAx2-ACx1-ABx1-ADx2-ABx2-ACx2-AAx4-AFx2-AGx3-AAx11-ACx8-AAx3-ACx1-AAx1-ACx1-AAx3-ACx1-AAx4-ACx1-AGx23-AHx1-AGx1-AAx16-ACx3-AAx10-ACx1-AAx3-ACx2-AGx21-AFx1-AGx2-AAx16-ACx2-AAx12-ACx1-AAx2-ACx1-ABx2-AGx7-AFx1-AGx3-AFx1-AGx7-AFx1-AGx1-AFx2-AAx17-ACx1-AAx13-ACx1-AAx2-ACx2-AAx1-AGx19-AFx2-AAx5-ACx4-AAx9-ACx1-AAx14-ACx1-AAx1-ACx1-ABx1-AAx2-AGx4-AFx1-AGx7-AFx3-AGx1-AIx1-ACx3-ABx2-ACx2-AAx5-ACx1-ABx2-ACx2-AAx20-ACx1-AAx1-ACx2-AAx2-AFx5-AAx6-AFx3-ACx1-AAx1-AFx1-ABx4-ADx2-ABx1-ACx1-AAx5-ACx3-ABx2-ACx1-AAx17-ACx4-AAx3-AGx2-AFx1-AGx3-AFx1-AGx1-AAx1-ABx1-ACx2-AAx1-AFx2-ACx1-ABx7-ADx2-ABx2-AAx2-ACx4-ABx3-ACx1-AAx16-ACx3-AAx4-AGx7-AFx1-ABx4-ACx1-ABx2-ADx3-ABx6-ADx1-AEx3-ABx1-ACx4-ABx3-ACx9-AAx2-ACx3-AAx10-AGx1-AFx3-AAx2-AFx1-AAx1-ACx1-ABx4-AJx1-ADx1-ABx1-AJx2-AKx2-AJx2-AEx4-ALx3-AEx1-ABx3-ADx1-ABx4-ACx1-ABx1-ACx14-AAx10-AFx3-ACx1-AMx1-ABx2-AKx2-AJx2-ABx1-ANx1-AJx4-AEx1-ALx1-AOx1-AEx2-ALx5-AEx1-ADx1-ABx10-ACx4-ABx3-ACx4-AAx6-AGx1-AFx2-AGx4-ACx1-AMx1-AKx3-AJx2-AEx2-ANx1-AJx4-ALx2-AOx1-ALx6-AOx3-AEx1-ABx3-ACx15-AAx8-AGx3-AAx1-AFx1-AAx1-AFx1-AAx1-AKx3-AJx4-ADx1-AJx4-ANx1-AEx1-ALx7-AOx4-ALx2-AEx1-ADx1-ABx2-ACx21-AFx2-AGx1-AFx1-AGx4-ABx1-AJx6-AEx4-AJx1-ANx1-ALx4-AOx4-ALx4-AEx2-ADx3-ABx15-ACx1-ABx1-ACx5-AGx1-AAx5-ACx2-ABx1-ADx1-AJx2-AEx8-AJx1-AEx1-ALx3-AOx2-ALx5-AEx6-ADx7-AEx1-ADx10-ABx4-ADx4-AEx10-ALx5-AEx2-ALx5-AEx16-ADx4-ABx3-ADx4-ABx7-ADx9-AEx1-ADx1-AEx14-ADx4-ABx1-ADx12-ABx14-ACx4-AAx1-ABx1-ADx9-AEx1-ADx2-AEx3-ADx2-AEx1-ADx11-ABx3-ADx2-ABx18-ACx6-ABx1-ADx13-AEx1-ADx13-ABx24-ACx8-ABx2-ADx25-ABx18-ACx2-ABx5-ACx2-AAx1-ACx5-ABx5-ADx1-ABx2-ADx16-ABx2-ACx1-AAx2-AFx3-AAx3-AFx2-AAx3-ACx3-AAx2-ACx1-AAx3-AFx2-AGx2-AHx2-AGx5-ABx3-ACx1-AAx4-ACx2-ABx2-ACx5-AAx6-AGx1-AHx8-AGx3-AHx1-AGx1-AHx1-AGx1-AHx3-AGx2-AHx1-AGx6-AHx4-AGx2-AHx1-AGx2-AFx2-AGx6-AHx3-AGx3-AHx3-AGx3-AHx6-AGx3-AHx2-AGx8-AHx1-AGx4-AHx1-AGx10-AHx1-AGx6-AHx1-AGx3-AFx1-AAx1-AGx1-AHx1-AGx2-AHx1-AGx9-AFx1-AGx30-AFx1-AGx1-AHx2-AGx4-AFx1-AGx2-AFx1-AGx2-AHx1-AGx7-AAx1-AFx1-AGx1-AFx1-AGx13-AAx2-AGx1-AFx1-AGx20-AFx3-AGx1-AFx1-AGx4-AHx4-AGx4-AHx1-AGx20-AAx1-AFx1-AGx11-AFx1-AGx2-AFx1-AAx1-AFx1-";
        break;
      case 19:
        theFullImage = "AA#DFAA86,AB#C59572,AC#8F6A49,AD#AC8061,AE#705336,AF#5A4127,AG#C3714F,AH#864028,AI#9F5130,AJ#552817,AK#431A0A,AL#280B07,AM#B26240,AN#F4BA91;AAx1-ABx1-ACx3-ADx2-ACx2-ADx1-AAx2-ABx1-ADx1-AEx10-ACx1-ABx2-ADx14-ACx10-AEx1-ACx3-ADx4-ABx1-AAx11-ABx1-ACx1-AFx1-AEx7-ACx1-ADx1-ACx1-ABx1-AAx1-ABx1-ADx13-ACx8-ADx3-ACx4-ADx2-ACx1-ADx2-AAx6-ABx2-ACx2-AEx4-ACx5-ABx3-AAx3-ABx1-ADx17-ACx3-ADx7-ACx1-ADx3-ACx1-ADx2-AAx10-ABx1-AAx13-ABx1-ADx17-ACx3-ADx10-ACx1-ADx1-ACx1-ADx2-AAx23-ABx1-ADx18-ACx2-ADx13-ACx2-ADx1-AAx21-ABx2-ADx3-ACx1-ADx14-ACx2-ADx14-ACx2-ADx1-AAx20-ADx7-ACx5-ADx9-ACx3-ADx13-ACx2-ADx1-AAx12-ABx1-AAx1-ABx1-AGx2-ACx2-AEx2-ACx3-ADx7-ACx5-ADx4-ACx4-ADx8-ACx2-ADx2-ACx4-AAx3-ABx2-AAx1-ABx2-AAx1-ABx1-AAx1-ABx1-ACx1-ADx1-AAx1-ADx1-ACx1-AEx7-ACx1-ADx7-ACx3-AEx1-ACx8-ADx6-ACx3-ADx3-ACx4-ABx1-AAx2-ABx3-ACx4-AAx1-ABx1-AAx1-ABx1-ACx1-AEx11-ACx2-ADx1-ACx6-AEx2-ACx3-ADx1-ACx3-ADx2-ACx14-AAx5-ACx3-AEx1-ACx1-ADx1-ACx2-AEx12-AFx3-ACx6-AEx2-ACx24-AAx4-ABx1-ACx4-AEx1-ACx1-AEx3-AFx1-AHx2-AIx1-AHx1-AEx2-AJx4-AKx4-AJx1-ACx3-AEx3-ACx24-ADx1-ABx1-ADx1-ABx1-ADx1-ACx1-AIx5-AFx2-AEx1-AJx1-AHx5-AJx1-AKx2-ALx1-AJx2-AKx3-ALx2-AFx1-AEx2-AFx1-AEx1-ACx24-AAx2-ABx2-ADx1-AMx1-AIx5-AHx1-AFx2-AJx2-AHx4-AJx1-AKx2-ALx1-AJx1-AKx4-ALx4-AJx1-AEx4-ACx22-AAx4-ABx1-AMx2-AIx2-AHx2-AFx2-AJx2-AHx3-AFx1-AJx2-AKx1-ALx2-AJx1-ALx1-AKx2-ALx7-AFx1-AEx1-ACx23-ADx1-ABx4-AIx3-AHx5-AFx2-AHx4-AJx2-AKx2-ALx1-AKx1-ALx12-AKx1-AFx1-AEx2-ACx13-AEx6-AAx5-ADx1-AHx7-AJx7-AKx1-ALx11-AKx3-AJx2-AFx3-AEx1-AFx4-AEx15-ADx3-ACx3-AFx4-AJx2-AFx1-AJx8-AKx3-ALx10-AKx4-AJx9-AKx6-AJx7-AFx8-AJx4-AKx2-ALx2-AKx5-ALx7-AKx7-AJx24-AFx12-AJx1-AFx1-AJx2-AKx5-AJx2-AFx10-AJx3-AFx25-AEx4-AFx2-AEx2-AFx52-AEx8-AFx8-AEx1-AFx29-AEx1-AFx9-AEx1-AFx2-AEx12-AFx2-AEx5-AFx3-AEx4-AFx5-AEx3-AFx7-AEx4-AFx8-AEx1-AFx1-AEx31-AFx1-AEx28-ACx3-ABx1-ADx2-AEx20-AFx1-AEx4-ACx1-ADx1-ABx2-AAx7-ABx2-AAx2-ABx1-ADx1-ACx3-ABx1-ADx2-ACx3-ADx1-AAx3-ANx1-AAx4-AEx1-ACx7-AEx2-ACx4-AEx2-ACx5-ADx1-ABx1-AAx1-ANx6-AAx1-ANx2-AAx10-ANx2-AAx11-ANx4-AAx6-ANx4-AAx1-ANx5-AAx2-ANx2-AAx1-ANx1-AAx2-ANx2-AAx4-ANx2-AAx8-ANx1-AAx1-ANx1-AAx1-ANx1-AAx3-ANx1-AAx13-ABx1-AAx1-ABx1-AAx4-ABx1-AAx20-ANx2-AAx32-ABx1-AAx1-ANx1-AAx51-ABx1-AAx1-ABx1-AAx12-ABx3-AAx2-ABx1-AAx15-ABx2-";
        break;
      case 20:
        theFullImage = "AA#C58E66,AB#E9B08A,AC#946D4C,AD#AA805E,AE#D59E77,AF#7B5A3B,AG#604C30,AH#A75030,AI#4F301D,AJ#7D3A1E,AK#3D1206,AL#0C0304,AM#5F200D,AN#291C16,AO#F9C498;AAx1-ABx1-AAx1-ACx3-AAx1-ADx1-ACx2-AAx2-ABx1-AEx1-AAx1-AFx1-AGx6-AFx4-AAx3-ADx2-AAx5-ADx7-ACx13-ADx4-ABx12-AAx1-ACx1-AGx6-AFx1-AGx1-AFx1-ACx1-ADx2-AAx2-ADx3-AAx4-ADx3-AAx1-ADx3-ACx13-ADx2-ACx1-ADx2-ABx6-AEx1-AAx2-ACx1-AFx2-AGx1-AFx2-ACx5-AAx3-ABx3-AAx2-ADx15-ACx11-ADx2-ACx2-ADx2-ABx8-AEx3-AAx1-AEx1-ABx11-AEx1-ADx1-ACx4-ADx1-ACx2-ADx10-ACx14-ADx3-ABx22-AAx2-ADx3-ACx9-ADx4-ACx14-ADx6-ABx19-AAx2-ACx33-ADx3-ACx3-ABx12-AAx1-ABx1-AAx2-AHx2-AFx2-AGx1-AIx1-AGx1-AFx1-ACx14-AFx2-ACx20-ABx9-AAx1-ABx1-ACx3-AEx2-ACx1-AFx5-AGx4-AFx1-ACx13-AFx1-ACx19-ABx2-AEx1-AAx3-ACx1-ADx1-AAx1-ACx1-ABx1-AEx1-ABx1-AAx1-ACx1-AFx9-AGx4-AFx1-ACx10-AFx2-ACx19-AEx1-ABx2-AEx1-ACx2-AFx4-ADx1-ACx2-AFx1-AGx2-AFx10-AGx3-AIx1-AGx1-ACx8-AFx2-ACx19-ABx3-AAx1-ACx2-AFx3-ACx2-AFx1-AGx2-AIx2-AJx2-AHx2-AFx1-AHx1-AFx1-AJx1-AIx3-AKx5-AGx1-ACx5-AGx1-AFx1-ACx20-ABx3-ACx4-AFx2-AHx1-AFx1-AGx3-AFx1-AIx1-AJx1-AHx3-AJx2-AKx4-AIx3-AKx3-ALx1-AKx1-AFx1-ACx2-AGx2-AFx1-ACx20-AAx3-ACx2-AFx3-AHx3-AFx1-AIx1-AJx1-AFx1-AIx2-AJx1-AHx2-AJx2-AIx1-AKx2-ALx1-AIx2-AKx4-ALx3-AIx1-AGx3-AFx2-ACx1-AFx3-ACx9-AFx1-ACx1-AFx3-ACx1-AEx2-AAx1-ACx1-AHx7-AFx1-AIx1-AJx1-AIx2-AMx1-AJx5-AIx1-AKx2-ALx1-AKx6-ALx5-AKx1-AGx2-AFx7-ACx3-AFx7-ACx3-ABx3-ACx1-AHx7-AJx1-AGx1-AIx4-AJx4-AKx4-ALx1-AKx6-ALx7-AIx1-AFx2-ACx6-AFx6-ACx6-AAx3-ADx1-AHx5-AJx3-AIx2-AJx2-AHx4-AJx1-AIx2-AKx2-ALx2-AKx1-ALx13-ANx1-AIx1-AFx2-ACx15-ABx3-AEx1-AFx1-AHx3-AJx5-AIx6-AJx2-AIx1-AKx1-ALx3-AKx1-ALx7-AKx7-AIx5-AGx3-AFx7-ACx2-AFx2-ABx4-AAx1-AJx8-AIx6-AMx1-AJx1-AMx1-AKx1-ALx3-AKx1-ALx7-AKx4-AIx6-AGx6-AIx10-ACx1-AFx2-AGx2-AIx8-AKx2-AIx1-AKx1-AIx2-AMx1-AIx3-AKx3-ALx12-AKx6-AIx4-AKx12-AIx10-AKx13-ALx7-AKx22-AIx1-AKx1-AIx1-AKx3-AIx2-AGx1-AIx1-AGx1-AIx4-AGx1-AIx5-AKx5-ALx2-AKx4-AIx6-AGx2-AIx4-AKx1-AIx23-AGx7-AIx33-AGx1-AIx9-AGx3-AIx1-AGx13-AIx2-AGx2-AIx5-AGx2-AIx2-AGx1-AIx1-AGx1-AIx7-AGx1-AIx2-AGx1-AIx4-AGx2-AIx1-AGx4-AIx1-AGx23-AIx4-AGx7-AIx1-AGx16-AIx1-AGx1-AIx2-AGx48-AIx1-AGx28-AIx1-AGx29-AIx1-AGx52-AIx2-AGx7-AFx1-ACx1-ADx1-AAx1-ABx1-AEx1-AAx5-ACx3-ADx1-ACx1-AFx3-AGx1-AFx4-AGx5-AFx3-AGx2-AFx2-AGx15-AFx1-AGx3-AFx1-ACx1-AAx1-ABx14-AAx1-AEx1-ABx1-AEx1-ABx3-AAx3-ABx1-AEx1-ABx2-AEx1-ABx1-AAx2-AFx10-ACx3-AFx6-ACx3-ADx1-AAx1-ABx30-AEx1-ABx11-AOx1-ABx40-AOx1-";
        break;
      case 21:
        theFullImage = "AA#D8A57E,AB#C28E66,AC#A67B55,AD#84613D,AE#624F32,AF#F3B88F,AG#B46140,AH#A34727,AI#5C361E,AJ#3C2110,AK#823E20,AL#2D0A05,AM#080204,AN#702912;AAx2-ABx2-AAx2-ACx1-ADx2-ACx1-ABx1-ACx1-ADx2-ACx1-AAx3-ABx1-ADx1-AEx9-ADx1-ACx1-ABx1-ACx28-AAx7-ABx2-AAx8-ABx1-ADx1-AEx7-ADx2-ACx32-AFx9-AAx4-ABx1-ACx2-ADx1-AEx3-ADx5-ACx1-ABx3-ACx31-AFx1-AAx2-AFx1-AAx9-ABx4-AAx9-ACx12-ADx5-ACx17-AFx2-AAx11-AFx3-AAx8-ACx19-ADx3-ACx14-AAx13-ABx2-AAx2-ABx1-AGx1-AHx3-ADx2-AEx3-ADx2-ACx17-ADx2-ACx12-AFx1-AAx11-ABx1-ADx2-AGx1-ABx1-AAx1-ABx1-ADx6-AEx5-ADx1-ACx16-ADx2-ACx11-AFx1-AAx5-ABx3-AAx1-ACx1-AAx5-ABx1-ADx9-AEx5-AIx1-AEx1-ADx1-ACx12-ADx1-AEx1-ADx2-ACx10-AAx5-ACx1-ADx4-ACx1-ABx2-ACx2-ADx15-AEx5-ADx1-ACx10-ADx1-AEx2-ADx1-ACx10-AAx4-ABx1-ADx5-ACx2-ADx2-AEx2-AIx3-AHx1-ADx8-AHx2-AIx5-AJx2-AEx1-ADx1-ACx6-ADx1-AEx1-ADx3-ACx8-ADx2-AAx4-ACx1-ADx7-AEx4-AIx2-AKx1-AHx8-AIx1-AJx5-ALx5-AJx1-AEx1-ADx1-ACx3-ADx1-AEx2-ADx9-ACx1-ADx2-ACx1-AAx2-AFx1-ABx1-ACx1-ADx4-AHx2-ADx1-AEx2-AIx1-AEx1-AIx2-AHx7-AIx1-AJx1-ALx1-AJx2-AIx3-AJx2-ALx5-AJx1-ADx3-AEx1-AIx1-AEx2-ADx11-ACx1-AAx3-ACx2-ADx4-AHx2-ADx1-AIx3-AHx1-AIx3-AKx1-AHx4-AIx2-AJx3-ALx2-AIx2-AJx2-ALx4-AMx2-ALx1-AIx1-AEx5-ADx12-ABx1-AAx2-ACx1-AGx1-ADx3-AHx3-ADx1-AHx1-AIx1-AHx2-AIx1-AJx1-AIx2-AHx5-AIx1-AJx3-ALx2-AIx1-AJx1-ALx1-AJx3-ALx7-AJx1-AEx1-ADx1-AEx1-ADx12-AAx3-ACx1-AGx1-ADx2-AGx1-AHx3-ADx1-AKx1-AIx4-AJx1-AIx1-AHx5-AIx2-AJx3-ALx2-AJx1-ALx2-AJx3-ALx2-AMx1-ALx1-AMx4-ALx1-AIx1-ADx13-AAx3-ACx1-AGx1-ADx2-AGx1-AHx4-AIx1-AEx1-AIx5-AHx3-AKx1-AIx1-AJx3-ALx1-AJx1-ALx2-AJx1-ALx2-AJx2-ALx3-AMx8-ALx1-AIx1-ADx11-ABx1-AAx1-ACx2-ADx2-AGx1-AHx5-AKx1-AIx2-AHx9-AIx2-AJx3-ALx2-AJx1-ALx6-AMx12-ALx1-AJx1-AEx1-ADx6-ACx1-AAx1-AFx2-AAx1-AHx9-AKx1-AIx6-AKx1-AHx2-AIx2-ALx9-AMx3-ALx13-AJx5-AIx1-AEx2-ADx2-AFx4-ABx1-AHx2-AKx2-AHx1-AKx1-AIx1-AKx2-AIx3-AJx2-AIx1-AJx2-AIx2-AJx2-ALx7-AMx4-ALx5-AJx7-AIx3-AEx5-AIx3-ACx2-ADx3-AEx1-AIx12-AJx1-AIx2-AJx1-AIx2-ANx1-AJx2-ALx3-AJx1-ALx2-AMx7-ALx4-AJx10-AIx3-AJx2-AIx14-AJx2-ALx1-AJx2-ALx1-AJx4-AIx3-AJx3-ALx2-AMx10-ALx11-AJx5-ALx2-AJx1-AIx8-AJx5-ALx1-AJx1-ALx25-AJx20-AIx8-AEx1-AIx6-AJx2-ALx8-AJx4-AIx2-AJx2-AIx3-AEx3-AIx1-AEx1-AIx1-AJx18-AEx1-AIx8-AJx2-AIx1-AJx2-AIx8-AJx2-AIx22-AJx1-AIx13-AEx7-AIx53-AEx2-AIx2-AEx2-AIx9-AEx1-AIx2-AEx1-AIx41-AEx8-AIx9-AEx5-AIx3-AEx2-AIx10-AEx1-AIx9-AEx1-AIx3-AEx7-AIx1-AEx11-AIx4-AEx2-AIx1-AEx6-AIx5-AEx1-AIx1-AEx3-AIx9-AEx2-AIx3-AEx1-AIx3-AEx11-AIx1-AEx8-AIx2-AEx9-AIx1-AEx10-AIx4-AEx46-AIx1-AEx9-AIx2-";
        break;
      case 22:
        theFullImage = "AA#D3A07B,AB#A37A54,AC#73522E,AD#806442,AE#BA8F6A,AF#ECB38C,AG#564526,AH#BD6748,AI#A65939,AJ#472C1B,AK#CD805E,AL#6A3619,AM#2E1A0D,AN#934927,AO#0E0804,AP#5E1F0B;AAx9-ABx1-ACx2-ADx1-ABx2-ADx2-ABx2-AEx2-ABx2-ACx11-ABx26-AEx3-AAx2-AEx1-AAx2-AFx1-AAx1-AEx1-ABx1-AEx1-AAx9-ADx1-ACx7-ADx2-ABx10-ADx4-ABx14-AFx16-AAx3-AEx1-ABx1-ACx1-AGx1-ACx6-ADx1-ABx16-ADx2-ACx2-ADx2-ABx8-AFx13-AAx2-AEx1-ABx7-AHx1-AAx1-AEx1-ABx1-AEx1-ABx22-ADx1-ACx3-ADx2-ABx4-AFx13-AAx2-AEx2-AAx2-AEx2-AIx2-AHx1-AIx1-ACx4-AJx2-AGx1-ACx1-ADx1-ABx20-ADx1-ACx2-ADx2-ABx1-AFx1-AAx1-AFx7-AAx1-AFx1-AAx3-AEx1-ADx1-ACx1-ABx1-AKx1-AAx3-ADx1-ACx9-AGx3-ACx1-ABx20-ADx1-ACx2-ABx1-AAx1-AFx5-AAx2-AEx1-AAx1-AEx1-AAx1-AFx2-AAx2-AFx1-AAx1-AFx1-AAx1-ABx1-ADx1-ACx13-AGx1-AJx1-ACx1-ABx20-ACx1-ADx1-AFx5-AAx1-ADx2-ABx1-ADx2-AAx2-AEx1-AAx1-AEx1-AAx2-ABx1-ACx18-AGx1-AJx1-AGx1-ADx1-ABx17-ACx2-AFx3-AAx2-ADx2-ACx3-ABx5-ADx1-ABx1-ACx23-AGx2-ADx1-ABx14-ACx3-AAx4-ABx1-ADx2-ACx2-ADx1-ABx1-AEx1-ABx1-ACx1-ADx1-ACx2-AGx2-ALx2-ACx16-AJx2-AGx2-AJx1-AMx1-AJx1-ACx1-ADx1-ABx10-ADx1-ACx3-AAx3-AEx1-ABx1-ADx2-ACx4-ABx1-ACx6-AGx1-ALx1-ACx12-AGx2-AJx3-AMx8-AJx1-ACx1-ADx2-ABx6-ADx1-ACx3-ADx1-AFx2-AAx1-ABx2-ADx2-ACx2-ADx1-ACx4-AGx1-ACx3-AJx1-ACx1-ALx1-ANx1-AIx4-ANx1-ACx2-AJx1-AMx3-AJx6-AMx6-AOx1-AMx1-AGx1-ADx6-ACx5-ADx1-AAx3-ABx1-ADx3-ACx2-ABx1-AIx2-ACx1-AGx2-ALx1-ACx2-AJx1-ACx2-ALx1-ANx1-AIx2-ANx2-ALx2-AJx1-AMx2-AJx1-AMx1-AOx2-AJx1-ALx1-AJx2-AMx5-AOx3-AJx1-ACx3-ADx1-ACx2-AGx2-ACx2-ADx1-AAx1-AEx1-ABx2-AHx2-ADx1-ACx3-AIx2-ACx1-AGx2-ACx3-AJx2-ALx2-ANx6-ACx1-AJx1-AMx2-AJx1-AOx3-AMx1-ACx1-AJx1-AMx7-AOx3-AMx1-AGx1-ACx5-AGx1-ACx2-ADx1-AAx2-ABx2-AHx2-AIx2-AHx1-AIx1-ACx3-AGx1-ACx4-AJx1-AMx1-AJx1-ALx1-ANx6-ALx1-AJx1-AMx2-AJx1-AOx3-AMx1-ALx1-AJx1-AMx7-AOx5-AMx1-AGx1-AJx1-ACx6-AFx1-AAx1-ABx2-AHx2-AIx3-ANx1-ACx5-AJx4-AMx1-AJx1-ALx1-ACx3-ANx2-ALx1-APx1-AJx1-AMx2-AJx1-AOx3-AMx1-AJx1-AMx8-AOx7-AMx1-AJx1-ACx5-AAx2-ABx2-AHx2-AIx3-ANx2-ACx2-AGx4-AJx3-ALx1-ACx6-AMx6-AOx3-AMx1-AJx1-AMx8-AOx9-AMx1-ACx3-ADx1-AAx2-ABx2-AHx1-AIx3-ANx5-ACx1-ALx1-ACx12-AMx1-AJx2-AMx2-AJx1-AMx1-AOx3-AJx1-AMx1-AOx2-AMx2-AOx14-AMx1-ACx1-ADx1-AFx3-ADx1-ACx1-AIx2-ANx5-ALx2-ACx1-ALx1-AJx2-ACx7-ANx1-ACx1-ALx1-AJx1-AMx4-AOx4-AMx2-AOx21-AFx3-AAx1-ACx3-ALx2-ANx1-ALx4-AJx4-AMx1-AJx4-ALx4-AMx2-AOx6-AMx3-AOx11-AMx9-AJx2-AAx4-ABx1-ACx3-ALx7-AGx2-AJx2-AMx1-AJx3-AMx1-APx1-ALx2-AMx3-AOx3-AMx3-AOx13-AMx11-AGx2-AJx1-AGx1-AJx1-AGx3-ALx2-AJx2-ALx1-AJx1-AGx1-AJx10-ALx3-APx1-AMx6-AJx1-AMx1-AOx22-AMx1-AGx1-AJx14-AMx3-AJx2-AMx1-AJx3-AMx1-AJx1-ALx3-AJx4-AMx2-AOx25-AGx3-AJx1-AGx3-AJx2-AMx7-AOx3-AMx1-AOx1-AMx10-AOx2-AMx2-AOx7-AMx1-AOx5-AMx12-AGx4-AJx1-AGx1-AJx1-AGx5-AJx3-AMx3-AOx12-AMx3-AJx3-AMx1-AJx2-AGx2-AJx1-AGx1-AJx1-AGx1-AJx2-AGx2-AJx1-AMx7-AJx3-AGx4-AJx1-AGx3-AJx6-AGx5-AJx5-AMx1-AOx1-AJx16-AGx4-AJx8-AMx1-AJx5-AGx3-ACx3-AGx2-AJx7-AGx2-AJx5-AGx1-AJx33-AGx6-ACx1-AGx5-AJx1-AGx5-AJx2-AGx7-AJx3-AGx2-AJx1-AGx1-AJx11-AGx5-AJx11-AGx2-AJx1-AGx18-AJx1-AGx5-AJx2-AGx10-AJx2-AGx6-AJx1-AGx13-AJx1-AGx1-ACx1-";
        break;
      case 23:
        theFullImage = "AA#D09D78,AB#DFAD86,AC#886842,AD#9D7853,AE#B48B65,AF#6C5535,AG#EFBC94,AH#CA7D5A,AI#BB6D4A,AJ#A14F2C,AK#392416,AL#4F3424,AM#723C1F,AN#1D1008,AO#602913,AP#4E1708;AAx2-ABx1-AAx5-ABx1-AAx1-ACx8-ADx1-AEx2-ADx2-ACx1-AFx7-ACx1-ADx2-ACx1-ADx12-ACx12-ADx1-AAx8-ABx2-AAx1-ADx1-AEx1-AAx10-ADx3-ACx1-AFx1-ACx10-ADx7-ACx15-AGx1-ABx2-AGx4-ABx7-AGx1-AAx3-ABx1-AAx1-AHx1-AIx1-AJx4-ACx3-AFx1-AKx3-AFx1-ACx10-ADx3-ACx13-AGx1-ABx1-AGx1-ABx9-AAx2-ADx1-ACx2-AJx1-AHx1-AEx1-AAx1-ABx1-ADx1-ACx5-AFx5-ALx1-AKx2-ALx1-ACx23-ABx7-AAx3-AEx1-AAx1-AGx2-AEx1-AAx3-ABx2-AAx1-ACx4-AFx1-ACx1-AFx10-AKx2-AFx1-ACx20-ABx5-AAx1-ADx5-AAx2-ABx1-AAx1-ABx1-AAx2-AEx1-ACx7-AFx12-ALx1-AKx2-ALx1-ACx18-ABx4-ADx1-ACx5-ADx1-AEx1-ADx3-ACx1-AAx1-ACx1-AFx6-ACx8-AFx1-ACx2-AFx6-ALx1-AKx1-ALx1-ACx16-ABx3-AEx1-ACx6-ADx2-ACx2-ADx1-AFx2-ALx3-AFx6-ACx6-AFx11-ALx1-AKx2-AFx1-ACx13-ABx2-AAx1-ACx7-ADx2-AFx6-ALx3-AMx2-AJx1-AFx1-ACx5-AFx3-ACx1-AFx3-AMx1-AFx1-AKx7-ANx1-AKx1-AFx1-ACx11-ABx1-AAx1-ADx1-ACx9-AFx1-ALx1-AFx4-ALx2-AMx2-AJx6-ACx1-AFx2-AJx1-ALx1-AKx6-ANx10-AKx1-ACx10-ABx1-AAx1-ADx1-ACx8-AFx2-ALx3-AFx2-AKx1-ALx2-AMx1-AJx6-AMx3-AKx1-ANx3-AKx3-AOx1-AKx3-ANx9-AFx1-ACx6-AFx1-ALx1-ABx1-AEx1-ADx1-ACx8-AJx1-AFx1-AKx2-ALx1-AMx1-AFx1-ALx1-AKx1-AMx3-AJx5-AMx2-AKx1-ANx3-AKx1-ANx4-AKx1-AOx1-AKx3-ANx8-AKx1-AFx3-ACx1-AFx2-ALx1-AAx1-ADx2-ACx4-AJx3-AFx2-AMx1-ALx2-AMx2-AFx1-ALx1-AKx1-ALx1-AMx3-AJx3-AMx3-ALx1-AKx1-ANx1-AKx2-ANx4-AKx1-ALx1-AKx1-ANx12-ALx1-AFx2-ACx1-AFx2-AAx1-ADx2-AIx1-AJx7-ACx1-AFx1-ALx1-AMx3-AFx1-ALx1-AKx1-AOx2-AMx2-AJx2-AMx3-AJx1-ALx1-AKx4-ANx4-AKx1-ALx1-AKx1-ANx13-AKx3-ACx1-AFx1-AAx1-ADx2-AIx1-ACx2-AJx5-ACx1-ALx1-AFx2-ALx1-AKx1-ALx1-AKx2-APx1-AOx1-AMx8-AKx2-ANx2-AKx1-ANx4-AKx2-ANx4-AKx1-ANx11-AKx2-ACx1-AAx1-ACx3-AJx4-AMx1-AJx1-AMx2-ALx1-AFx1-ALx1-AKx5-AOx1-AMx8-APx1-AKx1-ANx8-AKx2-ANx3-AKx1-ANx14-AKx1-AAx1-ADx1-ACx1-AIx1-AJx6-AMx2-ALx4-AMx1-ALx3-AMx2-AFx2-AMx4-AKx2-AOx1-AKx1-ANx3-AKx1-ANx4-AKx1-ANx19-AAx1-ADx1-ACx1-AJx6-AMx8-AJx1-AFx1-AJx6-AMx1-AJx2-ALx1-AOx2-AKx4-ANx5-AKx1-ANx19-AGx1-AAx1-AJx5-AMx1-AJx1-AMx5-ALx1-AKx5-AMx3-AJx2-AMx4-AKx2-ANx29-AGx2-AEx1-AMx11-ALx1-AKx10-AMx3-AOx1-AKx1-ANx30-ABx2-AAx1-ACx1-AMx10-ALx2-AKx9-AOx4-APx1-AKx1-ANx28-AKx6-ALx5-AOx1-AMx2-AOx1-AKx12-AMx3-AOx2-APx1-AKx1-ANx4-AKx2-ANx21-AKx15-ANx3-AKx2-ANx2-AKx4-AOx1-AMx1-AOx3-AKx6-ANx23-AKx3-ALx2-AKx9-ANx1-AKx1-ANx9-AKx6-ANx29-ALx3-AKx3-ALx3-AKx5-ANx21-AKx3-ANx2-AKx3-ANx2-AKx1-ANx4-AKx4-ANx2-AKx1-ANx2-AKx6-ALx1-AKx8-ALx1-AKx4-ANx9-AKx16-ALx7-AKx9-ALx5-AKx3-ANx1-AKx51-AFx1-ALx3-AKx1-ALx1-AKx3-ALx4-AKx47-ALx4-AKx1-ALx9-AKx4-ALx3-AKx39-ALx18-AKx2-ALx3-AKx4-ALx4-AKx5-ALx1-AKx16-ALx5-";
        break;
      case 24:
        theFullImage = "AA#CFA17C,AB#AF825C,AC#8D6C44,AD#705632,AE#E7B089,AF#B76947,AG#D08A65,AH#A05330,AI#3F2A1D,AJ#523929,AK#F3C39F,AL#612D15,AM#2E1B10,AN#7D401E,AO#200504,AP#4B1307;AAx9-ABx1-ACx14-ADx8-ACx1-ABx9-ACx1-ABx3-ACx14-AAx8-AEx2-ABx1-ACx1-ABx6-AAx5-ABx1-AFx1-ABx2-ADx2-ACx11-ABx1-ACx17-ADx2-AEx11-AAx1-AEx3-AAx1-ABx1-AGx1-AAx2-AGx1-ABx1-AHx2-AFx1-AHx1-ADx3-ACx1-ADx1-AIx3-ADx1-ACx25-AEx12-AAx2-ABx1-ACx2-AHx1-AFx1-ABx1-AAx1-AEx1-AAx1-ACx4-ADx6-AJx1-AIx3-ADx1-ACx22-AEx6-AAx4-ABx1-AAx1-AKx1-AEx1-AAx4-AEx2-AAx1-ABx1-ACx2-ADx13-AJx1-AIx2-ADx1-ACx19-AEx1-AAx1-AEx2-AAx1-ABx4-ACx1-AAx4-AGx1-AAx4-ABx1-ACx1-ADx18-AJx1-AIx2-ADx1-ACx17-AEx3-AAx1-ACx4-ADx1-ACx1-ABx1-AAx1-ABx3-ACx1-AAx1-ACx1-ADx24-AIx2-ADx1-ACx15-AEx2-AAx1-ACx3-ADx3-ABx1-ACx4-ABx1-ADx2-AJx1-ADx1-AJx1-ADx24-AIx2-ADx1-ACx13-AEx2-ABx1-ACx5-ADx1-ACx1-ABx1-ACx1-ADx3-AJx5-ALx1-AJx1-ADx18-AJx1-AIx5-AMx2-AIx1-ADx1-ACx10-AAx1-ABx1-ACx4-ADx4-ACx1-ADx2-AJx2-ADx2-AJx2-ALx1-AJx1-ALx1-ANx2-AHx2-ADx8-AJx1-AIx5-AMx1-AOx2-AMx1-AOx2-AMx2-AOx1-AMx1-AIx1-ADx1-ACx8-AAx1-ABx1-ACx3-ADx3-ACx2-ADx2-AJx1-AIx2-AJx1-ADx2-AIx1-AJx2-ANx2-AHx6-ANx1-ADx2-AIx1-AMx1-AOx1-AMx1-AIx4-AMx3-AOx3-AMx1-AOx4-AMx1-ADx1-ACx7-AAx1-ABx1-ACx4-ADx2-ACx2-ADx3-AIx2-AJx1-ADx2-AIx2-AJx1-ALx1-ANx2-AHx4-ANx2-AJx1-AIx1-AMx1-AOx2-AIx1-AMx1-AOx1-AMx1-AIx1-ALx2-AMx5-AOx6-AIx1-ADx2-ACx4-ABx2-ACx4-ADx7-AIx2-AJx1-ADx2-AIx2-ALx3-ANx6-ALx2-AIx1-AMx1-AOx1-AMx1-AIx1-AOx4-AIx1-ALx2-AIx1-AMx3-AOx2-AMx1-AOx4-AIx1-ADx5-ABx2-ACx3-ADx1-AHx3-ADx3-AJx1-AIx1-AJx1-ADx3-AIx1-AMx1-ALx3-ANx8-AIx2-AMx2-AIx1-AOx4-AIx1-ALx1-AIx1-AMx4-AOx9-AIx1-AJx2-ADx1-ABx2-AFx2-ACx2-AHx3-ADx3-AJx2-ADx2-AJx2-AIx1-AMx2-ALx2-ANx7-ALx1-AIx1-AMx3-AIx1-AOx4-AIx2-AMx2-AOx1-AMx4-AOx9-AIx2-ABx1-ACx3-AHx5-ANx1-ADx2-AJx1-ADx1-AJx1-AIx3-AMx2-ALx2-ANx7-ALx1-AMx1-AIx1-AMx3-AIx1-AOx4-AMx1-AIx1-AMx1-AOx2-AMx4-AOx10-AMx1-ABx1-ACx2-AFx1-AHx4-ANx3-ALx1-AJx1-ADx1-AJx1-AIx4-AMx1-ALx2-ADx4-AJx1-ADx1-AJx1-AMx4-AOx2-AMx1-AOx4-AMx1-AIx1-AMx1-AOx2-AMx5-AOx10-ABx1-ACx2-AFx1-AHx2-ANx1-AHx1-ANx1-AHx1-ANx2-AJx3-ALx2-AJx2-ANx3-ADx4-AJx1-ADx1-AJx1-AMx1-ALx1-AMx6-AOx3-AMx1-AIx1-AMx1-AOx2-AMx2-AOx13-AAx1-ACx1-AHx5-ANx5-ALx4-ANx1-ADx2-ANx1-AHx2-ADx3-ANx1-ADx1-ANx2-ALx3-AIx1-AMx3-AOx5-AMx1-AOx18-AEx1-ABx1-ADx1-AHx1-ANx6-ALx2-ANx1-ALx1-AIx3-AMx2-AIx1-AJx2-ALx1-ANx5-ALx1-AIx1-AMx2-AOx27-AMx1-AEx2-ACx1-ANx6-ALx5-AIx5-AMx2-AIx2-AMx1-AIx1-ALx4-AMx2-AOx7-AMx2-AOx16-AMx4-ABx2-ACx1-AJx3-ALx7-ADx1-AIx6-AMx1-AIx3-AMx1-APx1-ALx3-APx1-AMx2-AOx26-AMx2-AIx6-AJx3-AIx1-ALx3-AIx7-AMx1-AIx4-AMx1-ALx5-APx1-AIx1-AMx1-AOx3-AMx3-AOx20-AIx12-AMx1-AIx2-AMx1-AOx1-AMx5-AIx2-AMx2-ALx6-AIx4-AMx2-AOx22-AIx2-AJx1-AIx5-AMx8-AOx3-AMx2-AOx1-AMx6-AIx1-AMx5-AOx24-AMx2-AIx7-AJx1-AIx6-AMx1-AOx20-AMx2-AIx1-AMx4-AIx3-AMx1-AIx1-AMx3-AOx1-AIx4-AMx5-AIx19-AMx5-AOx3-AMx3-AIx12-AMx1-AIx4-AJx4-AIx3-AMx1-AIx1-AMx3-AIx1-AJx1-AIx6-AMx2-AIx3-AMx1-AIx13-AMx1-AIx33-ADx1-AJx3-AIx9-AMx1-";
        break;
      case 25:
        theFullImage = "AA#CFA17B,AB#AE825B,AC#8D6C44,AD#795834,AE#604C2C,AF#E6AF88,AG#B96542,AH#CC8865,AI#A25331,AJ#503525,AK#352218,AL#1B100A,AM#793D1D,AN#5A200E,AO#F5BF98;AAx9-ABx1-ACx14-ADx2-AEx2-ADx4-ACx5-ABx6-ACx1-ABx1-ACx1-ABx1-ACx10-ADx2-ACx1-AAx8-AFx2-ABx1-ACx1-ABx6-AAx5-ABx1-AGx2-AHx1-ACx1-ADx1-ACx1-ABx1-ACx10-ABx1-ACx18-AFx11-AAx2-AFx1-AAx2-ABx3-AAx3-ABx1-AIx1-AGx2-ADx1-ACx1-ADx3-AJx2-AKx1-AJx1-AEx1-ADx1-ACx23-AFx12-AAx3-ACx3-ABx1-AHx1-AAx3-ACx3-ADx9-AEx1-AJx2-AEx1-ACx21-AFx6-AAx4-ABx1-AAx1-AFx2-AAx3-AFx3-AAx1-ABx1-ACx2-ADx13-AEx2-AJx2-ADx1-ACx18-AFx1-AAx1-AFx3-ABx1-ACx1-ABx2-ACx1-ABx1-AAx4-AHx1-AFx1-AAx2-ABx1-ACx1-ADx18-AEx2-AJx2-AEx1-ACx16-AFx4-ACx4-ADx1-ACx2-ABx2-ACx1-ABx1-ACx1-AAx1-ACx1-ADx25-AEx1-AJx1-AEx1-ACx14-AFx3-ABx1-ACx3-ADx2-ABx1-ACx3-ADx1-ABx1-ADx2-AEx6-ADx17-AEx6-AKx1-AJx1-ADx1-ACx11-AFx2-ABx1-ACx3-ADx4-ACx2-ADx3-AEx5-AJx3-ADx16-AEx2-AJx1-AKx6-ALx1-AKx1-ADx1-ACx9-AAx2-ACx4-ADx4-ACx1-ADx2-AEx3-ADx1-AEx2-AJx3-AMx2-ADx9-AEx1-AJx1-AKx6-ALx10-AJx1-ADx1-ACx7-AFx1-ABx1-ACx4-ADx2-ACx1-ADx3-AEx2-AJx2-AEx1-ADx1-AJx4-AMx2-AIx3-AMx2-ADx2-AMx1-AJx1-ALx3-AKx3-AJx1-AKx4-ALx9-AEx1-ACx6-AAx1-ABx1-ACx4-ADx2-ACx1-ADx1-ACx1-ADx2-AJx4-AEx2-AKx1-AJx2-AMx3-AIx2-AMx3-AJx2-ALx4-AKx1-ALx3-AKx1-ANx2-AKx3-ALx8-AJx1-ADx4-ACx1-AAx1-ABx1-ACx4-ADx1-ACx1-ADx1-ACx2-ADx2-AJx2-AEx1-AJx1-ADx1-AEx1-AKx1-AJx1-AMx9-AJx2-AKx1-ALx2-AKx1-ALx5-ANx1-AJx1-AKx2-ALx10-AKx1-AEx1-ADx3-ABx2-ACx4-ADx2-AMx1-ADx4-AJx1-AEx1-AMx1-ADx2-AEx1-ALx1-AKx1-ANx2-AMx7-ADx1-AJx1-AKx4-ALx5-AJx2-AKx1-ALx13-AKx1-AEx2-ABx2-ACx1-AGx1-ACx1-AGx2-ADx1-AIx1-ADx3-AEx4-AJx1-AEx1-AKx3-ANx1-AMx8-AJx2-AKx1-ALx2-AKx1-ALx5-AJx1-AKx1-ALx16-AKx1-ABx2-ACx1-AGx1-ACx1-ADx2-AIx1-AMx1-ADx3-AEx3-AJx1-AKx1-AJx1-AKx2-ANx2-AMx2-ADx1-AMx5-ANx1-AKx2-ALx2-AKx2-ALx4-AKx2-ALx17-ABx2-ACx1-AGx2-ADx1-AIx2-AMx2-ADx1-AMx1-AEx3-AJx3-AKx2-AJx2-AMx1-AEx1-ADx1-AEx1-AMx1-ADx1-AMx1-AKx3-ALx9-AKx2-ALx3-AKx2-ALx12-ABx1-ACx2-AGx1-AIx1-ADx1-AMx1-AIx1-AMx4-AJx1-AEx1-AJx1-AEx1-AMx1-AJx2-AMx3-ADx4-AJx1-AMx1-AEx1-AKx2-ANx1-ALx4-AKx1-ALx5-AKx1-ALx17-AAx1-ACx1-ADx3-AIx1-AMx9-AJx1-AEx2-ADx9-AMx2-AJx1-ANx2-AKx2-ALx1-AKx2-ALx5-AKx1-ALx17-AOx1-ABx1-ADx1-AIx1-AMx10-AKx6-AJx1-AMx9-AKx1-ALx29-AFx2-ABx1-ADx2-AMx9-AJx4-AKx7-ANx1-AMx2-ANx2-AKx1-ALx29-ABx2-ACx1-ADx1-AEx1-AJx1-AMx5-ANx1-AMx1-ADx1-AJx5-AKx3-AJx1-AKx2-ALx1-ANx5-ALx29-AJx6-AEx2-AJx2-ANx3-AJx2-AKx3-AJx2-AKx2-AJx2-AKx2-ANx6-ALx7-AKx2-ALx19-AJx2-AKx4-AJx3-AKx2-AJx1-AKx3-ALx3-AKx3-ALx1-AKx1-AJx1-AKx2-ANx7-AKx6-ALx21-AJx2-AEx2-AJx2-AKx4-ALx1-AKx3-ALx1-AKx1-ALx7-AKx1-ALx2-AKx1-ANx1-AKx1-ANx1-AKx3-ALx27-AJx3-AKx2-AJx1-AEx3-AJx1-AKx4-ALx23-AKx2-ALx3-AKx3-ALx1-AKx2-ALx4-AKx2-AJx1-AKx1-ALx4-AJx2-AKx1-AJx3-AKx3-AJx1-AKx2-AJx5-AKx2-ALx12-AKx13-AJx3-AEx6-AJx1-AKx1-ALx1-AKx1-ALx1-AKx2-AJx7-AKx5-ALx1-AKx5-AJx1-AKx1-AJx2-AKx4-ALx1-AKx1-AJx3-AKx18-AJx1-AKx9-AJx1-AEx3-AJx4-AKx2-AJx4-AKx1-AJx5-AKx3-AJx2-AKx10-AJx1-AKx1-AJx11-AKx1-AJx1-AKx1-AJx9-AKx1-AEx3-AJx6-AEx1-AJx3-AKx2-AJx1-AEx1-AJx15-AKx4-";
        break;
  #pragma endregion images

    }

    fullImages[i].importedImage = theFullImage;
    theFullImage = "";
    importToImage(i);
    fullImages[i].importedImage = "";
    wait(100, msec);
  }
  
  Controller1.Screen.clearScreen();
  refreshScreen(true, true, true);

  return;
}

/** AUTONOMOUS **/
void StartAutonomous(int mode){ // All autonomous actions should happen here
  LeftDriveMotors.setTimeout(2, seconds);
  RightDriveMotors.setTimeout(2, seconds);
  Brain.Timer.reset();

  if(mode == 1){

  }

  Brain.Screen.clearScreen();
  Brain.Screen.setCursor(1, 1);
  Brain.Screen.print("Time: %f", Brain.Timer.value());
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
  int failsafe = 0;
  FingerMotor.setStopping(hold);
  FingerMotor.setTimeout(1, seconds); // Do not hurt kid named finger :(
    
  if(fingerMode == 1){ // FINGER OUT!
    FingerMotor.setVelocity(30, percent);
    FingerMotor.spin(forward);
    if(FingerMotor.position(turns) >= 0.2){
      FingerMotor.stop();
      fingerMode = 2;
    }
  }
  else if(fingerMode == 2){ // FINGER RETURN!
    FingerMotor.setVelocity(30, percent);
    FingerMotor.spin(reverse);
    failsafe ++;
    if(FingerMotor.position(turns) < 0 || failsafe >= 30){
      FingerMotor.stop();
      fingerMode = 0;
      if(fingerButtonPressed == false){
        refreshScreen(false, false, true);
      }
    }
  }
  /*
  if(fingerMode == 1){

    FingerMotor.setVelocity(40, percent);
    FingerMotor.spinFor(forward, 60, degrees);
    FingerMotor.spinFor(reverse, 60, degrees);
    FingerMotor.spinToPosition(60, degrees);
    FingerMotor.spinToPosition(20, degrees);

    FingerMotor.stop();
    fingerMode = 0;
  }*/
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
  nextScreenRef = 50;

}

#pragma endregion RC_FUNCTIONS

#pragma region RC

void usercontrol(void) {
  // User control code here, inside the loop
  Brain.Timer.reset();

  int counter = 0;

  Brain.Screen.clearScreen();
  Brain.Screen.setPenWidth(0);
  
  drawCurrentImage();


  while (1) {
    // The funny
    if(waltMode == 0){
      counter ++;
      if(counter > 40){
        curImage = curImage == 0 ? 1 : 0;
        counter = 0;
        drawCurrentImage();
      }
    }
    if(waltMode == 1){
      counter ++;
      if(counter > 3){
        counter = 0;
        curImage ++;
        if(curImage >= 26){
          curImage = 25;
          waltMode = 2;

        }
        drawCurrentImage();
      }
    }

    // Regular boring RC code
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

      //accelLeftSpd += (leftSpd - accelLeftSpd) > maxAccel ? maxAccel : ((leftSpd - accelLeftSpd) < -maxAccel ? -maxAccel : (leftSpd - accelLeftSpd));
      //accelRightSpd += (rightSpd - accelRightSpd) > maxAccel ? maxAccel : ((rightSpd - accelRightSpd) < -maxAccel ? -maxAccel : (rightSpd - accelRightSpd));
      
      // Set velocities of motors
      LeftDriveMotors.setVelocity(leftSpd, percent);
      RightDriveMotors.setVelocity(rightSpd, percent);
      LeftDriveMotors.spin(forward);
      RightDriveMotors.spin(forward);
      //FlywheelMotor.setVelocity(100, percent);
    }
    else{
      // Stop motors
      leftSpd = 0;
      rightSpd = 0;
      
      accelLeftSpd += (leftSpd - accelLeftSpd) > maxAccel ? maxAccel : ((leftSpd - accelLeftSpd) < -maxAccel ? -maxAccel : (leftSpd - accelLeftSpd));
      accelRightSpd += (rightSpd - accelRightSpd) > maxAccel ? maxAccel : ((rightSpd - accelRightSpd) < -maxAccel ? -maxAccel : (rightSpd - accelRightSpd));

      LeftDriveMotors.setVelocity(accelLeftSpd, percent);
      RightDriveMotors.setVelocity(accelRightSpd, percent);
      LeftDriveMotors.spin(forward);
      RightDriveMotors.spin(forward);
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
    if(!conveyorOn){
      if(Controller1.ButtonL2.pressing()){
        ConveyorMotor.setVelocity(100, percent);
        ConveyorMotor.spin(reverse);
      }
      else if(Controller1.ButtonR2.pressing()){
        ConveyorMotor.setVelocity(100, percent);
        ConveyorMotor.spin(forward);
      }
      else{
        ConveyorMotor.stop();
      }
    }

    // Finger - Update "fingerMode" to 1 to start finger sequence
    
    if(Controller1.ButtonL1.pressing()){ 
      FingerMotor.setVelocity(60, percent);
      FingerMotor.spin(reverse);
    }
    else{
      FingerMotor.stop();
    }

    // Expansion
    if(Controller1.ButtonB.pressing() && ((Brain.Timer.value() > 95.0 && !runningSkills) || ((Brain.Timer.value() > 50.0 && runningSkills)) || (Controller1.ButtonL2.pressing() && Controller1.ButtonR2.pressing()))) {
      Pneumatic.set(true);
      if(waltMode < 1){
        waltMode = 1;
        curImage = 0;
        drawCurrentImage();
      }
    }

    // Screen Refresh every x msec (Not every check unless you want to REALLY lag the brain/controller)
    screenRefCount +=1;
    if(screenRefCount >= nextScreenRef){ // Have we reached the next screen refresh?
      screenRefCount = 0; // Reset counter
      
      if(nextScreenRef == 50){
        nextScreenRef = 100;
        refreshScreen(true, true, true); // Run refresh screen function
        lastBatteryAmt = Brain.Battery.capacity(); // Update "lastBatteryAmt" to remember what the battery % was last
      }
      else{
        screenInformationMode ++;
        if(screenInformationMode > 2){
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