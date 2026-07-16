/**
 * @file      ServoActuator.h
 * @author    Ian Macavilca Quispe
 * @brief     Automatic feeder servo actuator for ReefKeeper Pro.
 * @date      2026-07-15
 * @copyright BlueTide Technologies
 */

#pragma once

#include "DeviceDomain.h"
#include <ESP32Servo.h>

/**
 * @brief Positions the feeder servo according to received commands.
 */
class ServoActuator : public Actuator, public CommandHandler {
private:
  const uint8_t pwmPin;
  Servo servo;
  uint8_t currentAngle;

  void writeAngle(uint8_t angle) {
    servo.write(angle);
    currentAngle = angle;
  }

public:
  explicit ServoActuator(uint8_t servoPin)
    : pwmPin(servoPin), currentAngle(0U) {}

  void begin() override {
    servo.attach(pwmPin, 500, 2400);
    writeAngle(0U);
  }

  void handle(const Command &command) override {
    if (command.kind() != CommandKind::Servo) {
      return;
    }

    const ServoCommand &servoCommand = static_cast<const ServoCommand &>(command);
    writeAngle(servoCommand.angleDegrees);
  }

  uint8_t getAngle() const {
    return currentAngle;
  }
};

