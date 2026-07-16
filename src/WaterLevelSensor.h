/**
 * @file      WaterLevelSensor.h
 * @author    Ian Macavilca Quispe
 * @brief     HC-SR04 ultrasonic sensor adapter for ReefKeeper Pro.
 * @date      2026-07-15
 * @copyright BlueTide Technologies
 */

#pragma once

#include "DeviceDomain.h"

/**
 * @brief Measures the distance from the sensor to the water surface.
 *
 * A lower distance value means a higher water level. The coordinator decides
 * whether the measured distance crosses the low-water threshold.
 */
class WaterLevelSensor : public Sensor {
private:
  EventHandler *eventHandler;
  const uint8_t triggerPin;
  const uint8_t echoPin;
  const unsigned long sampleIntervalMs;
  unsigned long lastSampleMs;

  float readDistanceCm() {
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    const unsigned long duration = pulseIn(echoPin, HIGH, 30000UL);
    if (duration == 0UL) {
      return -1.0f;
    }

    return static_cast<float>(duration) / 58.0f;
  }

public:
  WaterLevelSensor(uint8_t trigPin, uint8_t echoPinNumber, EventHandler *handler, unsigned long intervalMs = 1000UL)
    : eventHandler(handler),
      triggerPin(trigPin),
      echoPin(echoPinNumber),
      sampleIntervalMs(intervalMs),
      lastSampleMs(0UL) {}

  void begin() override {
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);
    digitalWrite(triggerPin, LOW);
  }

  void sample() override {
    const unsigned long now = millis();
    if (now - lastSampleMs < sampleIntervalMs) {
      return;
    }

    lastSampleMs = now;

    const float distanceCm = readDistanceCm();
    if (distanceCm < 0.0f) {
      return;
    }

    Serial.print("Water level sample: ");
    Serial.println(distanceCm);

    WaterLevelReadEvent event(distanceCm, now);
    if (eventHandler != nullptr) {
      eventHandler->onEvent(event);
    }
  }
};
