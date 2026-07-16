/**
 * @file      ModestIoT.h
 * @author    Ian Macavilca Quispe
 * @brief     Minimal ModestIoT-inspired framework for the ReefKeeper Pro device.
 * @date      2026-07-15
 * @copyright BlueTide Technologies
 *
 * The framework keeps the sketch focused on Device orchestration while
 * sensors and actuators encapsulate their own hardware behaviour.
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Command identifiers used by the application layer.
 */
enum class CommandKind : uint8_t {
  LcdUpdate,
  Relay,
  Led,
  Servo
};

/**
 * @brief Event identifiers produced by the application sensors.
 */
enum class EventKind : uint8_t {
  TemperatureRead,
  WaterLevelRead,
  ButtonPressed,
  ClockTick
};

/**
 * @brief Base class for all output intent objects.
 */
class Command {
public:
  virtual ~Command() = default;
  virtual CommandKind kind() const = 0;
};

/**
 * @brief Base class for all input fact objects.
 */
class Event {
public:
  virtual ~Event() = default;
  virtual EventKind kind() const = 0;
};

/**
 * @brief Consumer of domain events.
 */
class EventHandler {
public:
  virtual ~EventHandler() = default;
  virtual void onEvent(const Event &event) = 0;
};

/**
 * @brief Executor of domain commands.
 */
class CommandHandler {
public:
  virtual ~CommandHandler() = default;
  virtual void handle(const Command &command) = 0;
};

/**
 * @brief Input boundary that reads physical signals and emits events.
 */
class Sensor {
public:
  virtual ~Sensor() = default;
  virtual void begin() = 0;
  virtual void sample() = 0;
};

/**
 * @brief Output boundary that applies physical changes.
 */
class Actuator {
public:
  virtual ~Actuator() = default;
  virtual void begin() = 0;
};

/**
 * @brief Mediator that coordinates sensors, actuators and business rules.
 */
class Device : public EventHandler, public CommandHandler {
public:
  virtual ~Device() = default;
  virtual void begin() = 0;
  virtual void loop() = 0;
};

