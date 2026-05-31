#pragma once
#include <stdint.h>

#if defined(TARGET_RP2040)

// GP23 is the usual WS2812B data pin on the YD-RP2040 (VCC-GND).
// Some board revisions require closing the RGB/R58/R68 solder bridge before the LED is connected.
// Override via build flag if needed.
#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN 12
#endif

// Keep the external WS2812 enabled by default, but allow test environments to disable it.
#ifndef STATUS_LED_USE_NEOPIXEL
#define STATUS_LED_USE_NEOPIXEL 1
#endif

// Raspberry Pi Pico boards expose the onboard LED as LED_BUILTIN on GPIO25.
// Leave enabled by default so the board LED mirrors the status state.
#ifndef STATUS_LED_USE_BUILTIN
#define STATUS_LED_USE_BUILTIN 1
#endif

void ledBegin();

// Change the active NeoPixel data pin at runtime for diagnostic sweeps.
void ledSetPin(uint8_t pin);

// Set LED to a solid colour (0-255 per channel).
void ledSolid(uint8_t r, uint8_t g, uint8_t b);

// Breathe blue for durationMs, then return.  Replaces a plain delay() in calibration.
void ledPulseBlue(uint32_t durationMs);

#else // ---- non-RP2040 stubs ------------------------------------------------

#include <Arduino.h>
inline void ledBegin() {}
inline void ledSetPin(uint8_t) {}
inline void ledSolid(uint8_t, uint8_t, uint8_t) {}
inline void ledPulseBlue(uint32_t ms) { delay(ms); }

#endif
