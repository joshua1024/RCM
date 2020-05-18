boolean enabled = false;
boolean wasEnabled = false;
float batVolt = 0.0;
const int ONBOARD_LED = 2;
const float DAC_UnitsPerVolt = 413;
const int BAT_PIN = 36;
const int PWM_FREQ = 12000;
const int PWM_RES = 8;
const int PWM_RANGE = 255;
//          PWM_CH, PIN
#define port1 4,27
#define port2 5,26
#define port3 6,25
#define port4 7,33
#define port5 8,32
//           PWM_CH, EN_PIN, PIN1, PIN2
#define portA 0,13,15,14
#define portB 1,18,5,21
#define portC 2,17,16,4
#define portD 3,23,22,19


struct PVector {
  float x;
  float y;
};