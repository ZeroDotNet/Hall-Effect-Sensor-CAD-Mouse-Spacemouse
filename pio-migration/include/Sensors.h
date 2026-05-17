#pragma once

#include "Settings.h"

void configureAnalogReference(uint8_t debugLevel);
void readAllFromSensors(int *rawReads, const Settings &currentSettings);
void captureCenter(int *targetCenterPoints, const Settings &currentSettings);
void centerAndFilterSensors(const int *rawReads, const int *sourceCenterPoints, int *centered, const Settings &currentSettings);

