#include <Servo.h>

#define GEAR_LIGHT_CH 1
#define DIFFERENTIAL_CH 2
#define WINCH_CH 3

// input
#define GEAR_LIGHT_PIN 8
#define DIFFERENTIAL_PIN 9
#define WINCH_PIN 10

// output
#define GEAR_PIN 3
#define DIFFERENTIAL_FRONT_PIN 5
#define DIFFERENTIAL_BACK_PIN 6
#define LIGHT_PIN 16
#define WINCH_IN_PIN A2
#define WINCH_OUT_PIN A3

// config
// time you need to have channel one above upper bound for light to toggle
#define LIGHT_TIME 500
// time you need to have channel one below lower bound for gear to toggle
#define GEAR_TIME 20
#define GEAR_LIGHT_UPPER_BOUND 0.5
#define GEAR_LIGHT_LOWER_BOUND -0.5
// triggers if higher
#define DIFFERENTIAL_FRONT_BOUND 0.5
// triggers if lower
#define DIFFERENTIAL_BACK_BOUND -0.5
// double click channel 1 to reset all gear, light and differentials
#define DOUBLEE_CLICK_TIME 500

// servo positions in microseconds
#define GEAR_LOW 2000
#define GEAR_HIGH 1000
#define DIFFERENTIAL_FRONT_LOCK 1000
#define DIFFERENTIAL_FRONT_UNLOCK 2000
#define DIFFERENTIAL_BACK_LOCK 2000
#define DIFFERENTIAL_BACK_UNLOCK 1000

// default bounds for normalization
#define DEFAULT_MIN 1000
#define DEFAULT_MID 1500
#define DEFAULT_MAX 2000

#define LOCKED true
#define UNLOCKED false

#define LOG_LEVEL INFO
// https://docs.python.org/3/library/logging.html#logging-levels
#define CRITICAL 50
#define ERROR 40
#define WARNING 30
#define INFO 20
#define DEBUG 10
#define NOTSET 0


Servo gear_servo;
Servo differential_front_servo;
Servo differential_back_servo;

unsigned long now;
unsigned long rc_update;
unsigned long stat_update;
const int channels = 3;
float RC_in[channels];

float Min[channels] = {DEFAULT_MIN, DEFAULT_MIN, DEFAULT_MIN};
float Mid[channels] = {DEFAULT_MID, DEFAULT_MID, DEFAULT_MID};
float Max[channels] = {DEFAULT_MAX, DEFAULT_MAX, DEFAULT_MAX};
unsigned long useful_update[channels];

void recalibrate(int i, float p) {
  if (Min[i] == DEFAULT_MIN && p < DEFAULT_MID) Min[i] = p;
  else Min[i] = min(Min[i], p);
  if (Max[i] == DEFAULT_MAX && p > DEFAULT_MID) Max[i] = p;
  else Max[i] = max(Max[i], p);
  Mid[i] = (Min[i] + Max[i]) / 2;
}

void setup() {
  Serial.begin(9600);
  //while (!Serial); // uncomment to get all output BUT will block when not connected to USB
  Serial.println("starting setup");
  setup_pwmRead();

  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(WINCH_IN_PIN, OUTPUT);
  pinMode(WINCH_OUT_PIN, OUTPUT);
  gear_servo.attach(GEAR_PIN, 500, 2500);
  differential_front_servo.attach(DIFFERENTIAL_FRONT_PIN, 500, 2500);
  differential_back_servo.attach(DIFFERENTIAL_BACK_PIN, 500, 2500);
  delay(50);
  reset_state();
  Serial.println("Read to crawl!\n-------");
}

void reset_state() {
  log(INFO, "Reset!");
  set_low_gear();
  set_front(LOW);
  set_back(LOW);
  light_off();
  blink_hello();
}

void blink_hello() {
  light_on();
  delay(200);
  light_off();
  delay(200);
  light_on();
  delay(300);
  light_off();
}

void loop() {
  now = millis();
  if (RC_avail() || now - rc_update > 25) { // if RC data is available or 25ms has passed since last update (adjust to be equal or greater than the frame rate of receiver)
    rc_update = now;
    for (int i = 0; i < channels; i++) {    // run through each RC channel
      int CH = i + 1;
      if (PWM_read(CH)) {
        float p = PWM();
        float c = calibrate(p, Min[i], Mid[i], Max[i]);
        //Serial.println("Channel: " + String(CH) + " - " + String(p) + " - " + String(c));
        evaluate(CH, c);
      }
    }
  }
}


const int log_level = LOG_LEVEL;
void log(int level, String s) {
  if (level >= log_level)
    Serial.println(s);
}
