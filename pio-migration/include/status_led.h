#pragma once
#include <stdint.h>

#if defined(TARGET_RP2040)

#ifndef ENABLE_LEDS
#define ENABLE_LEDS 0
#endif
#ifndef STATUS_LED_USE_BUILTIN
#define STATUS_LED_USE_BUILTIN 0
#endif
#ifndef STATUS_LED_USE_ONBOARD_NEOPIXEL
#define STATUS_LED_USE_ONBOARD_NEOPIXEL 0
#endif
#ifndef STATUS_LED_ONBOARD_PIN
#define STATUS_LED_ONBOARD_PIN 24
#endif
#ifndef STATUS_LED_USE_RING_NEOPIXEL
#define STATUS_LED_USE_RING_NEOPIXEL 0
#endif
#ifndef STATUS_LED_RING_PIN
#define STATUS_LED_RING_PIN 16
#endif
#ifndef STATUS_LED_RING_COUNT
#define STATUS_LED_RING_COUNT 12
#endif
#ifndef STATUS_LED_TEST_ONLY
#define STATUS_LED_TEST_ONLY 0
#endif
#ifndef STATUS_LED_TEST_RGB_CYCLE
#define STATUS_LED_TEST_RGB_CYCLE 0
#endif
#ifndef STATUS_LED_TEST_RGB_PIN_SWEEP
#define STATUS_LED_TEST_RGB_PIN_SWEEP 0
#endif

void ledBegin();
void ledUpdate();
void ledSolid(uint8_t r, uint8_t g, uint8_t b);

#else // ---- non-RP2040 stubs ------------------------------------------------

inline void ledBegin() {}
inline void ledUpdate() {}
inline void ledSolid(uint8_t, uint8_t, uint8_t) {}

#endif
