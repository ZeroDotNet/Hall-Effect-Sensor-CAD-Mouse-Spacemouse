#pragma once

#include "Settings.h"
#include <stdint.h>

struct MotionReport
{
  int16_t transX;
  int16_t transY;
  int16_t transZ;
  int16_t rotX;
  int16_t rotY;
  int16_t rotZ;
};

MotionReport computeMotion(const int *centered, const Settings &currentSettings);

