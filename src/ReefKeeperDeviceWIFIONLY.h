/**
 * @file      ReefKeeperDeviceWIFIONLY.h
 * @author    Ian Macavilca Quispe
 * @brief     WiFi-only device orchestration for telemetry publishing.
 * @date      2026-07-17
 * @copyright BlueTide Technologies
 *
 * This version keeps only the pieces required to read sensors and publish
 * telemetry over WiFi. All actuator and control logic from the full device has
 * been removed.
 */

#pragma once

#include "TemperatureSensor.h"
#include "WaterLevelSensor.h"
#include "RealTimeClockSensor.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

/**
 * @brief Minimal device controller that only samples sensors and sends telemetry.
 */
class ReefKeeperDeviceWIFIONLY : public Device {
private:
  static constexpr float WATER_LEVEL_ALERT_DISTANCE_CM = 25.0f;
  static constexpr unsigned long TELEMETRY_INTERVAL_MS = 5000UL;
  static constexpr const char *WIFI_SSID = "Wokwi-GUEST";
  static constexpr const char *WIFI_PASSWORD = "";
  static constexpr const char *TELEMETRY_URL = "https://test-api-1.free.beeceptor.com/api/v1/data-records";

  TemperatureSensor temperatureSensor;
  WaterLevelSensor waterLevelSensor;
  RealTimeClockSensor clockSensor;

  float currentTemperatureC;
  bool hasCurrentTemperature;
  float lastWaterDistanceCm;
  bool hasWaterDistance;
  bool lowWaterAlert;
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
    Serial.println("ReefKeeper Pro - WiFi Only Telemetry");
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

    StaticJsonDocument<256> doc;
    doc["deviceMacAddress"] = WiFi.macAddress();
    doc["operationMode"] = operationModeTelemetryString();
    doc["currentTemperature"] = currentTemperatureC;
    doc["nivelAgua"] = lastWaterDistanceCm;
    doc["createdAt"] = timestampBuffer;

    char payloadBuffer[320];
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
  ReefKeeperDeviceWIFIONLY()
    : temperatureSensor(23, this),
      waterLevelSensor(5, 4, this),
      clockSensor(this),
      currentTemperatureC(0.0f),
      hasCurrentTemperature(false),
      lastWaterDistanceCm(0.0f),
      hasWaterDistance(false),
      lowWaterAlert(false),
      lastTelemetryMs(0UL) {}

  void begin() override {
    Serial.begin(115200);
    delay(1200);

    printStartupBanner();
    connectWiFi();

    temperatureSensor.begin();
    waterLevelSensor.begin();
    clockSensor.begin();
  }

  void loop() override {
    temperatureSensor.sample();
    waterLevelSensor.sample();
    clockSensor.sample();

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
