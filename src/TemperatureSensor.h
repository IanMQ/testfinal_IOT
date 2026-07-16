/**
 * @file      TemperatureSensor.h
 * @author    Ian Macavilca Quispe
 * @brief     DHT22 temperature sensor adapter for ReefKeeper Pro.
 * @date      2026-07-15
 * @copyright BlueTide Technologies
 */

#pragma once

#include "DeviceDomain.h"
#include <DHT.h>

/**
 * @brief Reads water temperature from a DHT22 and emits a domain event.
 */
class TemperatureSensor : public Sensor {
private:
  EventHandler *eventHandler;
  DHT dhtSensor;
  const unsigned long sampleIntervalMs;
  unsigned long lastSampleMs;

public:
  TemperatureSensor(uint8_t dataPin, EventHandler *handler, unsigned long intervalMs = 2000UL)
    : eventHandler(handler),
      dhtSensor(dataPin, DHT22),
      sampleIntervalMs(intervalMs),
      lastSampleMs(0UL) {}

  void begin() override {
    dhtSensor.begin();
  }

  void sample() override {
    const unsigned long now = millis();
    if (now - lastSampleMs < sampleIntervalMs) {
      return;
    }

    lastSampleMs = now;

    const float temperature = dhtSensor.readTemperature();
    if (isnan(temperature)) {
      return;
    }

    TemperatureReadEvent event(temperature, now);
    if (eventHandler != nullptr) {
      eventHandler->onEvent(event);
    }
  }
};

