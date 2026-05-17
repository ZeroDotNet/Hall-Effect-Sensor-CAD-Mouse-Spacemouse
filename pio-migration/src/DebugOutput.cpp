#include "DebugOutput.h"

#include <Arduino.h>

void debugPrintSensors(const int *values, bool carriageReturnPrefix, bool newline, bool padWhenNoNewline)
{
  if (carriageReturnPrefix)
  {
    Serial.print('\r');
  }

  static const char *labels[NUM_SENSORS] = {
      "HES0:",
      "HES1:",
      "HES2:",
      "HES3:",
      "HES6:",
      "HES7:",
      "HES8:",
      "HES9:"};

  for (uint8_t i = 0; i < NUM_SENSORS; i++)
  {
    Serial.print(labels[i]);
    Serial.print(values[i]);
    if (i < NUM_SENSORS - 1)
    {
      Serial.print(",");
    }
  }

  if (newline)
  {
    Serial.println();
  }
  else if (padWhenNoNewline)
  {
    Serial.print("    ");
  }
}

void debugPrintSensorsAndButtons(const int *values, const uint8_t *buttonValues)
{
  debugPrintSensors(values, false, false, false);
  Serial.print(",");
  Serial.print("But0:");
  Serial.print(buttonValues[0]);
  Serial.print(",");
  Serial.print("But1:");
  Serial.print(buttonValues[1]);
  Serial.print(",");
  Serial.print("But2:");
  Serial.print(buttonValues[2]);
  Serial.print(",");
  Serial.print("But3:");
  Serial.println(buttonValues[3]);
}

void debugPrintMovement(const MotionReport &motion)
{
  Serial.print("TX:");
  Serial.print(motion.transX);
  Serial.print(",");
  Serial.print("TY:");
  Serial.print(motion.transY);
  Serial.print(",");
  Serial.print("TZ:");
  Serial.print(motion.transZ);
  Serial.print(",");
  Serial.print("RX:");
  Serial.print(motion.rotX);
  Serial.print(",");
  Serial.print("RY:");
  Serial.print(motion.rotY);
  Serial.print(",");
  Serial.print("RZ:");
  Serial.println(motion.rotZ);
}

void debugPrintSensorsAndMovement(const int *values, const MotionReport &motion)
{
  debugPrintSensors(values, false, false, false);
  Serial.print("||");
  debugPrintMovement(motion);
}
