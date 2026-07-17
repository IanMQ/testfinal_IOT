/**
 * @file      ReefKeeperDeviceWIFISTANDALONE.h
 * @author    Ian Macavilca Quispe
 * @brief     Standalone WiFi telemetry device for ReefKeeper Pro.
 * @date      2026-07-17
 * @copyright BlueTide Technologies
 *
 * This version does not depend on any project-specific sensor or framework
 * headers. It keeps only begin() and loop(), generates synthetic telemetry
 * locally, and posts the JSON payload over HTTPS.
 */

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

/**
 * @brief Fully standalone controller that only connects to WiFi and sends telemetry.
 */
class ReefKeeperDeviceWIFISTANDALONE {
private:
  static constexpr float WATER_LEVEL_ALERT_DISTANCE_CM = 25.0f;
  static constexpr unsigned long TELEMETRY_INTERVAL_MS = 5000UL;
  static constexpr const char *WIFI_SSID = "Wokwi-GUEST";
  static constexpr const char *WIFI_PASSWORD = "";
  static constexpr const char *TELEMETRY_URL = "https://test-api-1.free.beeceptor.com/api/v1/data-records";

  float currentTemperatureC;
  float lastWaterDistanceCm;
  bool lowWaterAlert;
  unsigned long lastTelemetryMs;
  unsigned long lastFakeUpdateMs;

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
    Serial.println("ReefKeeper Pro - WiFi Standalone");
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

  void updateFakeTelemetryValues() {
    const unsigned long now = millis();
    if (now - lastFakeUpdateMs < 1000UL) {
      return;
    }
    lastFakeUpdateMs = now;

    // Hook point:
    // Change these synthetic values if you want to simulate different sensor
    // behavior without adding any other project-specific headers.
    const unsigned long seconds = now / 1000UL;
    currentTemperatureC = 24.0f + static_cast<float>(seconds % 10UL) * 0.2f;
    lastWaterDistanceCm = 18.0f + static_cast<float>(seconds % 6UL) * 1.5f;
    lowWaterAlert = (lastWaterDistanceCm > WATER_LEVEL_ALERT_DISTANCE_CM);
  }

  void formatTimestamp(char *buffer, size_t bufferSize) const {
    const unsigned long totalSeconds = millis() / 1000UL;
    const unsigned long seconds = totalSeconds % 60UL;
    const unsigned long minutes = (totalSeconds / 60UL) % 60UL;
    const unsigned long hours = (totalSeconds / 3600UL) % 24UL;

    snprintf(buffer, bufferSize, "1970-01-01T%02lu:%02lu:%02luZ", hours, minutes, seconds);
  }

  void sendTelemetryIfNeeded() {
    const unsigned long now = millis();
    if (now - lastTelemetryMs < TELEMETRY_INTERVAL_MS) {
      return;
    }
    lastTelemetryMs = now;

    updateFakeTelemetryValues();

    char timestampBuffer[32];
    formatTimestamp(timestampBuffer, sizeof(timestampBuffer));

    // Hook point:
    // If you want to fake, enrich, or replace values before sending telemetry,
    // do it here. These locals are what get serialized into the POST body.
    const float telemetryCurrentTemperature = currentTemperatureC;
    const float telemetryWaterDistanceCm = lastWaterDistanceCm;
    const bool telemetryLowWaterAlert = lowWaterAlert;

    StaticJsonDocument<256> doc;
    doc["deviceMacAddress"] = WiFi.macAddress();
    doc["operationMode"] = operationModeTelemetryString();
    doc["currentTemperature"] = telemetryCurrentTemperature;
    doc["nivelAgua"] = telemetryWaterDistanceCm;
    doc["lowWaterAlert"] = telemetryLowWaterAlert;
    doc["createdAt"] = timestampBuffer;

    // Hook point:
    // Add any extra JSON fields the backend expects right here, before
    // serializeJson() converts the document into the request body.
    // Example:
    // doc["salinity"] = 35.2;
    // doc["ph"] = 8.1;
    // doc["firmwareVersion"] = "wifi-standalone";

    char payloadBuffer[320];
    const size_t payloadLength = serializeJson(doc, payloadBuffer, sizeof(payloadBuffer) - 1);
    payloadBuffer[payloadLength < sizeof(payloadBuffer) ? payloadLength : sizeof(payloadBuffer) - 1] = '\0';

    Serial.println(payloadBuffer);

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Telemetry skipped: WiFi not connected");
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
  ReefKeeperDeviceWIFISTANDALONE()
    : currentTemperatureC(24.0f),
      lastWaterDistanceCm(18.0f),
      lowWaterAlert(false),
      lastTelemetryMs(0UL),
      lastFakeUpdateMs(0UL) {}

  void begin() {
    Serial.begin(115200);
    delay(1200);

    printStartupBanner();
    connectWiFi();
  }

  void loop() {
    sendTelemetryIfNeeded();
    delay(10);
  }
};
