/**
 * @file      ReefKeeperDevice.h
 * @author    Ian Macavilca Quispe
 * @brief     Device orchestration for ReefKeeper Pro.
 * @date      2026-07-15
 * @copyright BlueTide Technologies
 *
 * The coordinator receives sensor events, applies the aquarium business rules,
 * dispatches commands to actuators, and publishes telemetry records securely.
 */

#pragma once

#include "TemperatureSensor.h"
#include "WaterLevelSensor.h"
#include "ButtonPanelSensor.h"
#include "RealTimeClockSensor.h"
#include "LcdActuator.h"
#include "RelayActuator.h"
#include "LedActuator.h"
#include "ServoActuator.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <cstring>

/**
 * @brief Central controller for the ReefKeeper Pro aquarium maintenance device.
 */
class ReefKeeperDevice : public Device {
private:
  // Business rule constants
  static constexpr float MIN_TARGET_TEMPERATURE_C = 22.0f;
  static constexpr float MAX_TARGET_TEMPERATURE_C = 28.0f;
  static constexpr float TARGET_TEMPERATURE_STEP_C = 0.5f;
  static constexpr float TEMPERATURE_TOLERANCE_C = 1.0f;
  static constexpr float WATER_LEVEL_ALERT_DISTANCE_CM = 25.0f;
  static constexpr unsigned long TELEMETRY_INTERVAL_MS = 5000UL;
  static constexpr unsigned long TELEMETRY_WORKER_PERIOD_MS = 1000UL;
  static constexpr unsigned long FEEDER_OPEN_DURATION_MS = 3000UL;
  static constexpr uint8_t FEEDER_OPEN_ANGLE = 90U;
  static constexpr uint8_t FEEDER_CLOSED_ANGLE = 0U;
  static constexpr uint8_t FEED_HOUR_MORNING = 8U;
  static constexpr uint8_t FEED_HOUR_EVENING = 18U;

  // Network constants
  static constexpr const char *WIFI_SSID = "Wokwi-GUEST";
  static constexpr const char *WIFI_PASSWORD = "";
  static constexpr const char *TELEMETRY_URL = "https://test-api-1.free.beeceptor.com/api/v1/data-records";

  // Sensor and actuator composition
  TemperatureSensor temperatureSensor;
  WaterLevelSensor waterLevelSensor;
  ButtonPanelSensor buttonPanelSensor;
  RealTimeClockSensor clockSensor;
  LcdActuator lcdActuator;
  RelayActuator relayActuator;
  LedActuator ledActuator;
  ServoActuator servoActuator;

  // Runtime state
  float targetTemperatureC;
  float currentTemperatureC;
  bool hasCurrentTemperature;
  float lastWaterDistanceCm;
  bool hasWaterDistance;
  OperationMode operationMode;
  HeaterStatus heaterStatus;
  bool feederRunning;
  unsigned long feederStartMs;
  uint8_t lastFedHour;
  unsigned long lastTelemetryMs;
  volatile bool telemetryPending;
  char telemetryPayload[512];
  portMUX_TYPE telemetryMux = portMUX_INITIALIZER_UNLOCKED;
  uint8_t currentHour;
  uint8_t currentMinute;
  uint8_t currentSecond;

  static void telemetryWorkerTask(void *parameter) {
    auto *device = static_cast<ReefKeeperDevice *>(parameter);

    for (;;) {
      if (device->telemetryPending && WiFi.status() == WL_CONNECTED) {
        char payloadCopy[512];
        bool hasPayload = false;

        portENTER_CRITICAL(&device->telemetryMux);
        if (device->telemetryPending) {
          strncpy(payloadCopy, device->telemetryPayload, sizeof(payloadCopy));
          payloadCopy[sizeof(payloadCopy) - 1] = '\0';
          device->telemetryPending = false;
          hasPayload = true;
        }
        portEXIT_CRITICAL(&device->telemetryMux);

        if (hasPayload) {
          WiFiClientSecure secureClient;
          secureClient.setInsecure();
          secureClient.setTimeout(1000);

          HTTPClient http;
          if (http.begin(secureClient, TELEMETRY_URL)) {
            http.addHeader("Content-Type", "application/json");
            const int statusCode = http.POST(payloadCopy);
            Serial.print("Telemetry POST status: ");
            Serial.println(statusCode);
            if (statusCode > 0) {
              Serial.println(http.getString());
            }
            http.end();
          }
        }
      }

      vTaskDelay(pdMS_TO_TICKS(TELEMETRY_WORKER_PERIOD_MS));
    }
  }

