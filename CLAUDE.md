# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

DIY 6-DOF (6 Degrees of Freedom) CAD SpaceMouse firmware using Hall Effect sensors. It emulates a 3DConnexion SpaceMouse Pro Wireless over USB HID, compatible with Fusion 360 and OnShape.

## Build System

**PlatformIO** (primary, in `pio-migration/`):

```bash
cd pio-migration
pio run -e arduino-pro-micro    # ATmega32U4 (primary target)
pio run -e rp2040               # Raspberry Pi Pico
pio run -e esp32s3              # ESP32-S3 DevKitC-1
pio run -t upload -e arduino-pro-micro  # Build and flash
pio device monitor              # Serial monitor at 250000 baud
```

**Arduino IDE** (legacy, in `HE_Spacemouse/`): Requires the custom 3DConnexion board definition to be installed separately. The PlatformIO version is preferred.

## Architecture

### Codebase Layout

- `HE_Spacemouse/HE_Spacemouse.ino` — Original Arduino IDE firmware
- `pio-migration/src/main.cpp` — Refactored multi-platform firmware (primary)
- `pio-migration/platformio.ini` — Board targets and build flags
- `STL Files/`, `STEP file/`, `Fusion 360 design file/` — 3D-printable mechanical parts

### Firmware Architecture (`main.cpp`, 876 lines)

The firmware runs a single-loop Arduino sketch with these stages:

1. **Sensor reading** — 8 Hall Effect sensors (SS49e) on analog pins. ADC values are centered against idle baseline values stored at startup (`initRaw[]`) and applied deadzone filtering (`DEADZONE = 40`).

2. **6-DOF calculation** — 8 sensors arranged in 4 pairs. Sensor math maps pairs to translation axes (X, Y, Z) and rotation axes (RX, RY, RZ).

3. **Button handling** — 3 physical buttons with 20ms debounce for detecting simultaneous presses (pseudo-buttons). Center button cycles through 3 view modes.

4. **USB HID report** — Sends 3DConnexion-compatible HID descriptor reports. AVR uses NicoHood's HID library; RP2040 and ESP32-S3 use their native USB stacks.

### Multi-Platform Conditional Compilation

The firmware uses `#ifdef` blocks to handle platform differences:

```cpp
#if defined(__AVR_ATmega32U4__)
  // NicoHood HID library, specific ADC pin mapping
#elif defined(ARDUINO_ARCH_RP2040)
  // Native USB, note: RP2040 only has 4 ADCs
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
  // Native USB, ESP32 ADC pin mapping
#endif
```

### Key Configuration Variables

All live near the top of `main.cpp`:

| Variable | Purpose |
|---|---|
| `DEADZONE` | Minimum sensor movement to register (default: 40) |
| `movement3DC` | Axis mapping mode — `true` for 3DC default, `false` for Teaching Tech |
| `invertX/Y/Z/RX/RY/RZ` | Per-axis inversion booleans |
| `debug` (0–6) | Verbosity level for Serial output |
| `debug1SameLine` | Overprint debug output on same line |

## Development Notes

- The RP2040 target has a hardware limitation: only 4 ADC pins, so all 8 sensors cannot be read simultaneously without a multiplexer or other workaround.
- Serial debug output is at 250000 baud. Set `debug` to 1–6 to enable progressively more verbose output.
- `initRaw[]` is captured at boot to center sensor readings — the device must be at rest with no force applied when powered on.
- No automated tests exist; validation is done by flashing and testing the physical device.
