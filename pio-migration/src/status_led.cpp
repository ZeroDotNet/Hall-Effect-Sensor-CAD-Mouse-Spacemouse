#if defined(TARGET_RP2040)

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "status_led.h"

#if STATUS_LED_USE_ONBOARD_NEOPIXEL
static Adafruit_NeoPixel onboardPixel(1, STATUS_LED_ONBOARD_PIN, NEO_GRB + NEO_KHZ800);
#endif

#if STATUS_LED_USE_RING_NEOPIXEL
static Adafruit_NeoPixel ringPixel(STATUS_LED_RING_COUNT, STATUS_LED_RING_PIN, NEO_GRB + NEO_KHZ800);
#endif

static constexpr uint32_t LED_STEP_MS = 250;
static constexpr uint8_t STATUS_BLUE = 32;
static constexpr uint8_t TEST_COLORS[][3] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 255, 255},
    {0, 0, 0},
};

void ledSolid(uint8_t r, uint8_t g, uint8_t b)
{
#if ENABLE_LEDS && STATUS_LED_USE_ONBOARD_NEOPIXEL
  onboardPixel.setPixelColor(0, onboardPixel.Color(r, g, b));
  onboardPixel.show();
#endif

#if ENABLE_LEDS && STATUS_LED_USE_RING_NEOPIXEL
  for (uint16_t i = 0; i < STATUS_LED_RING_COUNT; i++)
  {
    ringPixel.setPixelColor(i, ringPixel.Color(r, g, b));
  }
  ringPixel.show();
#endif

#if STATUS_LED_USE_BUILTIN && defined(LED_BUILTIN)
  digitalWrite(LED_BUILTIN, (r || g || b) ? HIGH : LOW);
#endif
}

void ledBegin()
{
#if ENABLE_LEDS && STATUS_LED_USE_ONBOARD_NEOPIXEL
  onboardPixel.begin();
  onboardPixel.clear();
  onboardPixel.show();
#endif

#if ENABLE_LEDS && STATUS_LED_USE_RING_NEOPIXEL
  ringPixel.begin();
  ringPixel.clear();
  ringPixel.show();
#endif

#if STATUS_LED_USE_BUILTIN && defined(LED_BUILTIN)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif

#if ENABLE_LEDS && !STATUS_LED_TEST_ONLY
  ledSolid(0, 0, STATUS_BLUE);
#endif
}

void ledUpdate()
{
#if !ENABLE_LEDS
  return;
#else
  static uint32_t lastStepMs = 0;
  static size_t colorIndex = 0;
  const uint32_t now = millis();

  if (now - lastStepMs < LED_STEP_MS)
  {
    return;
  }

  lastStepMs = now;

#if STATUS_LED_TEST_RGB_PIN_SWEEP && STATUS_LED_USE_ONBOARD_NEOPIXEL
  static const uint8_t candidatePins[] = {12, 16, 17, 18, 19, 20, 21, 22, 23, 24};
  static size_t pinIndex = 0;

  if (colorIndex == 0)
  {
    onboardPixel.clear();
    onboardPixel.show();
    onboardPixel.setPin(candidatePins[pinIndex]);
    onboardPixel.begin();
    onboardPixel.clear();
    onboardPixel.show();

    Serial.print("Testing onboard NeoPixel on GPIO");
    Serial.println(candidatePins[pinIndex]);
    pinIndex = (pinIndex + 1) % (sizeof(candidatePins) / sizeof(candidatePins[0]));
  }
#endif

#if STATUS_LED_TEST_RGB_CYCLE || STATUS_LED_TEST_RGB_PIN_SWEEP
  ledSolid(TEST_COLORS[colorIndex][0], TEST_COLORS[colorIndex][1], TEST_COLORS[colorIndex][2]);
  colorIndex = (colorIndex + 1) % (sizeof(TEST_COLORS) / sizeof(TEST_COLORS[0]));
#else
  ledSolid(0, 0, STATUS_BLUE);
#endif
#endif
}

#endif // TARGET_RP2040
