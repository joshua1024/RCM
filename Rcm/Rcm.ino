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
float speedVal = 0;
float turnVal = 0;
float trimVal = 0;
float intake = 0;
float sensor = 0;
float leftSpeed = 0;
float rightSpeed = 0;
float shoulder = 0;
float elbow = 0;

void Enabled() { //code to run while enabled
  leftSpeed = speedVal + turnVal * (trimVal + 1);
  rightSpeed = speedVal - turnVal * (-trimVal + 1);
  setMot(portA, rightSpeed);
  setMot(portB, leftSpeed);
  setMot(portC, rightSpeed);
  setMot(portD, leftSpeed);
  setSer(port1, intake);
  setSer(port2, -intake);
  setSer(port3, shoulder);
  setSer(port4, elbow);
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
