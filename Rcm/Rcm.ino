#include "rcmutil.h"
#include "wifi.h"
#include <FastLED.h>

const char *routerName = "chicken";
const char *routerPass = "bawkbawk";
const char *APPass = "RCMpassword";
int port = 25212;
const boolean connectToNetwork = true; //true=try to connect to router  false=go straight to hotspot mode
const boolean wifiRestartNotHotspot = true; //when connection issue, true=retry connection to router  false=fall back to hotspot
const int SIGNAL_LOSS_TIMEOUT = 1000; //disable if no signal after this many milliseconds
//////////////////////////// add variables here
PVector move = {0, 0};
boolean climb = false;
boolean raiseArmToScore = false;
boolean loadingStationIntake = false;
boolean eject = false;
boolean autoIntake = false;
boolean autoMode = false;
boolean autoEject = false;
boolean autoStop = false;
float manualArm = 1;
float manualClaw = 0;
boolean jogMode = false;
float trim = 0;
byte upLidarP1 = 0;
byte upLidarP2 = 0;
byte upLidarP3 = 0;
byte frontLidarP1 = 0;
byte clawLidarP1 = 0;
byte backLidarP1 = 0;
byte upLidarMin = 0;
byte upLidarMax = 0;
byte clawLidarMin = 0;
byte clawLidarMax = 0;
byte frontLidarMin = 0;
byte frontLidarMax = 0;
byte backLidarMin = 0;
byte backLidarMax = 0;
byte leftClawCenter = 0;
byte leftClawRange = 0;
byte rightClawCenter = 0;
byte rightClawRange = 0;
byte armRange = 0;
byte armCenter = 0;
byte motPower = 0;
byte armSpeed = 0;
byte armAccel = 0;
boolean armSmooth = true;
boolean smoothDrive = true;
byte driveAcc = 0;
int loadIntakeDriveTime = 0;

float upLidar = 0;
byte upLidarP = 0;
float clawLidar = 0;
byte clawLidarP = 0;
float frontLidar = 0;
byte frontLidarP = 0;
float backLidar = 0;
byte backLidarP = 0;
boolean runningAutoIntakeRoutine = false;
boolean ejectReady = false;
boolean driveStopped = false;
boolean armMoving = false;

float leftSpeed = 0;
float rightSpeed = 0;
float leftWriteSpeed = 0;
float rightWriteSpeed = 0;
CRGB leds[12];
float armPos = -1;
float armPosSpeed = 0;
float armPosWrite = armPos;
float clawPos = 0;
unsigned long lastCycleMicros = 0;
unsigned long lastCycleIntervalMicros = 0;
boolean score = false;
unsigned long loadIntakeStartMillis = 0;
boolean loadStationIntake = false;

