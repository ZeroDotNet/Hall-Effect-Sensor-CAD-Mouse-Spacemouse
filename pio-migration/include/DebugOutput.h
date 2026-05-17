#pragma once

#include "Config.h"
#include "Motion.h"
#include <stdint.h>

void debugPrintSensors(const int *values, bool carriageReturnPrefix, bool newline, bool padWhenNoNewline);
void debugPrintSensorsAndButtons(const int *values, const uint8_t *buttonValues);
void debugPrintMovement(const MotionReport &motion);
void debugPrintSensorsAndMovement(const int *values, const MotionReport &motion);
