#if defined(TARGET_RP2040)

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "status_led.h"

static Adafruit_NeoPixel _pixel(1, STATUS_LED_PIN, NEO_GRB + NEO_KHZ800);

// Solid blue brightness for the "running" state.  Dim enough not to be distracting.
static constexpr uint8_t BLUE_STEADY = 40;

// Peak brightness during the calibration breath cycle.
static constexpr uint8_t BLUE_PEAK = 180;

// Breath period in ms.
static constexpr uint32_t BREATH_PERIOD_MS = 1200;

void ledBegin()
{
    _pixel.begin();
    _pixel.clear();
    _pixel.show();
}

void ledSolid(uint8_t r, uint8_t g, uint8_t b)
{
    _pixel.setPixelColor(0, _pixel.Color(r, g, b));
    _pixel.show();
}

void ledPulseBlue(uint32_t durationMs)
{
    uint32_t start = millis();
    while (millis() - start < durationMs)
    {
        uint32_t phase = (millis() - start) % BREATH_PERIOD_MS;
        // Triangle wave: 0 → BLUE_PEAK → 0 over one period.
        uint8_t level = (phase < BREATH_PERIOD_MS / 2)
                            ? (uint8_t)(phase * BLUE_PEAK / (BREATH_PERIOD_MS / 2))
                            : (uint8_t)((BREATH_PERIOD_MS - phase) * BLUE_PEAK / (BREATH_PERIOD_MS / 2));
        _pixel.setPixelColor(0, _pixel.Color(0, 0, level));
        _pixel.show();
        delay(16); // ~60 fps update rate
    }
}

#endif // TARGET_RP2040
