/**
 * @file      DeviceDomain.h
 * @author    Ian Macavilca Quispe
 * @brief     Domain value objects, events and commands for ReefKeeper Pro.
 * @date      2026-07-15
 * @copyright BlueTide Technologies
 */

#pragma once

#include "ModestIoT.h"

/**
 * @brief Identifiers for the two physical buttons on the front panel.
 */
enum class ButtonId : uint8_t {
  IncreaseTargetTemperature,
  DecreaseTargetTemperature
};

/**
 * @brief Identifiers for the two indicator LEDs.
 */
enum class LedId : uint8_t {
  StatusBlue,
  AlertYellow
};

/**
 * @brief Discrete states that the heater logic can report.
 */
enum class HeaterStatus : uint8_t {
  Heating,
  Optimal,
  Cooling
};

/**
 * @brief High-level operation modes of the aquarium controller.
 */
enum class OperationMode : uint8_t {
  Normal,
  WaterLevelAlert
};

/**
 * @brief LED output states handled by the indicator actuator.
 */
enum class LedState : uint8_t {
  Off,
  On,
  Blinking
};

/**
 * @brief Domain event emitted when a new temperature sample is available.
 */
class TemperatureReadEvent : public Event {
public:
  const float celsius;
  const unsigned long timestampMs;

  TemperatureReadEvent(float celsiusValue, unsigned long timestamp)
    : celsius(celsiusValue), timestampMs(timestamp) {}

  EventKind kind() const override {
    return EventKind::TemperatureRead;
  }
};

/**
 * @brief Domain event emitted when a new water level sample is available.
 */
class WaterLevelReadEvent : public Event {
public:
  const float distanceCm;
  const unsigned long timestampMs;

  WaterLevelReadEvent(float distanceValue, unsigned long timestamp)
    : distanceCm(distanceValue), timestampMs(timestamp) {}

  EventKind kind() const override {
    return EventKind::WaterLevelRead;
  }
};

/**
 * @brief Domain event emitted when a front panel button is pressed.
 */
class ButtonPressedEvent : public Event {
public:
  const ButtonId buttonId;
  const unsigned long timestampMs;

  ButtonPressedEvent(ButtonId id, unsigned long timestamp)
    : buttonId(id), timestampMs(timestamp) {}

  EventKind kind() const override {
    return EventKind::ButtonPressed;
  }
};

/**
 * @brief Domain event emitted when the RTC clock advances to a new second.
 */
class ClockTickEvent : public Event {
public:
  const uint8_t hour;
  const uint8_t minute;
  const uint8_t second;
  const unsigned long timestampMs;

  ClockTickEvent(uint8_t hourValue, uint8_t minuteValue, uint8_t secondValue, unsigned long timestamp)
    : hour(hourValue), minute(minuteValue), second(secondValue), timestampMs(timestamp) {}

  EventKind kind() const override {
    return EventKind::ClockTick;
  }
};

/**
 * @brief Command that updates the LCD screen with current operational data.
 */
class LcdUpdateCommand : public Command {
public:
  const bool alertMode;
  const float currentTemperature;
  const float targetTemperature;
  const HeaterStatus heaterStatus;

  LcdUpdateCommand(bool alert, float currentTemp, float targetTemp, HeaterStatus status)
    : alertMode(alert), currentTemperature(currentTemp), targetTemperature(targetTemp), heaterStatus(status) {}

  CommandKind kind() const override {
    return CommandKind::LcdUpdate;
  }
};

/**
 * @brief Command that turns the heater relay on or off.
 */
class RelayCommand : public Command {
public:
  const bool active;

  explicit RelayCommand(bool relayActive) : active(relayActive) {}

  CommandKind kind() const override {
    return CommandKind::Relay;
  }
};

/**
 * @brief Command that sets the state of one of the indicator LEDs.
 */
class LedCommand : public Command {
public:
  const LedId ledId;
  const LedState state;

  LedCommand(LedId ledIdentifier, LedState ledState)
    : ledId(ledIdentifier), state(ledState) {}

  CommandKind kind() const override {
    return CommandKind::Led;
  }
};

/**
 * @brief Command that moves the feeder servo to a specific angle.
 */
class ServoCommand : public Command {
public:
  const uint8_t angleDegrees;

  explicit ServoCommand(uint8_t angle) : angleDegrees(angle) {}

  CommandKind kind() const override {
    return CommandKind::Servo;
  }
};

