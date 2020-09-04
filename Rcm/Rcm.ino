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
float val1 = 0.0;
float val2 = 0.0;
float val3 = 0.0;
float val4 = 0.0;
float val5 = 0.0;
float val6 = 0.0;
float val7 = 0.0;
float val8 = 0.0;

void Enabled() { //code to run while enabled
  setMot(portA, val1);
  setMot(portB, val2);
  setMot(portC, val3);
  setMot(portD, val4);
  setSer(port1, -val5, 1280, 1480);
  setSer(port2, val6, 1500, 1000);
  setSer(port3, val7, 1500, 1000);
  setSer(port4, val8, 1500, 1000);
  setSer(port5, val8, 1500, 1000);
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
  enableSer(port5);
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
  disableSer(port5);
}

void PowerOn() { //runs once on robot startup
}

void Always() { //always runs if void loop is running, don't control outputs here
}

//you can communicate booleans, bytes, ints, floats, and vectors
void WifiDataToParse() {
  wifiArrayCounter = 0;
  enabled = recvBl();
  //add data to read here:
  val1 = recvFl();
  val2 = recvFl();
  val3 = recvFl();
  val4 = recvFl();
  val5 = recvFl();
  val6 = recvFl();
  val7 = recvFl();
  val8 = recvFl();

}
int WifiDataToSend() {
  wifiArrayCounter = 0;
  sendFl(batVoltAvg);
  //add data to send here:
  sendFl(0);
  sendFl(0);
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