void Enabled() { //code to run while enabled
  if (!autoMode) {//manual
    armPos = manualArm;
    clawPos = manualClaw;
  } else { //auto mechanism
    if (autoIntake) {
      score = false;
      if (move.y - abs(move.x) > .1) {
        clawPos = 0;
      }
      if (move.y - abs(move.x) < -.1) {
        clawPos = -1;
      }
    }
    if (loadingStationIntake == true) {
      if (millis() - loadIntakeStartMillis > loadIntakeDriveTime * 7) {
        loadStationIntake = true;
        runningAutoIntakeRoutine = true;
      }
    }
    if (loadStationIntake) {
      if (!armMoving && clawLidarP == 1 && millis() - loadIntakeStartMillis > loadIntakeDriveTime * 7) {
        loadIntakeStartMillis = millis();
      }
      score = false;
      clawPos = 0;
      leftSpeed = (1.0 + move.x) * (trim + 1.0);
      rightSpeed = (1.0 - move.x) * (-trim + 1.0);
      if (millis() - loadIntakeStartMillis <= loadIntakeDriveTime) {
        runningAutoIntakeRoutine = true;
        clawPos = -1;
        leftSpeed = (-1.0) * (trim + 1.0);
        rightSpeed = (-1.0) * (-trim + 1.0);
      } else  if (millis() - loadIntakeStartMillis <= loadIntakeDriveTime * 2) {
        runningAutoIntakeRoutine = true;
        clawPos = 1;
        leftSpeed = (1.0) * (trim + 1.0);
        rightSpeed = (1.0) * (-trim + 1.0);
      } else  if (millis() - loadIntakeStartMillis <= loadIntakeDriveTime * 3) {
        runningAutoIntakeRoutine = true;
        clawPos = -1;
        leftSpeed = (-1.0) * (trim + 1.0);
        rightSpeed = (-1.0) * (-trim + 1.0);
      } else  if (millis() - loadIntakeStartMillis <= loadIntakeDriveTime * 4) {
        runningAutoIntakeRoutine = false;
        clawPos = -1;
        score = true;
        loadStationIntake = false;
      }
    }

    if (eject) {
      clawPos = 0;
      score = false;
      loadStationIntake = false;
      runningAutoIntakeRoutine = false;
    }

    if (raiseArmToScore) {
      score = true;
    }


    if (score == false) {
      armPos = -1;
    } else {
      armPos = .59;
      if (upLidarP != 0) {
        armPos = .8;
      }
      if (frontLidarP == 1) {
        armPos = -.55;
      }
      if (backLidarP == 1) {
        armPos = 1;
      }
    }



  }
  if (!runningAutoIntakeRoutine && !driveStopped) {
    leftSpeed = (move.y + move.x) * (trim + 1.0);
    rightSpeed = (move.y - move.x) * (-trim + 1.0);
  }
  if (armSmooth) {
    armPosSpeed = armAccelFunction();
    armPosWrite += armPosSpeed;
  } else {
    armPosWrite = armPos;
  }
  setSer(port1, -armPosWrite, 1500 + (armCenter - 127) * 5, 1000 + (armRange - 75) * 9);
  clawPos = constrain(clawPos, -1, 0);
  setSer(port5, -clawPos, (leftClawCenter - 127) * 5 + 1500, 1000 + (leftClawRange - 100) * 5);
  setSer(port4, clawPos, (rightClawCenter - 127) * 5 + 1500, 1000 + (rightClawRange - 100) * 5);
  setMotorCalibration(motPower / 100.0, .05);
  if (smoothDrive) {
    leftWriteSpeed += constrain(leftSpeed - leftWriteSpeed, -.08 / driveAcc, .08 / driveAcc);
    rightWriteSpeed += constrain(rightSpeed - rightWriteSpeed, -.08 / driveAcc, .08 / driveAcc);
  } else {
    leftWriteSpeed = leftSpeed;
    rightWriteSpeed = rightSpeed;
  }
  if (jogMode) {

  }
  setMot(portA, leftWriteSpeed);
  setMot(portB, rightWriteSpeed);

}

void Enable() { //turn on outputs
  enableMot(portA);
  enableMot(portB);
  enableSer(port1);
  enableSer(port4);
  enableSer(port5);
}

void Disable() { //shut off all outputs
  disableMot(portA);
  disableMot(portB);
  disableSer(port1);
  disableSer(port4);
  disableSer(port5);
}

void PowerOn() { //runs once on robot startup
  pinMode(inport1, INPUT);
  pinMode(inport1, INPUT);
  pinMode(inport1, INPUT);
  pinMode(port2Pin, INPUT);
  FastLED.addLeds<WS2812B, port3Pin, GRB>(leds, 12);
  leds[0] = CRGB(55, 0, 0);
  leds[1] = CRGB(0, 55, 0);
  leds[2] = CRGB(0, 55, 0);
  leds[3] = CRGB(0, 0, 55);
  leds[4] = CRGB(0, 0, 55);
  leds[5] = CRGB(0, 0, 55);
  leds[6] = CRGB(0, 0, 0);
  FastLED.show();
}

void Always() { //always runs if void loop is running, don't control outputs here
  upLidar = map(analogRead(port2Pin), (int)upLidarMin * 16, (int)upLidarMax * 16, 0, 10000) / 10000.0;
  upLidarP = 0;
  if (upLidar < upLidarP3 / 255.0) {
    upLidarP = 3;
  } else if (upLidar < upLidarP2 / 255.0) {
    upLidarP = 2;
  } else  if (upLidar < upLidarP1 / 255.0) {
    upLidarP = 1;
  }

  clawLidar = map(analogRead(inport2), (int)clawLidarMin * 16, (int)clawLidarMax * 16, 0, 10000) / 10000.0;
  clawLidarP = 0;
  if (clawLidar > clawLidarP1 / 255.0) {
    clawLidarP = 1;
  }
  frontLidar = map(analogRead(inport1), (int)frontLidarMin * 16, (int)frontLidarMax * 16, 0, 10000) / 10000.0;
  frontLidarP = 0;
  if (frontLidar > frontLidarP1 / 255.0) {
    frontLidarP = 1;
  }
  backLidar = map(analogRead(inport3), (int)backLidarMin * 16, (int)backLidarMax * 16, 0, 10000) / 10000.0;
  backLidarP = 0;
  if (backLidar > backLidarP1 / 255.0) {
    backLidarP = 1;
  }

  delayMicroseconds(100);
  lastCycleIntervalMicros = micros() - lastCycleMicros;
  lastCycleMicros = micros();
}

