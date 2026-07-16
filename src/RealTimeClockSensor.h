/**
 * @file      RealTimeClockSensor.h
 * @author    Ian Macavilca Quispe
 * @brief     DS1307 real-time clock adapter for ReefKeeper Pro.
 * @date      2026-07-15
 * @copyright BlueTide Technologies
 */

#pragma once

#include "DeviceDomain.h"
#include <RTClib.h>

/**
 * @brief Samples the DS1307 RTC and emits a ClockTickEvent once per second.
 */
class RealTimeClockSensor : public Sensor {
private:
  EventHandler *eventHandler;
  RTC_DS1307 rtc;
  const unsigned long sampleIntervalMs;
  unsigned long lastSampleMs;
  uint8_t lastSecond;

public:
  RealTimeClockSensor(EventHandler *handler, unsigned long intervalMs = 1000UL)
    : eventHandler(handler),
      sampleIntervalMs(intervalMs),
      lastSampleMs(0UL),
      lastSecond(255U) {}

  void begin() override {
    rtc.begin();
  }

  void sample() override {
    const unsigned long now = millis();
    if (now - lastSampleMs < sampleIntervalMs) {
      return;
    }

    lastSampleMs = now;

    const DateTime nowDateTime = rtc.now();
    if (!nowDateTime.isValid()) {
      return;
    }

    if (nowDateTime.second() == lastSecond) {
      return;
    }

    lastSecond = nowDateTime.second();

    ClockTickEvent event(nowDateTime.hour(), nowDateTime.minute(), nowDateTime.second(), now);
    if (eventHandler != nullptr) {
      eventHandler->onEvent(event);
    }
  }

  /**
   * @brief Returns the most recent valid date/time read from the RTC.
   * @return The current DateTime object.
   */
  DateTime currentDateTime() {
    return rtc.now();
  }
};

