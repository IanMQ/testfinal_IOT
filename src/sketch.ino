/**
 * @file      sketch.ino
 * @author    Ian Macavilca Quispe
 * @brief     Entry point for the ReefKeeper Pro Wokwi project.
 * @date      2026-07-15
 * @copyright BlueTide Technologies
 *
 * The sketch instantiates the ReefKeeper Pro device statically and delegates
 * setup() and loop() to the ModestIoT Device coordinator.
 */

#include "ReefKeeperDeviceWIFnLCD.h"

static ReefKeeperDeviceWIFnLCD reefKeeperDevice;

void setup() {
  reefKeeperDevice.begin();
}

void loop() {
  reefKeeperDevice.loop();
}
