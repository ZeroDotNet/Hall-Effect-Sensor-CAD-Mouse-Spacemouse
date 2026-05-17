#include "Buttons.h"

#include <string.h>

#if !defined(UNIT_TEST)
#include <Arduino.h>
#else
#define INPUT_PULLUP 2
namespace
{
struct TestSerialType
{
  template <typename T>
  void print(T) {}

  template <typename T>
  void println(T) {}
};

TestSerialType Serial;

void pinMode(int, int) {}
int digitalRead(int) { return 1; }
unsigned long millis() { return 0; }
}
#endif

namespace
{
unsigned long keyTimeOld = 0;
uint8_t keyState = 0;
uint8_t keyPressed = 0;
uint8_t cycleInitialButton = 0;
}

void configureButtonPins()
{
  for (uint8_t i = 0; i < NUM_BUTTONS; i++)
  {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }
}

void readAllFromButtons(uint8_t *buttonValues, const Settings &currentSettings)
{
  memset(buttonValues, 0, NUM_BUTTON_VALUES);

  for (uint8_t i = 1; i < 4; i++)
  {
    buttonValues[i] = !digitalRead(BUTTON_PINS[i - 1]);
  }

  const unsigned long keyTimeNew = millis();
  switch (keyState)
  {
  case 0:
    if (buttonValues[1] || buttonValues[2] || buttonValues[3])
    {
      if (SPACEMOUSE_DEBUG == 6)
      {
        Serial.println("keyState 0 - button pressed move to keyState 1");
      }
      keyState = 1;
      keyTimeOld = keyTimeNew;
      buttonValues[1] = buttonValues[2] = buttonValues[3] = false;
    }
    break;

  case 1:
    if (SPACEMOUSE_DEBUG == 6)
    {
      Serial.println("keyState 1 - one button pressed");
    }
    if (keyTimeNew - keyTimeOld > SPACEMOUSE_BUTTON_DELAY_MS)
    {
      keyState = 3;
    }
    else if (buttonValues[1] && buttonValues[3])
    {
      keyState = 2;
    }
    else if (buttonValues[1] && buttonValues[2])
    {
      keyState = 5;
    }
    else if (buttonValues[2] && buttonValues[3])
    {
      keyState = 6;
    }

    buttonValues[1] = buttonValues[2] = buttonValues[3] = false;
    break;

  case 2:
    if (SPACEMOUSE_DEBUG == 6)
    {
      Serial.println("keyState 2 - second button pressed - set logical button");
    }
    buttonValues[0] = true;
    keyState = 4;
    keyPressed = 0;
    buttonValues[1] = buttonValues[3] = false;
    break;

  case 3:
    if (SPACEMOUSE_DEBUG == 6)
    {
      Serial.println("keyState 3 - second button not pressed in time");
    }
    keyState = 4;
    if (buttonValues[1])
    {
      keyPressed = 1;
    }
    else if (buttonValues[2])
    {
      if (currentSettings.cycleButton)
      {
        buttonValues[6 + cycleInitialButton] = true;
        keyPressed = 6 + cycleInitialButton;
        cycleInitialButton = (cycleInitialButton + 1) % 3;
        if (SPACEMOUSE_DEBUG == 6)
        {
          Serial.print("cycleInitialButton = ");
          Serial.println(keyPressed);
        }
      }
      else
      {
        keyPressed = 2;
      }
    }
    else
    {
      keyPressed = 3;
    }
    break;

  case 4:
    if (!buttonValues[1] && !buttonValues[2] && !buttonValues[3])
    {
      keyState = 0;
    }
    buttonValues[0] = buttonValues[1] = buttonValues[2] = buttonValues[3] = false;
    buttonValues[keyPressed] = true;
    break;

  case 5:
    if (SPACEMOUSE_DEBUG == 6)
    {
      Serial.println("keyState 5 - second button pressed - set logical button");
    }
    buttonValues[4] = true;
    keyState = 4;
    keyPressed = 4;
    buttonValues[1] = buttonValues[2] = buttonValues[3] = false;
    break;

  case 6:
    if (SPACEMOUSE_DEBUG == 6)
    {
      Serial.println("keyState 6 - second button pressed - set logical button");
    }
    buttonValues[5] = true;
    keyState = 4;
    keyPressed = 5;
    buttonValues[1] = buttonValues[2] = buttonValues[3] = false;
    break;
  }
}

uint16_t computeButtonStateMask(const uint8_t *buttonValues)
{
  uint16_t mask = 0;
  for (uint8_t i = 0; i < NUM_BUTTON_VALUES; i++)
  {
    if (buttonValues[i])
    {
      mask |= static_cast<uint16_t>(1U << i);
    }
  }
  return mask;
}

void buildButtonReport(const uint8_t *buttonValues, const Settings &currentSettings, uint8_t *report)
{
  report[0] = static_cast<uint8_t>(32 * buttonValues[3] + 16 * buttonValues[2] + 4 * buttonValues[1] + buttonValues[0] + 2 * buttonValues[5]);
  report[1] = 0;
  report[2] = 0;
  report[3] = static_cast<uint8_t>(4 * buttonValues[4]);

  if (currentSettings.cycleButton)
  {
    report[0] = static_cast<uint8_t>(32 * buttonValues[6] + 16 * buttonValues[7] + 4 * buttonValues[8] + buttonValues[0] + 2 * buttonValues[1]);
    report[1] = static_cast<uint8_t>(buttonValues[4] + 16 * buttonValues[5]);
    report[3] = static_cast<uint8_t>(4 * buttonValues[3]);
  }
}
