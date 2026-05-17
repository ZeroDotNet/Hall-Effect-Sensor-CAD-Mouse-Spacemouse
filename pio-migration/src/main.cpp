#include <Arduino.h>

#include "Buttons.h"
#include "Config.h"
#include "DebugOutput.h"
#include "HidDevice.h"
#include "Motion.h"
#include "Sensors.h"
#include "Settings.h"

namespace
{
uint16_t keyChange = 0;

uint8_t lowByte16(int16_t value)
{
  return static_cast<uint8_t>(value & 0xFF);
}

uint8_t highByte16(int16_t value)
{
  return static_cast<uint8_t>((value >> 8) & 0xFF);
}

void sendSpaceMouseReport(const MotionReport &motion, const uint8_t *buttonValues)
{
  const int16_t reportY = settings.movement3DC ? motion.transY : motion.transZ;
  const int16_t reportZ = settings.movement3DC ? motion.transZ : motion.transY;
  const uint8_t trans[6] = {
      lowByte16(motion.transX),
      highByte16(motion.transX),
      lowByte16(reportY),
      highByte16(reportY),
      lowByte16(reportZ),
      highByte16(reportZ)};
  sendHidReport(1, trans, sizeof(trans));

  const uint8_t rot[6] = {
      lowByte16(motion.rotX),
      highByte16(motion.rotX),
      lowByte16(motion.rotY),
      highByte16(motion.rotY),
      lowByte16(motion.rotZ),
      highByte16(motion.rotZ)};
  sendHidReport(2, rot, sizeof(rot));

  const uint16_t newKeyChange = computeButtonStateMask(buttonValues);
  if (newKeyChange != keyChange)
  {
    uint8_t btn[HID_BUTTON_REPORT_BYTES];
    buildButtonReport(buttonValues, settings, btn);
    if (SPACEMOUSE_DEBUG == 6)
    {
      Serial.print("btn[0] = ");
      Serial.print(btn[0]);
      Serial.print(" btn[1] = ");
      Serial.print(btn[1]);
      Serial.print(" btn[2] = ");
      Serial.print(btn[2]);
      Serial.print(" btn[3] = ");
      Serial.println(btn[3]);
    }
    sendHidReport(3, btn, sizeof(btn));
    keyChange = newKeyChange;
    if (SPACEMOUSE_DEBUG == 6)
    {
      Serial.print("keyChange = ");
      Serial.println(keyChange);
    }
  }
}
}

void setup()
{
  setupSpaceMouseHid();
  Serial.begin(250000);
  delay(100);

  configureButtonPins();
  configureAnalogReference(SPACEMOUSE_DEBUG);

  const bool loadedSettings = loadStoredSettings(settings, centerPoints);
  if (!loadedSettings)
  {
    settings = defaultSettings();
    captureCenter(centerPoints, settings);
    saveStoredSettings(settings, centerPoints);
  }
}

void loop()
{
  int rawReads[NUM_SENSORS] = {};
  int centered[NUM_SENSORS] = {};
  uint8_t buttonReads[NUM_BUTTON_VALUES] = {};

  readAllFromSensors(rawReads, settings);
  readAllFromButtons(buttonReads, settings);

  if (SPACEMOUSE_DEBUG == 1)
  {
    debugPrintSensors(rawReads, SPACEMOUSE_DEBUG_SAME_LINE != 0, SPACEMOUSE_DEBUG_SAME_LINE == 0, SPACEMOUSE_DEBUG_SAME_LINE != 0);
  }

  centerAndFilterSensors(rawReads, centerPoints, centered, settings);

  if (SPACEMOUSE_DEBUG == 2)
  {
    debugPrintSensors(centered, false, true, false);
  }
  if (SPACEMOUSE_DEBUG == 3)
  {
    debugPrintSensorsAndButtons(centered, buttonReads);
  }

  const MotionReport motion = computeMotion(centered, settings);

  if (SPACEMOUSE_DEBUG == 4)
  {
    debugPrintMovement(motion);
  }
  if (SPACEMOUSE_DEBUG == 5)
  {
    debugPrintSensorsAndMovement(centered, motion);
  }

  sendSpaceMouseReport(motion, buttonReads);
}
