/**
 * @file      ButtonPanelSensor.h
 * @author    Ian Macavilca Quispe
 * @brief     Front panel button sensor with debouncing for ReefKeeper Pro.
 * @date      2026-07-15
 * @copyright BlueTide Technologies
 */

#pragma once

#include "DeviceDomain.h"

/**
 * @brief Reads the (+) and (-) tactile switches and emits button press events.
 *
 * The buttons are wired between the GPIO pin and ground, using the internal
 * pull-up resistor. A press is therefore detected on a LOW transition.
 */
class ButtonPanelSensor : public Sensor {
private:
  EventHandler *eventHandler;
  const uint8_t increasePin;
  const uint8_t decreasePin;
  const unsigned long debounceMs;
  unsigned long lastIncreaseMs;
  unsigned long lastDecreaseMs;
  bool increaseLatched;
  bool decreaseLatched;

  bool isDebounced(unsigned long lastPressMs, unsigned long now) const {
    return (now - lastPressMs) >= debounceMs;
  }

  void emitIfPressed(bool currentState, bool &latched, unsigned long &lastPressMs, ButtonId id, unsigned long now) {
    if (currentState == LOW && !latched && isDebounced(lastPressMs, now)) {
      lastPressMs = now;
      latched = true;
      ButtonPressedEvent event(id, now);
      if (eventHandler != nullptr) {
        eventHandler->onEvent(event);
      }
    }
    if (currentState == HIGH) {
      latched = false;
    }
  }

public:
  ButtonPanelSensor(uint8_t increaseButtonPin, uint8_t decreaseButtonPin, EventHandler *handler, unsigned long debounceMsValue = 250UL)
    : eventHandler(handler),
      increasePin(increaseButtonPin),
      decreasePin(decreaseButtonPin),
      debounceMs(debounceMsValue),
      lastIncreaseMs(0UL),
      lastDecreaseMs(0UL),
      increaseLatched(false),
      decreaseLatched(false) {}

  void begin() override {
    pinMode(increasePin, INPUT_PULLUP);
    pinMode(decreasePin, INPUT_PULLUP);
  }

  void sample() override {
    const unsigned long now = millis();

    const bool increaseState = digitalRead(increasePin);
    const bool decreaseState = digitalRead(decreasePin);

    emitIfPressed(increaseState, increaseLatched, lastIncreaseMs, ButtonId::IncreaseTargetTemperature, now);
    emitIfPressed(decreaseState, decreaseLatched, lastDecreaseMs, ButtonId::DecreaseTargetTemperature, now);
  }
};
