#include "Sensors.h"

#include <Arduino.h>
#include <stdlib.h>

void configureAnalogReference(uint8_t debugLevel)
{
#if defined(ARDUINO_ARCH_AVR)
  if (debugLevel == 1)
  {
    analogReference(DEFAULT);
  }
  else
  {
    analogReference(INTERNAL);
  }
#else
  (void)debugLevel;
#endif
}

void readAllFromSensors(int *rawReads, const Settings &currentSettings)
{
  for (uint8_t i = 0; i < NUM_SENSORS; i++)
  {
    rawReads[i] = currentSettings.hallSensorEnabled[i] ? analogRead(SENSOR_PINS[i]) : 0;
  }
}

void captureCenter(int *targetCenterPoints, const Settings &currentSettings)
{
  readAllFromSensors(targetCenterPoints, currentSettings);
  delay(1000);
  readAllFromSensors(targetCenterPoints, currentSettings);
  readAllFromSensors(targetCenterPoints, currentSettings);
}

void centerAndFilterSensors(const int *rawReads, const int *sourceCenterPoints, int *centered, const Settings &currentSettings)
{
  for (uint8_t i = 0; i < NUM_SENSORS; i++)
  {
    centered[i] = sourceCenterPoints[i] - rawReads[i];
    if (centered[i] < currentSettings.deadzone && centered[i] > -currentSettings.deadzone)
    {
      centered[i] = 0;
    }
    else
    {
      const int sgn = centered[i] / abs(centered[i]);
      centered[i] = sgn * (abs(centered[i]) - currentSettings.deadzone);
    }
  }
}