//you can communicate booleans, bytes, ints, floats, and vectors
void WifiDataToParse() {
  wifiArrayCounter = 0;
  enabled = recvBl();
  //add data to read here:
  move = recvVect();
  autoMode = recvBl();
  autoEject = recvBl();
  autoStop = recvBl();
  climb = recvBl();
  raiseArmToScore = recvBl();
  loadingStationIntake = recvBl();
  eject = recvBl();
  autoIntake = recvBl();
  manualArm = recvFl();
  manualClaw = recvFl();
  jogMode = recvBl();
  trim = recvFl();
  upLidarP1 = recvBy();
  upLidarP2 = recvBy();
  upLidarP3 = recvBy();
  frontLidarP1 = recvBy();
  clawLidarP1 = recvBy();
  backLidarP1 = recvBy();
  upLidarMin = recvBy();
  upLidarMax = recvBy();
  clawLidarMin = recvBy();
  clawLidarMax = recvBy();
  frontLidarMin = recvBy();
  frontLidarMax = recvBy();
  backLidarMin = recvBy();
  backLidarMax = recvBy();
  leftClawCenter = recvBy();
  leftClawRange = recvBy();
  rightClawCenter = recvBy();
  rightClawRange = recvBy();
  armCenter = recvBy();
  armRange = recvBy();
  motPower = recvBy();
  armAccel = recvBy();
  armSpeed = recvBy();
  armSmooth = recvBl();
  smoothDrive = recvBl();
  driveAcc = recvBy();
  loadIntakeDriveTime = recvBy() * 4;
}
int WifiDataToSend() {
  wifiArrayCounter = 0;
  sendFl(batVoltAvg);
  //add data to send here:
  sendFl(upLidar);
  sendBy(upLidarP);
  sendFl(clawLidar);
  sendBy(clawLidarP);
  sendFl(frontLidar);
  sendBy(frontLidarP);
  sendFl(backLidar);
  sendBy(backLidarP);
  sendBl(runningAutoIntakeRoutine);
  sendBl(ejectReady);
  sendBl(driveStopped);
  sendBl(armMoving);
  sendFl(clawPos);
  return wifiArrayCounter;
}

////////////////////////////////////////////////
void setup() {
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(BAT_PIN, INPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println("##########esp32 powered on.");
  setupWifi();
  batVoltAvg = analogRead(BAT_PIN) / DAC_UnitsPerVolt;
  PowerOn();
  Disable();
}

void loop() {
  batVolt = analogRead(BAT_PIN) / DAC_UnitsPerVolt;
  batVoltAvg = batVolt * .001 + batVoltAvg * (.999);
  wasEnabled = enabled;
  wifiComms();
  if (millis() - lastMessageTimeMillis > SIGNAL_LOSS_TIMEOUT) {
    enabled = false;
  }
  Always();
  if (enabled && !wasEnabled) {
    Enable();
  }
  if (!enabled && wasEnabled) {
    Disable();
  }
  if (enabled) {
    Enabled();
    digitalWrite(ONBOARD_LED, millis() % 500 < 250);
  } else {
    digitalWrite(ONBOARD_LED, HIGH);
  }
}
float armAccelFunction() {
  int big = 100;
  long distanceTo = (armPos - armPosWrite) * big;
  float _acceleration = .0000005 * big * armAccel / 50;
  float _maxSpeed = .0012 * big * armSpeed / 50;
  float _speed = armPosSpeed * big;

  // Max possible speed that can still decelerate in the available distance
  float requiredSpeed;
  if (distanceTo == 0) {
    armMoving = false;
    return 0.0; // We're there
  }
  else if (distanceTo > 0) // Clockwise
    requiredSpeed = sqrt(2.0 * distanceTo * _acceleration);
  else  // Anticlockwise
    requiredSpeed = -sqrt(2.0 * -distanceTo * _acceleration);

  if (requiredSpeed > _speed)
  {
    // Need to accelerate in clockwise direction
    if (_speed == 0)
      requiredSpeed = sqrt(2.0 * _acceleration);
    else
      requiredSpeed = _speed + abs(_acceleration / _speed);
    if (requiredSpeed > _maxSpeed)
      requiredSpeed = _maxSpeed;
  }
  else if (requiredSpeed < _speed)
  {
    // Need to accelerate in anticlockwise direction
    if (_speed == 0)
      requiredSpeed = -sqrt(2.0 * _acceleration);
    else
      requiredSpeed = _speed - abs(_acceleration / _speed);
    if (requiredSpeed < -_maxSpeed)
      requiredSpeed = -_maxSpeed;
  }
  armMoving = true;
  return 1.0 * requiredSpeed / big;
}
