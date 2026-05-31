#if defined(TARGET_RP2040)

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "status_led.h"

#if STATUS_LED_USE_NEOPIXEL
static Adafruit_NeoPixel _pixel(1, STATUS_LED_PIN, NEO_GRB + NEO_KHZ800);
#endif

#if STATUS_LED_USE_BUILTIN && defined(LED_BUILTIN)
static constexpr bool HAS_BUILTIN_LED = true;
#else
static constexpr bool HAS_BUILTIN_LED = false;
#endif

// Solid blue brightness for the "running" state.  Dim enough not to be distracting.
static constexpr uint8_t BLUE_STEADY = 40;

// Peak brightness during the calibration breath cycle.
static constexpr uint8_t BLUE_PEAK = 180;

// Breath period in ms.
static constexpr uint32_t BREATH_PERIOD_MS = 1200;

void ledBegin()
{
#if STATUS_LED_USE_NEOPIXEL
    _pixel.begin();

    _pixel.clear();
    _pixel.show();
#endif

    if (HAS_BUILTIN_LED)
    {
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, LOW);
    }
}

void ledSetPin(uint8_t pin)
{
#if STATUS_LED_USE_NEOPIXEL
    _pixel.clear();
    _pixel.show();

    _pixel.setPin(pin);
    _pixel.begin();
    _pixel.clear();
    _pixel.show();
#else
    (void)pin;
#endif
}

int ledGetPin()
{
#if STATUS_LED_USE_NEOPIXEL
    return _pixel.getPin();
#endif
    return -1;
}

void ledSolid(uint8_t r, uint8_t g, uint8_t b)
{
#if STATUS_LED_USE_NEOPIXEL
    _pixel.setPixelColor(0, _pixel.Color(r, g, b));
    _pixel.show();
#endif

    if (HAS_BUILTIN_LED)
    {
        digitalWrite(LED_BUILTIN, (r || g || b) ? HIGH : LOW);
    }
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
#if STATUS_LED_USE_NEOPIXEL
        _pixel.setPixelColor(0, _pixel.Color(0, 0, level));
        _pixel.show();
#endif
        if (HAS_BUILTIN_LED)
        {
            digitalWrite(LED_BUILTIN, level > 0 ? HIGH : LOW);
        }
        delay(16); // ~60 fps update rate
    }

    if (HAS_BUILTIN_LED)
    {
        digitalWrite(LED_BUILTIN, LOW);
    }
}

#endif // TARGET_RP2040
