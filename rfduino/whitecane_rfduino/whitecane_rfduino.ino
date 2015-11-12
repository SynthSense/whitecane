// Project: Whitecane
// Organization: Synthsense
// Authors: Adhitya Murali, Tomas Vega

#include <Wire.h>
#include "Adafruit_MCP23008.h"

#define TRIGGER_PIN_1  1  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN_1     2  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define TRIGGER_PIN_2  3
#define ECHO_PIN_2     4

// These should be using the MCP23008 port expander
#define MOTOR_1        4
#define MOTOR_2        5
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

long read_distance(int trigger_pin, int echo_pin) {
  digitalWrite(trigger_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger_pin, LOW);
  long duration = pulseIn(echo_pin, HIGH);
  return duration/58.2;
}

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  state = INIT;  // By right this should be CALIBRATION

  // Initialize ultrasonic sensors
  threshold_us1 = 100.0; // This is in centimeters!
  threshold_us2 = 100.0; // This is in centimeters!
  pinMode(TRIGGER_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
}

void loop() {
  switch(state) {

    case INIT:
      state = US_1;
      break;

    case US_1:
      delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
      curr_dist = read_distance(TRIGGER_PIN_1, ECHO_PIN_1);
      Serial.print("US_1: ");
      Serial.print(curr_dist);
      Serial.println("cm");
      if ((curr_dist > threshold_us1) || (curr_dist <= 1.0)) {
        state = INIT;
      } else {
        Serial.println("OBSTACLE DETECTED 1");
        state = INIT;
      }
      break;

    case VIBRATE_1:
      digitalWrite(MOTOR_1, HIGH);
      delay(500);
      digitalWrite(MOTOR_1, LOW);
      state = INIT;
      break;
      
    default:
      Serial.print("DEFAULT");
      break;
  }
}
