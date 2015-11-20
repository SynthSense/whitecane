// Project: Whitecane
// Organization: Synthsense
// Authors: Adhitya Murali, Tomas Vega, Craig Hiller

#include <Wire.h>
#include "Adafruit_MCP23008.h"
#include <RFduinoBLE.h>

#define TRIGGER_PIN_1  6  // I/O extender ID
#define ECHO_PIN_1     2  // RFduino
#define TRIGGER_PIN_2  7  // I/O extender ID
#define ECHO_PIN_2     3  // RFduino

// These should be using the MCP23008 port expander
#define CANE_MOTOR_1        0
#define CANE_MOTOR_2        1
#define NAV_MOTOR_1         2
#define NAV_MOTOR_2         3
#define NAV_MOTOR_3         4
#define NAV_MOTOR_4         5

#define MAX_DISTANCE 500 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

enum state_type {
  INIT,
  US_1,
  VIBRATE_1,
  US_2,
  VIBRATE_2,
};

Adafruit_MCP23008 mcp;
long threshold_us1;
long threshold_us2;
state_type state;
long curr_dist;

void pin_set(char pattern) {
  for(char i = 0; i < 8; i++) {
    bool val = (pattern & (1 << i)) >> i;

    if (val == 0) {
      mcp.digitalWrite(i, LOW);
    } else {
      mcp.digitalWrite(i, HIGH);      
    }

  }
  
}
long read_distance(int trigger_pin, int echo_pin) {
  mcp.digitalWrite(trigger_pin, LOW);
  delayMicroseconds(2);
  mcp.digitalWrite(trigger_pin, HIGH);
  delayMicroseconds(10);
  mcp.digitalWrite(trigger_pin, LOW);
  long duration = pulseIn(echo_pin, HIGH);
  return duration/58.2;
}

void navband_vibrate_left(void) {
  Serial.print("*****NAV-BAND LEFT");
  for (int i = 4; i <= 32; i=i << 1) {
    pin_set(i);
    delay(200);
  }
  pin_set(0);
}

void navband_vibrate_right(void) {
  Serial.println("*****NAV-BAND RIGHT");
  for (int i = 32; i >= 4; i=i >> 1) {
    pin_set(i);
    delay(200);
  }
  pin_set(0);
}

void navband_vibrate(void) {
  mcp.digitalWrite(NAV_MOTOR_1, HIGH);
  mcp.digitalWrite(NAV_MOTOR_2, LOW);
  mcp.digitalWrite(NAV_MOTOR_3, LOW);
  mcp.digitalWrite(NAV_MOTOR_4, LOW);
  delay(200);

  mcp.digitalWrite(NAV_MOTOR_1, LOW);
  mcp.digitalWrite(NAV_MOTOR_2, HIGH);
  mcp.digitalWrite(NAV_MOTOR_3, LOW);
  mcp.digitalWrite(NAV_MOTOR_4, LOW);
  delay(200);
  
  mcp.digitalWrite(NAV_MOTOR_1, LOW);
  mcp.digitalWrite(NAV_MOTOR_2, LOW);
  mcp.digitalWrite(NAV_MOTOR_3, HIGH);
  mcp.digitalWrite(NAV_MOTOR_4, LOW);
  delay(200);
  
  mcp.digitalWrite(NAV_MOTOR_1, LOW);
  mcp.digitalWrite(NAV_MOTOR_2, LOW);
  mcp.digitalWrite(NAV_MOTOR_3, LOW);
  mcp.digitalWrite(NAV_MOTOR_4, HIGH);
  delay(200);
}

void setup() {
  Serial.begin(9600); // Open serial monitor at 115200 baud to see ping results.
  state = INIT;  // By right this should be CALIBRATION

  // Initialize ultrasonic sensors
  threshold_us1 = 100.0; // This is in centimeters!
  threshold_us2 = 100.0; // This is in centimeters!
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(ECHO_PIN_2, INPUT);

  // Initialize Adafruit MCP23008 I/O expander
  mcp.begin();
  for (int i = 0; i < 8; i++) {
    mcp.pinMode(i, OUTPUT);  
  }
  RFduinoBLE.advertisementData = "CouDow";
  RFduinoBLE.deviceName = "ChillerNavBand";
  RFduinoBLE.begin();
}

void loop() {
  
  switch(state) {

    case INIT:
      Serial.println("INIT");
      state = US_1;
      break;

    case US_1:
      delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
      curr_dist = read_distance(TRIGGER_PIN_1, ECHO_PIN_1);
      Serial.print("US_1: ");
      Serial.print(curr_dist);
      Serial.println("cm");
      if ((curr_dist > threshold_us1) || (curr_dist <= 1.0)) {
        state = US_2;
      } else {
        Serial.println("OBSTACLE DETECTED 1");
        state = VIBRATE_1;
      }
      break;

    case VIBRATE_1:
      Serial.println("VIBRATE 1");
      mcp.digitalWrite(CANE_MOTOR_1, HIGH);
      delay(500);
      mcp.digitalWrite(CANE_MOTOR_1, LOW);
      state = US_2;
      break;

    case US_2:
      delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
      curr_dist = read_distance(TRIGGER_PIN_2, ECHO_PIN_2);
      Serial.print("US_2: ");
      Serial.print(curr_dist);
      Serial.println("cm");
      if ((curr_dist > threshold_us2) || (curr_dist <= 1.0)) {
        state = INIT;
      } else {
        Serial.println("OBSTACLE DETECTED 2");
        state = VIBRATE_2;
      }
      break;

    case VIBRATE_2:
      Serial.println("VIBRATE 2");
      mcp.digitalWrite(CANE_MOTOR_2, HIGH);
      delay(500);
      mcp.digitalWrite(CANE_MOTOR_2, LOW);
      state = INIT;
      break;
      
    default:
      Serial.print("DEFAULT");
      break;
  }
}

void RFduinoBLE_onReceive(char *data, int len) {
  if (data[0] == 0){  
    navband_vibrate_left();
  } else if (data[0] == 1) {
    navband_vibrate_right();
  }
}

