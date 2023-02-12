#include <Servo.h>

#include "state.h"
#include "command.h"
#include "buttons.h"

const int HC_06_BAUD_RATE = 9600;

const int LIGHTS_PIN = 3;

const int HORN_PIN = 4;

const int FRONT_WHEEL_SERVO_PIN = 9;
const uint8_t DEFAULT_FRONT_WHEEL_ANGLE = 90;

const int MOTOR_SPEED_PIN = 11;
const int MOTOR_DIRECTION_PIN = 13;

const int RX_TIMEOUT_MILLIS = 500;
const int RX_RECOVERY_TIMEOUT_MILLIS = 50;

Servo frontWheels;

uint64_t lastRxTimestamp = 0;

void halt() {
  digitalWrite(MOTOR_DIRECTION_PIN, LOW);
  analogWrite(MOTOR_SPEED_PIN, 0);
  frontWheels.write(DEFAULT_FRONT_WHEEL_ANGLE);
  digitalWrite(LIGHTS_PIN, LOW);
  digitalWrite(HORN_PIN, LOW);
}

void setup() {
  Serial1.begin(HC_06_BAUD_RATE);

  pinMode(LIGHTS_PIN, OUTPUT);
  pinMode(HORN_PIN, OUTPUT);
  pinMode(MOTOR_SPEED_PIN, OUTPUT);
  pinMode(MOTOR_DIRECTION_PIN, OUTPUT);

  frontWheels.attach(FRONT_WHEEL_SERVO_PIN);

  lastRxTimestamp = millis();
  halt();
}

State onCommand(uint8_t command) {
  switch (command) {
    case DRIVE_FORWARD:
      digitalWrite(MOTOR_DIRECTION_PIN, HIGH);
      return MOTOR_SPEED;
    case DRIVE_BACKWARD:
    case STALL:
      digitalWrite(MOTOR_DIRECTION_PIN, LOW);
      return MOTOR_SPEED;
  }

  return COMMAND;
}

State onMotorSpeed(uint8_t speed) {
  static const size_t HISTORY_SIZE = 4;
  static uint8_t history[HISTORY_SIZE] = {0};
  static size_t cursor = 0;
  static uint8_t lastSpeed = 0;

  history[cursor] = speed;
  cursor = (cursor + 1) % HISTORY_SIZE;

  uint64_t sum = 0;
  for (size_t i = 0; i < HISTORY_SIZE; ++i)
    sum += history[i];

  uint8_t s = static_cast<uint8_t>(sum / HISTORY_SIZE);
  if (s != lastSpeed) {
    analogWrite(MOTOR_SPEED_PIN, s);
    lastSpeed = s;
  }

  return WHEEL_ANGLE;
}

State onWheelAngle(uint8_t angle) {
  static uint8_t lastAngle = 0;

  if (angle != lastAngle) {
    frontWheels.write(180 - angle);
    lastAngle = angle;
  }

  return BUTTONS;
}

State onButtons(uint8_t buttonState) {
  static uint8_t lastState = 0;

  if (buttonState != lastState) {
    digitalWrite(LIGHTS_PIN, buttonState & FRONT_LIGHTS ? HIGH : LOW);
    digitalWrite(HORN_PIN, buttonState & HORN ? HIGH : LOW);
    lastState = buttonState;
  }
  
  return COMMAND;
}

void loop() {
  if (Serial1.available() > 0) {
    lastRxTimestamp = millis();

    State state = COMMAND;
    uint8_t byte = static_cast<uint8_t>(Serial1.read());
    
    switch (state) {
      case COMMAND:
        state = onCommand(byte);
        break;
      case MOTOR_SPEED:
        state = onMotorSpeed(byte);
        break;
      case WHEEL_ANGLE:
        state = onWheelAngle(byte);
        break;
      case BUTTONS:
        state = onButtons(byte);
        break;
    }
  } else {
    uint64_t t = millis();
    if (t - lastRxTimestamp > RX_TIMEOUT_MILLIS) {
      halt();
      delay(RX_RECOVERY_TIMEOUT_MILLIS);
    }
  }
}