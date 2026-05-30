#pragma once
#include <stdint.h>

#if defined(TARGET_RP2040)

// GP23 is the WS2812B NeoPixel on the YD-RP2040 (VCC-GND). Override via build flag if needed.
#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN 23
#endif

void ledBegin();

// Set LED to a solid colour (0-255 per channel).
void ledSolid(uint8_t r, uint8_t g, uint8_t b);

// Breathe blue for durationMs, then return.  Replaces a plain delay() in calibration.
void ledPulseBlue(uint32_t durationMs);

#else  // ---- non-RP2040 stubs ------------------------------------------------

#include <Arduino.h>
inline void ledBegin() {}
inline void ledSolid(uint8_t, uint8_t, uint8_t) {}
inline void ledPulseBlue(uint32_t ms) { delay(ms); }

#endif
