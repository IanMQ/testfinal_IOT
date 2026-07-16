/**
 * @file      LcdActuator.h
 * @author    Ian Macavilca Quispe
 * @brief     LCD 1602 I2C actuator for ReefKeeper Pro.
 * @date      2026-07-15
 * @copyright BlueTide Technologies
 */

#pragma once

#include "DeviceDomain.h"
#include <LiquidCrystal_I2C.h>

/**
 * @brief Renders the operational screen or the low-water alert screen.
 */
class LcdActuator : public Actuator, public CommandHandler {
private:
  LiquidCrystal_I2C lcd;
  bool lastAlertMode;

  const char *heaterStatusLabel(HeaterStatus status) const {
    switch (status) {
      case HeaterStatus::Heating:
        return "HEATING";
      case HeaterStatus::Cooling:
        return "COOLING";
      case HeaterStatus::Optimal:
      default:
        return "OPTIMAL";
    }
  }

  void renderNormal(const LcdUpdateCommand &command) {
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(command.currentTemperature, 1);
    lcd.print("C");

    lcd.setCursor(11, 0);
    lcd.print(heaterStatusLabel(command.heaterStatus));

    lcd.setCursor(0, 1);
    lcd.print("Target:");
    lcd.print(command.targetTemperature, 1);
    lcd.print("C");
  }

  void renderAlert() {
    lcd.setCursor(0, 0);
    lcd.print("WATER LEVEL ALERT");
    lcd.setCursor(0, 1);
    lcd.print("Check aquarium!  ");
  }

public:
  explicit LcdActuator(uint8_t i2cAddress = 0x27)
    : lcd(i2cAddress, 16, 2), lastAlertMode(false) {}

  void begin() override {
    lcd.init();
    lcd.backlight();
    lcd.clear();
  }

  void handle(const Command &command) override {
    if (command.kind() != CommandKind::LcdUpdate) {
      return;
    }

    const LcdUpdateCommand &lcdCommand = static_cast<const LcdUpdateCommand &>(command);

    if (lcdCommand.alertMode != lastAlertMode) {
      lcd.clear();
      lastAlertMode = lcdCommand.alertMode;
    }

    if (lcdCommand.alertMode) {
      renderAlert();
    } else {
      renderNormal(lcdCommand);
    }
  }
};