  /**
   * @brief Connects the ESP32 to the configured WiFi network.
   */
  void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to WiFi");
    const unsigned long startMs = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startMs < 15000UL) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("WiFi connected, IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("WiFi connection failed, continuing offline.");
    }
  }

  /**
   * @brief Prints the startup banner with company and developer information.
   */
  void printStartupBanner() {
    Serial.println("===================================");
    Serial.println("ReefKeeper Pro - Smart Aquarium Controller");
    Serial.println("Company: BlueTide");
    Serial.println("Developer: Ian Macavilca Quispe");
    Serial.println("Student Code: U202121325");
    Serial.println("NRC: 6785");
    Serial.println("MAC: " + WiFi.macAddress());
    Serial.println("===================================");
  }

  /**
   * @brief Clamps the target temperature to the allowed range.
   */
  void clampTargetTemperature() {
    if (targetTemperatureC < MIN_TARGET_TEMPERATURE_C) {
      targetTemperatureC = MIN_TARGET_TEMPERATURE_C;
    } else if (targetTemperatureC > MAX_TARGET_TEMPERATURE_C) {
      targetTemperatureC = MAX_TARGET_TEMPERATURE_C;
    }
  }

  /**
   * @brief Evaluates temperature rules and dispatches actuator commands.
   */
  void evaluateHeaterLogic() {
    if (!hasCurrentTemperature) {
      return;
    }

    if (operationMode == OperationMode::WaterLevelAlert) {
      heaterStatus = HeaterStatus::Optimal;
      relayActuator.handle(RelayCommand(false));
      ledActuator.handle(LedCommand(LedId::StatusBlue, LedState::Off));
      ledActuator.handle(LedCommand(LedId::AlertYellow, LedState::Blinking));
      return;
    }

    const float lowThreshold = targetTemperatureC - TEMPERATURE_TOLERANCE_C;
    const float highThreshold = targetTemperatureC + TEMPERATURE_TOLERANCE_C;

    if (currentTemperatureC < lowThreshold) {
      heaterStatus = HeaterStatus::Heating;
      relayActuator.handle(RelayCommand(true));
      ledActuator.handle(LedCommand(LedId::StatusBlue, LedState::Off));
      ledActuator.handle(LedCommand(LedId::AlertYellow, LedState::On));
    } else if (currentTemperatureC > highThreshold) {
      heaterStatus = HeaterStatus::Cooling;
      relayActuator.handle(RelayCommand(false));
      ledActuator.handle(LedCommand(LedId::StatusBlue, LedState::Off));
      ledActuator.handle(LedCommand(LedId::AlertYellow, LedState::Off));
    } else {
      heaterStatus = HeaterStatus::Optimal;
      relayActuator.handle(RelayCommand(false));
      ledActuator.handle(LedCommand(LedId::StatusBlue, LedState::On));
      ledActuator.handle(LedCommand(LedId::AlertYellow, LedState::Off));
    }
  }

  /**
   * @brief Transitions the device into low-water alert mode.
   */
  void enterWaterLevelAlert() {
    if (operationMode == OperationMode::WaterLevelAlert) {
      return;
    }

    operationMode = OperationMode::WaterLevelAlert;
    heaterStatus = HeaterStatus::Optimal;

    // Safety actions: turn heater off, stop any active feeding, blink alert LED.
    relayActuator.handle(RelayCommand(false));
    servoActuator.handle(ServoCommand(FEEDER_CLOSED_ANGLE));
    feederRunning = false;
    ledActuator.handle(LedCommand(LedId::StatusBlue, LedState::Off));
    ledActuator.handle(LedCommand(LedId::AlertYellow, LedState::Blinking));

    Serial.println("Entered WATER LEVEL ALERT mode");
  }

  /**
   * @brief Restores normal operation after the water level recovers.
   */
  void enterNormalMode() {
    if (operationMode == OperationMode::Normal) {
      return;
    }

    operationMode = OperationMode::Normal;
    Serial.println("Restored NORMAL MODE");
    evaluateHeaterLogic();
  }

  /**
   * @brief Dispatches the current state to the LCD actuator.
   */
  void updateDisplay() {
    LcdUpdateCommand command(
      operationMode == OperationMode::WaterLevelAlert,
      currentTemperatureC,
      targetTemperatureC,
      heaterStatus
    );
    lcdActuator.handle(command);
  }

  /**
   * @brief Checks the RTC time and triggers the feeder at 08:00 and 18:00.
   */
  void feedIfScheduled() {
    if (operationMode == OperationMode::WaterLevelAlert) {
      return;
    }

    const bool isFeedHour = (currentHour == FEED_HOUR_MORNING || currentHour == FEED_HOUR_EVENING);
    if (!isFeedHour || currentMinute != 0U || currentSecond != 0U) {
      return;
    }

    if (currentHour == lastFedHour || feederRunning) {
      return;
    }

    feederRunning = true;
    feederStartMs = millis();
    lastFedHour = currentHour;
    servoActuator.handle(ServoCommand(FEEDER_OPEN_ANGLE));
    Serial.println("Feeding started");
  }

  /**
   * @brief Closes the feeder after the configured dispensing duration.
   */
  void updateFeeder() {
    if (!feederRunning) {
      return;
    }

    if (millis() - feederStartMs >= FEEDER_OPEN_DURATION_MS) {
      feederRunning = false;
      servoActuator.handle(ServoCommand(FEEDER_CLOSED_ANGLE));
      Serial.println("Feeding finished");
    }
  }

  /**
   * @brief Maps the internal heater status to the required telemetry value.
   */
  const char *heaterStatusTelemetryString(HeaterStatus status) const {
    switch (status) {
      case HeaterStatus::Heating:
        return "CALENTANDO";
      case HeaterStatus::Cooling:
        return "ENFRIANDO";
      case HeaterStatus::Optimal:
      default:
        return "OPTIMO";
    }
  }

  /**
   * @brief Maps the internal operation mode to the required telemetry value.
   */
  const char *operationModeTelemetryString(OperationMode mode) const {
    switch (mode) {
      case OperationMode::WaterLevelAlert:
        return "NIVEL_AGUA_ALERT";
      case OperationMode::Normal:
      default:
        return "NORMAL_MODE";
    }
  }

  /**
   * @brief Formats the current RTC time as an ISO 8601 UTC string.
   */
  void formatTimestamp(char *buffer, size_t bufferSize) {
    const DateTime now = clockSensor.currentDateTime();
    if (!now.isValid()) {
      snprintf(buffer, bufferSize, "2000-01-01T00:00:00Z");
      return;
    }

    snprintf(buffer, bufferSize, "%04d-%02d-%02dT%02d:%02d:%02dZ",
             static_cast<int>(now.year()),
             static_cast<int>(now.month()),
             static_cast<int>(now.day()),
             static_cast<int>(now.hour()),
             static_cast<int>(now.minute()),
             static_cast<int>(now.second()));
  }

  /**
   * @brief Sends a JSON telemetry record over HTTPS when the interval expires.
   */
  void sendTelemetryIfNeeded() {
    const unsigned long now = millis();
    if (now - lastTelemetryMs < TELEMETRY_INTERVAL_MS) {
      return;
    }
    lastTelemetryMs = now;

    if (!hasCurrentTemperature || !hasWaterDistance) {
      Serial.println("Telemetry skipped: incomplete sensor readings");
      return;
    }

    char timestampBuffer[32];
    formatTimestamp(timestampBuffer, sizeof(timestampBuffer));

    const String macAddress = WiFi.macAddress();

    StaticJsonDocument<384> doc;
    doc["deviceMacAddress"] = macAddress;
    doc["operationMode"] = operationModeTelemetryString(operationMode);
    doc["targetTemperature"] = targetTemperatureC;
    doc["currentTemperature"] = currentTemperatureC;
    doc["nivelAgua"] = lastWaterDistanceCm;
    doc["EstadoCalentado"] = heaterStatusTelemetryString(heaterStatus);
    doc["createdAt"] = timestampBuffer;

    char payloadBuffer[512];
    const size_t payloadLength = serializeJson(doc, payloadBuffer, sizeof(payloadBuffer));
    payloadBuffer[payloadLength] = '\0';

    portENTER_CRITICAL(&telemetryMux);
    strncpy(telemetryPayload, payloadBuffer, sizeof(telemetryPayload));
    telemetryPayload[sizeof(telemetryPayload) - 1] = '\0';
    telemetryPending = true;
    portEXIT_CRITICAL(&telemetryMux);

    Serial.println(payloadBuffer);
  }

