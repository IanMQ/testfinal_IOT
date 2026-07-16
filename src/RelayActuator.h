/**
 * @file      RelayActuator.h
 * @author    Ian Macavilca Quispe
 * @brief     Heater relay actuator for ReefKeeper Pro.
 * @date      2026-07-15
 * @copyright BlueTide Technologies
 */

#pragma once

#include "DeviceDomain.h"

/**
 * @brief Controls the water heater relay module.
 */
class RelayActuator : public Actuator, public CommandHandler {
private:
  const uint8_t controlPin;
  bool active;

public:
  explicit RelayActuator(uint8_t relayPin)
    : controlPin(relayPin), active(false) {}

  void begin() override {
    pinMode(controlPin, OUTPUT);
    digitalWrite(controlPin, LOW);
  }

  void handle(const Command &command) override {
    if (command.kind() != CommandKind::Relay) {
      return;
    }

    const RelayCommand &relayCommand = static_cast<const RelayCommand &>(command);
    active = relayCommand.active;
    digitalWrite(controlPin, active ? HIGH : LOW);
  }

  bool isActive() const {
    return active;
  }
};

