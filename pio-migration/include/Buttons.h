#pragma once

#include "Settings.h"
#include <stdint.h>

void configureButtonPins();
void readAllFromButtons(uint8_t *buttonValues, const Settings &currentSettings);
uint16_t computeButtonStateMask(const uint8_t *buttonValues);
void buildButtonReport(const uint8_t *buttonValues, const Settings &currentSettings, uint8_t *report);

