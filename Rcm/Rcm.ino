#include "rcmutil.h"
#include "wifi.h"
const char *routerName = "chicken";
const char *routerPass = "bawkbawk";
const char *APPass = "RCMpassword";
int port = 25211;
const boolean connectToNetwork = true; //true=try to connect to router  false=go straight to hotspot mode
const boolean wifiRestartNotHotspot = true; //when connection issue, true=retry connection to router  false=fall back to hotspot
const int SIGNAL_LOSS_TIMEOUT = 1000; //disable if no signal after this many milliseconds
//////////////////////////// add variables here
const unsigned int preIntakeDelay = 500;
const unsigned int postIntakeDelay = 250;

float speedVal = 0;
float trimVal = 0;
float turnVal = 0;
float lift = 0;
float climb = 0;
float intakeFl = 0;
float ejectFl = 0;
boolean revIntake = false;
boolean intake = false;
boolean eject = false;
boolean preIntake = false;
boolean postIntake = false;
boolean wasIntaking = false;
unsigned long intakeSwitch = 0;


void Enabled() { //code to run while enabled
  //delays for auto sequences
  intake = intakeFl == 1;
  revIntake = intakeFl == -1;
  eject = ejectFl == 1;
  preIntake = (intake && millis() - intakeSwitch < preIntakeDelay);
  postIntake = (!intake && millis() - intakeSwitch < postIntakeDelay);

  //Drive
  leftSpeed = speedVal + turnVal * (trimVal + 1);
  rightSpeed = speedVal - turnVal * (-trimVal + 1);
  setMot(portA, rightSpeed);
  setMot(portB, leftSpeed);

  //Climb
  setMot(portC, climb);

  //Lift
  setSer(port1, lift, 1500, 1000);
  if (lift > -.95) {
    intake = false;
  }

  //Chin
  if (intake) {
    setSer(port2, -1, 1500, 1000); //chin down
  } else {
    setSer(port2, 1, 1500, 1000); //chin up
  }

  //Claw
  if (eject || intake || postIntake) {
    setSer(port3, 1, 1500, 1000); //claw open
  } else {
    setSer(port3, -1, 1500, 1000); //claw close
  }

  //Intake
  if (revIntake) {
    setMot(portD, 1); //intake out
  } else  if (intake && !preIntake) {
    setMot(portD, -1); //intake in
  }

  //start timer on intake state change
  if (intake != wasIntaking) {
    intakeSwitch = millis();
  }
  wasIntaking = intake;
}

void Enable() { //turn on outputs
  enableMot(portA);
  enableMot(portB);
  enableMot(portC);
  enableMot(portD);
  enableSer(port1);
  enableSer(port2);
  enableSer(port3);
  enableSer(port4);
}

void Disable() { //shut off all outputs
  disableMot(portA);
  disableMot(portB);
  disableMot(portC);
  disableMot(portD);
  disableSer(port1);
  disableSer(port2);
  disableSer(port3);
  disableSer(port4);
}

void PowerOn() { //runs once on robot startup
  pinMode(inport1, INPUT);
}

void Always() { //always runs if void loop is running, don't control outputs here
  sensor = analogRead(inport1);
}

//you can communicate booleans, bytes, ints, floats, and vectors
void WifiDataToParse() {
  wifiArrayCounter = 0;
  enabled = recvBl();
  //add data to read here:
  speedVal = recvFl();
  trimVal = recvFl();
  turnVal = recvFl();
  intake = recvFl();
  shoulder = recvFl();
  elbow = recvFl();

}
int WifiDataToSend() {
  wifiArrayCounter = 0;
  sendFl(batVoltAvg);
  //add data to send here:
  sendFl(sensor);
  sendFl(0);
  sendFl(0);
  sendFl(0);
  sendFl(0);
  sendFl(0);
  sendFl(0);

  return wifiArrayCounter;
}

////////////////////////////////////////////////
void setup() {
  Disable();
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(BAT_PIN, INPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println("##########esp32 powered on.");
  setupWifi();
  batVoltAvg = analogRead(BAT_PIN) / DAC_UnitsPerVolt;
  PowerOn();
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
