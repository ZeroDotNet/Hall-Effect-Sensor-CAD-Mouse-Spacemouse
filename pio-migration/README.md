# PlatformIO Migration

This folder contains the maintained PlatformIO firmware for the Hall Effect
Sensor SpaceMouse. The original `HE_Spacemouse/HE_Spacemouse.ino` sketch is now
legacy reference material and is not kept in sync with this code.

The firmware emulates a USB HID 3DConnexion SpaceMouse-compatible device.
Build success only proves that the firmware compiles; USB enumeration, axis
behavior, button behavior, and persistent settings still require hardware
validation.

## Environments

- `arduino-pro-micro`: SparkFun Pro Micro 5V/16MHz, ATmega32U4.
- `rp2040`: Raspberry Pi Pico, RP2040. Compile-verified placeholder; the Pico
  exposes only four ADC-capable GPIOs, so update the pin map or add external
  ADC/mux hardware before treating this as a complete eight-sensor build.
- `esp32s3`: Espressif ESP32-S3 DevKitC-1. Compile-verified native USB target;
  hardware USB enumeration still needs validation.

Build from this folder with:

```sh
pio run -e arduino-pro-micro
pio run -e rp2040
pio run -e esp32s3
```

Run native logic tests with:

```sh
pio test -e native
```

Upload and monitor the baseline target with:

```sh
pio run -e arduino-pro-micro -t upload
pio device monitor -b 250000
```

## Source Layout

- `include/Config.h`: compile-time defaults, sizes, pin fallbacks, and build
  flag overrides.
- `include/Settings.h`, `src/Settings.cpp`: runtime settings plus persisted
  calibration/preferences.
- `include/HidDevice.h`, `src/HidDevice.cpp`: platform-specific USB HID setup
  behind a common interface.
- `include/Sensors.h`, `src/Sensors.cpp`: Hall sensor reads, center capture,
  and deadzone filtering.
- `include/Buttons.h`, `src/Buttons.cpp`: physical button reads, pseudo-button
  state machine, button mask, and HID button report packing.
- `include/Motion.h`, `src/Motion.cpp`: centered sensor values to 6-DOF
  translation/rotation values.
- `include/DebugOutput.h`, `src/DebugOutput.cpp`: serial debug formatting.
- `src/main.cpp`: setup/loop orchestration.

## Configuration

Common defaults can be overridden from `platformio.ini` with build flags:

| Setting | Default | Build flag |
|---|---:|---|
| Debug level | `1` | `-D SPACEMOUSE_DEBUG=<0-6>` |
| Debug level 1 same-line output | `1` | `-D SPACEMOUSE_DEBUG_SAME_LINE=<0/1>` |
| 3DConnexion movement mode | `1` | `-D SPACEMOUSE_MOVEMENT_3DC=<0/1>` |
| Cycle front button through views | `1` | `-D SPACEMOUSE_CYCLE_BUTTON=<0/1>` |
| Button chord delay | `20` ms | `-D SPACEMOUSE_BUTTON_DELAY_MS=<ms>` |
| Deadzone | `40` | `-D SPACEMOUSE_DEADZONE=<value>` |
| Enabled Hall sensor mask | `0x0F` | `-D SPACEMOUSE_ENABLED_HALL_MASK=<mask>` |

Axis inversion defaults can also be overridden with
`SPACEMOUSE_INV_X`, `SPACEMOUSE_INV_Y`, `SPACEMOUSE_INV_Z`,
`SPACEMOUSE_INV_RX`, `SPACEMOUSE_INV_RY`, and `SPACEMOUSE_INV_RZ`.

## Persistent Settings

The firmware stores a versioned record with a magic value and checksum. Blank or
invalid storage falls back to compiled defaults and captures a new center
calibration.

Persisted values:

- `centerPoints[8]`
- `invX`, `invY`, `invZ`, `invRX`, `invRY`, `invRZ`
- `movement3DC`
- `cycleButton`
- `deadzone`

Storage backends:

| Target | Backend | Status |
|---|---|---|
| `arduino-pro-micro` | AVR `EEPROM` | Implemented, compile-verified |
| `esp32s3` | Arduino `Preferences` / NVS | Implemented, compile-verified |
| `rp2040` | none for Arduino Mbed in this checkout | Stubbed; boots with defaults and compile-verifies |

## Pin Map

Default Arduino Pro Micro pins:

| Logical input | Default pin |
|---|---|
| `HES0` | `A0` |
| `HES1` | `A1` |
| `HES2` | `A2` |
| `HES3` | `A3` |
| `HES6` | `A6` |
| `HES7` | `A7` |
| `HES8` | `A8` |
| `HES9` | `A9` |
| `BTN0` | `0` |
| `BTN1` | `1` |
| `BTN2` | `2` |

Board-specific pin overrides live in `platformio.ini` through `HES_PIN_*` and
`BTN_PIN_*` build flags.

## Debug Modes

| Level | Output |
|---:|---|
| `0` | off |
| `1` | raw Hall sensor values |
| `2` | centered Hall sensor values |
| `3` | deadzone-filtered centered values plus buttons |
| `4` | translation and rotation values |
| `5` | filtered sensors plus movement values |
| `6` | pseudo-button state machine diagnostics |

## Hardware Validation Checklist

- Device enumerates as the intended SpaceMouse-compatible HID.
- 3DConnexion software detects the device.
- Debug level 1 reports all enabled Hall sensors.
- Debug level 2/3 confirms centered and deadzone-filtered values.
- Debug level 4/5 confirms expected axis mapping.
- All physical and pseudo buttons generate the intended report actions.
- For AVR/ESP32-S3, persisted calibration/preferences survive a power cycle.
