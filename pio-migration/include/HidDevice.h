#pragma once

#include <stdint.h>

void setupSpaceMouseHid();
void sendHidReport(uint8_t reportId, const uint8_t *data, uint8_t len);

