#pragma once

#include <stdint.h>

#if !defined(UNIT_TEST)
#include <Arduino.h>
#else
#ifndef A0
#define A0 0
#endif
#ifndef A1
#define A1 1
#endif
#ifndef A2
#define A2 2
#endif
#ifndef A3
#define A3 3
#endif
#ifndef A6
#define A6 6
#endif
#ifndef A7
#define A7 7
#endif
#ifndef A8
#define A8 8
#endif
#ifndef A9
#define A9 9
#endif
#endif

constexpr uint8_t NUM_SENSORS = 8;
constexpr uint8_t NUM_BUTTONS = 3;
constexpr uint8_t NUM_BUTTON_VALUES = 9;
constexpr uint8_t HID_BUTTON_REPORT_BYTES = 4;

#ifndef SPACEMOUSE_DEBUG
#define SPACEMOUSE_DEBUG 0
#endif

#ifndef SPACEMOUSE_DEBUG_SAME_LINE
#define SPACEMOUSE_DEBUG_SAME_LINE 1
#endif

#ifndef SPACEMOUSE_MOVEMENT_3DC
#define SPACEMOUSE_MOVEMENT_3DC 1
#endif

#ifndef SPACEMOUSE_CYCLE_BUTTON
#define SPACEMOUSE_CYCLE_BUTTON 1
#endif

#ifndef SPACEMOUSE_BUTTON_DELAY_MS
#define SPACEMOUSE_BUTTON_DELAY_MS 20
#endif

#ifndef SPACEMOUSE_INV_X
#define SPACEMOUSE_INV_X 0
#endif
#ifndef SPACEMOUSE_INV_Y
#define SPACEMOUSE_INV_Y 0
#endif
#ifndef SPACEMOUSE_INV_Z
#define SPACEMOUSE_INV_Z 1
#endif
#ifndef SPACEMOUSE_INV_RX
#define SPACEMOUSE_INV_RX 0
#endif
#ifndef SPACEMOUSE_INV_RY
#define SPACEMOUSE_INV_RY 0
#endif
#ifndef SPACEMOUSE_INV_RZ
#define SPACEMOUSE_INV_RZ 0
#endif

#ifndef SPACEMOUSE_DEADZONE
#define SPACEMOUSE_DEADZONE 40
#endif

#ifndef SPACEMOUSE_ENABLED_HALL_MASK
#define SPACEMOUSE_ENABLED_HALL_MASK 0x0F
#endif

#ifndef HES_PIN_0
#define HES_PIN_0 A0
#endif
#ifndef HES_PIN_1
#define HES_PIN_1 A1
#endif
#ifndef HES_PIN_2
#define HES_PIN_2 A2
#endif
#ifndef HES_PIN_3
#define HES_PIN_3 A3
#endif
#ifndef HES_PIN_6
#define HES_PIN_6 A6
#endif
#ifndef HES_PIN_7
#define HES_PIN_7 A7
#endif
#ifndef HES_PIN_8
#define HES_PIN_8 A8
#endif
#ifndef HES_PIN_9
#define HES_PIN_9 A9
#endif

#ifndef BTN_PIN_0
#define BTN_PIN_0 0
#endif
#ifndef BTN_PIN_1
#define BTN_PIN_1 1
#endif
#ifndef BTN_PIN_2
#define BTN_PIN_2 2
#endif

constexpr uint8_t HES0 = 0;
constexpr uint8_t HES1 = 1;
constexpr uint8_t HES2 = 2;
constexpr uint8_t HES3 = 3;
constexpr uint8_t HES6 = 4;
constexpr uint8_t HES7 = 5;
constexpr uint8_t HES8 = 6;
constexpr uint8_t HES9 = 7;

static const int SENSOR_PINS[NUM_SENSORS] = {
    HES_PIN_0,
    HES_PIN_1,
    HES_PIN_2,
    HES_PIN_3,
    HES_PIN_6,
    HES_PIN_7,
    HES_PIN_8,
    HES_PIN_9,
};

static const int BUTTON_PINS[NUM_BUTTONS] = {
    BTN_PIN_0,
    BTN_PIN_1,
    BTN_PIN_2,
};
