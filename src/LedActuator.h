/**
 * @file      LedActuator.h
 * @author    Ian Macavilca Quispe
 * @brief     Status and alert LED actuator for ReefKeeper Pro.
 * @date      2026-07-15
 * @copyright BlueTide Technologies
 */

#pragma once

#include "DeviceDomain.h"

/**
 * @brief Drives the blue status LED and the yellow alert LED.
 */
class LedActuator : public Actuator, public CommandHandler {
private:
  const uint8_t bluePin;
  const uint8_t yellowPin;
  LedState blueState;
  LedState yellowState;
  bool blueOutput;
  bool yellowOutput;
  unsigned long lastToggleMs;

  static constexpr unsigned long BLINK_INTERVAL_MS = 500UL;

  void applyOutputs() {
    digitalWrite(bluePin, blueOutput ? HIGH : LOW);
    digitalWrite(yellowPin, yellowOutput ? HIGH : LOW);
  }

  void resolveState(LedState state, bool &output) {
    switch (state) {
      case LedState::On:
        output = true;
        break;
      case LedState::Off:
        output = false;
        break;
      case LedState::Blinking:
      default:
        // Blinking state is handled by the update method.
        break;
    }
  }

public:
  LedActuator(uint8_t blueLedPin, uint8_t yellowLedPin)
    : bluePin(blueLedPin),
      yellowPin(yellowLedPin),
      blueState(LedState::Off),
      yellowState(LedState::Off),
      blueOutput(false),
      yellowOutput(false),
      lastToggleMs(0UL) {}

  void begin() override {
    pinMode(bluePin, OUTPUT);
    pinMode(yellowPin, OUTPUT);
    digitalWrite(bluePin, LOW);
    digitalWrite(yellowPin, LOW);
  }

  void handle(const Command &command) override {
    if (command.kind() != CommandKind::Led) {
      return;
    }

    const LedCommand &ledCommand = static_cast<const LedCommand &>(command);
    if (ledCommand.ledId == LedId::StatusBlue) {
      blueState = ledCommand.state;
      resolveState(blueState, blueOutput);
    } else {
      yellowState = ledCommand.state;
      resolveState(yellowState, yellowOutput);
    }

    applyOutputs();
  }

  /**
   * @brief Toggles blinking LEDs. Must be called from the main loop.
   */
  void update() {
    const unsigned long now = millis();
    if (now - lastToggleMs >= BLINK_INTERVAL_MS) {
      lastToggleMs = now;

      if (blueState == LedState::Blinking) {
        blueOutput = !blueOutput;
      }
      if (yellowState == LedState::Blinking) {
        yellowOutput = !yellowOutput;
      }

      applyOutputs();
    }
  }
};

