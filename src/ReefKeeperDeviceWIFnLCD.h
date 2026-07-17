/**
 * @file      ReefKeeperDeviceWIFnLCD.h
 * @author    Ian Macavilca Quispe
 * @brief     WiFi telemetry device with LCD output for ReefKeeper Pro.
 * @date      2026-07-17
 * @copyright BlueTide Technologies
 *
 * This version is based on WIFIONLY, but also renders the current state on
 * the LCD while keeping the HTTPS telemetry POST.
 */

#pragma once

#include "DeviceDomain.h"
#include "TemperatureSensor.h"
#include "WaterLevelSensor.h"
#include "RealTimeClockSensor.h"
#include "LcdActuator.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

/**
 * @brief Telemetry device that shows data on the LCD and sends POST updates.
 */
class ReefKeeperDeviceWIFnLCD : public Device {
private:
  static constexpr float WATER_LEVEL_ALERT_DISTANCE_CM = 25.0f;
  static constexpr float DEFAULT_TARGET_TEMPERATURE_C = 25.0f;
  static constexpr unsigned long TELEMETRY_INTERVAL_MS = 5000UL;
  static constexpr const char *WIFI_SSID = "Wokwi-GUEST";
  static constexpr const char *WIFI_PASSWORD = "";
  static constexpr const char *TELEMETRY_URL = "https://test-api-1.free.beeceptor.com/api/v1/data-records";

  TemperatureSensor temperatureSensor;
  WaterLevelSensor waterLevelSensor;
  RealTimeClockSensor clockSensor;
  LcdActuator lcdActuator;

  float currentTemperatureC;
  bool hasCurrentTemperature;
  float lastWaterDistanceCm;
  bool hasWaterDistance;
  bool lowWaterAlert;
  float targetTemperatureC;
  HeaterStatus heaterStatus;
  unsigned long lastTelemetryMs;

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

  void printStartupBanner() {
    Serial.println("===================================");
    Serial.println("ReefKeeper Pro - WiFi + LCD Telemetry");
    Serial.println("Company: BlueTide");
    Serial.println("Developer: Ian Macavilca Quispe");
    Serial.println("Student Code: U202121325");
    Serial.println("NRC: 6785");
    Serial.println("MAC: " + WiFi.macAddress());
    Serial.println("===================================");
  }

  const char *operationModeTelemetryString() const {
    return lowWaterAlert ? "NIVEL_AGUA_ALERT" : "NORMAL_MODE";
  }

  const char *heaterStatusTelemetryString() const {
    switch (heaterStatus) {
      case HeaterStatus::Heating:
        return "CALENTANDO";
      case HeaterStatus::Cooling:
        return "ENFRIANDO";
      case HeaterStatus::Optimal:
      default:
        return "OPTIMO";
    }
  }

  void updateHeaterStatus() {
    if (!hasCurrentTemperature) {
      heaterStatus = HeaterStatus::Optimal;
      return;
    }

    const float lowThreshold = targetTemperatureC - 1.0f;
    const float highThreshold = targetTemperatureC + 1.0f;

    if (currentTemperatureC < lowThreshold) {
      heaterStatus = HeaterStatus::Heating;
    } else if (currentTemperatureC > highThreshold) {
      heaterStatus = HeaterStatus::Cooling;
    } else {
      heaterStatus = HeaterStatus::Optimal;
    }
  }

  void updateDisplay() {
    // Hook point:
    // If you want to change what the LCD shows, edit the values passed below.
    // The current screen uses the existing actuator so you can keep the same
    // look while still driving telemetry in parallel.
    LcdUpdateCommand command(
      lowWaterAlert,
      currentTemperatureC,
      targetTemperatureC,
      heaterStatus
    );
    lcdActuator.handle(command);
  }

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

    // Hook point:
    // If you want to fake or enrich values before sending telemetry, do it
    // here. The local variables below are what get serialized into the POST.
    const float telemetryCurrentTemperature = currentTemperatureC;
    const float telemetryWaterDistanceCm = lastWaterDistanceCm;
    const bool telemetryLowWaterAlert = lowWaterAlert;
    const char *telemetryHeaterStatus = heaterStatusTelemetryString();

    StaticJsonDocument<320> doc;
    doc["deviceMacAddress"] = WiFi.macAddress();
    doc["operationMode"] = operationModeTelemetryString();
    doc["currentTemperature"] = telemetryCurrentTemperature;
    doc["targetTemperature"] = targetTemperatureC;
    doc["nivelAgua"] = telemetryWaterDistanceCm;
    doc["lowWaterAlert"] = telemetryLowWaterAlert;
    doc["heaterStatus"] = telemetryHeaterStatus;
    doc["createdAt"] = timestampBuffer;

    // Hook point:
    // Add any extra JSON fields the backend expects right here, before
    // serializeJson(...) converts the document into the request body.
    // Example:
    // doc["salinity"] = 35.2;
    // doc["ph"] = 8.1;
    // doc["screenMode"] = "lcd";

    char payloadBuffer[384];
    const size_t payloadLength = serializeJson(doc, payloadBuffer, sizeof(payloadBuffer) - 1);
    payloadBuffer[payloadLength < sizeof(payloadBuffer) ? payloadLength : sizeof(payloadBuffer) - 1] = '\0';

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Telemetry skipped: WiFi not connected");
      Serial.println(payloadBuffer);
      return;
    }

    WiFiClientSecure secureClient;
    secureClient.setInsecure();
    secureClient.setTimeout(1000);

    // Hook point:
    // This is the exact moment where the HTTP POST is created and sent.
    // If you need to change headers, authentication, content type, endpoint,
    // or attach a different body format, do it below before http.POST(...).
    HTTPClient http;
    if (http.begin(secureClient, TELEMETRY_URL)) {
      http.addHeader("Content-Type", "application/json");
      const int statusCode = http.POST(payloadBuffer);
      Serial.print("Telemetry POST status: ");
      Serial.println(statusCode);
      if (statusCode > 0) {
        Serial.println(http.getString());
      }
      http.end();
    }
  }

public:
  ReefKeeperDeviceWIFnLCD()
    : temperatureSensor(23, this),
      waterLevelSensor(5, 4, this),
      clockSensor(this),
      lcdActuator(0x27),
      currentTemperatureC(0.0f),
      hasCurrentTemperature(false),
      lastWaterDistanceCm(0.0f),
      hasWaterDistance(false),
      lowWaterAlert(false),
      targetTemperatureC(DEFAULT_TARGET_TEMPERATURE_C),
      heaterStatus(HeaterStatus::Optimal),
      lastTelemetryMs(0UL) {}

  void begin() override {
    Serial.begin(115200);
    delay(1200);

    printStartupBanner();
    connectWiFi();

    temperatureSensor.begin();
    waterLevelSensor.begin();
    clockSensor.begin();
    lcdActuator.begin();
  }

  void loop() override {
    temperatureSensor.sample();
    waterLevelSensor.sample();
    clockSensor.sample();

    updateHeaterStatus();
    updateDisplay();
    sendTelemetryIfNeeded();

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
        lowWaterAlert = (lastWaterDistanceCm > WATER_LEVEL_ALERT_DISTANCE_CM);
        break;
      }

      default:
        break;
    }
  }

  void handle(const Command &command) override {
    (void)command;
  }
};