public:
  /**
   * @brief Constructs the device with all hardware pin assignments.
   */
  ReefKeeperDevice()
    : temperatureSensor(23, this),
      waterLevelSensor(5, 4, this),
      buttonPanelSensor(18, 19, this),
      clockSensor(this),
      lcdActuator(0x27),
      relayActuator(16),
      ledActuator(14, 12),
      servoActuator(17),
      targetTemperatureC(25.0f),
      currentTemperatureC(0.0f),
      hasCurrentTemperature(false),
      lastWaterDistanceCm(0.0f),
      hasWaterDistance(false),
      operationMode(OperationMode::Normal),
      heaterStatus(HeaterStatus::Optimal),
      feederRunning(false),
      feederStartMs(0UL),
      lastFedHour(255U),
      lastTelemetryMs(0UL),
      telemetryPending(false),
      currentHour(0U),
      currentMinute(0U),
      currentSecond(0U) {}

  void begin() override {
    // Initialize the serial console at 115200 baud for debug output.
    Serial.begin(115200);
    // Give the simulator a brief moment to stabilize before starting I/O.
    delay(1200);

    // Put the ESP32 in WiFi station mode so it can connect to a network.
    WiFi.mode(WIFI_STA);
    // Print the startup banner with company and developer identification.
    printStartupBanner();
    // Connect to the configured WiFi access point.
    connectWiFi();

    // Start the background task that will send telemetry over POST.
    temperatureSensor.begin();
    waterLevelSensor.begin();
    buttonPanelSensor.begin();
    clockSensor.begin();
    lcdActuator.begin();
    relayActuator.begin();
    ledActuator.begin();
    servoActuator.begin();

    xTaskCreatePinnedToCore(
      telemetryWorkerTask,
      "telemetryWorker",
      6144,
      this,
      1,
      nullptr,
      1
    );

    // Initial visual state
    updateDisplay();
    evaluateHeaterLogic();
  }

  void loop() override {
    // Read the front-panel buttons first so user input is prioritized.
    buttonPanelSensor.sample();
    // Read the current water temperature from the DHT22 sensor.
    temperatureSensor.sample();
    // Read the water level using the HC-SR04 sensor.
    waterLevelSensor.sample();
    // Update the RTC-derived time values.
    clockSensor.sample();

    // Update the feeder state if it is currently dispensing food.
    updateFeeder();
    // Apply the current heating/cooling rules.
    evaluateHeaterLogic();
    // Refresh the LCD with the latest state.
    updateDisplay();

    // Update blinking LEDs if needed.
    ledActuator.update();
    // Build and queue the telemetry payload when the interval expires.
    sendTelemetryIfNeeded();

    // Short pause to keep the loop responsive and reduce CPU load.
    delay(10);
  }

  void onEvent(const Event &event) override {
    switch (event.kind()) {
      case EventKind::TemperatureRead: {
        const TemperatureReadEvent &temperatureEvent = static_cast<const TemperatureReadEvent &>(event);
        currentTemperatureC = temperatureEvent.celsius;
        hasCurrentTemperature = true;
        break;
      }

      case EventKind::WaterLevelRead: {
        const WaterLevelReadEvent &waterEvent = static_cast<const WaterLevelReadEvent &>(event);
        lastWaterDistanceCm = waterEvent.distanceCm;
        hasWaterDistance = true;

        if (lastWaterDistanceCm > WATER_LEVEL_ALERT_DISTANCE_CM) {
          enterWaterLevelAlert();
        } else if (lastWaterDistanceCm < WATER_LEVEL_ALERT_DISTANCE_CM) {
          enterNormalMode();
        }
        break;
      }

      case EventKind::ButtonPressed: {
        const ButtonPressedEvent &buttonEvent = static_cast<const ButtonPressedEvent &>(event);
        if (buttonEvent.buttonId == ButtonId::IncreaseTargetTemperature) {
          targetTemperatureC += TARGET_TEMPERATURE_STEP_C;
        } else {
          targetTemperatureC -= TARGET_TEMPERATURE_STEP_C;
        }
        clampTargetTemperature();
        evaluateHeaterLogic();
        updateDisplay();
        Serial.print("Target temperature set to: ");
        Serial.println(targetTemperatureC, 1);
        break;
      }

      case EventKind::ClockTick: {
        const ClockTickEvent &clockEvent = static_cast<const ClockTickEvent &>(event);
        currentHour = clockEvent.hour;
        currentMinute = clockEvent.minute;
        currentSecond = clockEvent.second;
        feedIfScheduled();
        break;
      }

      default:
        break;
    }
  }

  void handle(const Command &command) override {
    // The coordinator dispatches commands directly to actuators; external
    // command routing through this method is not used in this design.
    (void)command;
  }
};
