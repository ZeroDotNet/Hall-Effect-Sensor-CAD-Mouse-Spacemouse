# Optimization & Improvement Plan — pio-migration

## Context

The `pio-migration/` project is the sole going-forward firmware for the Hall Effect Sensor SpaceMouse. The original `HE_Spacemouse/HE_Spacemouse.ino` is legacy and will not be kept in sync. The firmware has grown organically to 876 lines in a single `main.cpp` with three real bugs, repeated/dead code, and all platform concerns tangled together. The goal is to fix bugs, improve code quality, split into clean modules, and add EEPROM persistence for settings — in that order so the device remains flashable and testable at every step.

---

## Phase 1 — Bug Fixes

**Files:** `pio-migration/src/main.cpp`, `pio-migration/platformio.ini`

### Bug 1: `keyChange` uint8_t overflow
- Line 537: `uint8_t keyChange = 0;` → change to `uint16_t keyChange = 0;`
- The bit-pack expression on lines 578 and 592 sums up to `256 * buttonValues[8]` = 256 max for that term alone, overflowing a `uint8_t`. This corrupts change-detection, causing redundant HID button reports every loop when `buttonValues[8]` is set (third cycle-view pseudo-button active).

### Bug 2: `configureAnalogReference()` called after first sensor read
- In `setup()`, move `configureAnalogReference()` to before the first `readAllFromSensors(centerPoints)` call.
- Current order: read → delay(1000) → configureAnalogReference → read × 2. The first read uses the wrong 5V reference, but the bigger issue is clarity — the intent is to calibrate with the correct reference from the start.

### Bug 3: RP2040 silently maps 8 sensor pins to 4 ADC pins
- In `platformio.ini` `[env:rp2040]`, `HES_PIN_6..9` are mapped to the same GPIO 26–29 as `HES_PIN_0..3`. This produces duplicate/wrong sensor data silently.
- Add a compile-time `#warning "RP2040: only 4 ADC pins available — HES6-9 mirror HES0-3"` guarded by `#ifdef TARGET_RP2040`.
- Add build flag `-D RP2040_FOUR_ADC_ONLY` in `platformio.ini` [env:rp2040], and in `main.cpp` use that flag to initialise `hallSensorEnabled[4..7] = false` for RP2040 builds, disabling the phantom duplicate sensors.

---

## Phase 2 — Code Quality

**Files:** `pio-migration/src/main.cpp`

### 2.1 Remove dead commented-out code
- Lines 491–512: old button suppression code (block comment)
- Lines 503–512: old single-button logical switch code (block comment)
- Lines 764–769: removed speed calculation block comment
- Lines 92–93: `speed` variable comment

### 2.2 Extract repeated debug sensor print helper
- The 8-sensor `Serial.print` block appears at debug levels 1, 2, 3, and 5 (4 near-identical copies).
- Extract to: `void debugPrintSensors(const int* values, bool sameLine)`
- Takes the array and the `debug1SameLine` flag; prints `HES0:val,HES1:val,...HES9:val` then `\r` or `\n`.

### 2.3 Extract repeated debug movement print helper
- The 6-axis movement print appears at debug levels 4 and 5.
- Extract to: `void debugPrintMovement(int16_t tx, int16_t ty, int16_t tz, int16_t rx, int16_t ry, int16_t rz)`

### 2.4 De-duplicate `keyChange` bit-pack expression
- Lines 578 and 592 contain the identical 9-term sum. Extract to:
  `uint16_t computeKeyChange(const uint8_t* bv)` (inline or static function).

### 2.5 Style fixes
- Replace all 6 `if (invX == true)` patterns with `if (invX)`.
- Change inversion lines `transX = transX * -1` to `transX = -transX`.

### 2.6 Const-correct globals
- `int PINLIST[8]` → `const int PINLIST[8]`
- `int BTNLIST[3]` → `const int BTNLIST[3]`
- `int DEADZONE = 40` → `const int DEADZONE = 40` (becomes a config variable in Phase 4)

### 2.7 Named constants for magic numbers
- Add `constexpr uint8_t NUM_SENSORS = 8;`
- Add `constexpr uint8_t NUM_BTNS = 3;`
- Add `constexpr uint8_t NUM_BTN_VALUES = 9;`
- Replace hardcoded `8`, `3`, `9` array sizes throughout.

---

## Phase 3 — Module Extraction (Heavy Split)

**New files created; `main.cpp` becomes a thin orchestrator (~80–100 lines).**

### File layout after Phase 3

```
pio-migration/
  include/
    config.h            ← all user-tuneable settings (replaces top-of-file globals)
    hid_platform.h      ← platform-agnostic HID interface (setupHID, sendHIDReport)
    sensors.h           ← readAllFromSensors, centerAndFilter, captureCenter
    buttons.h           ← readAllFromButtons, state machine
    movement.h          ← computeMovement (6-DOF math)
    debug_print.h       ← debugPrintSensors, debugPrintMovement
  src/
    hid_avr.cpp         ← AVR HID implementation
    hid_rp2040.cpp      ← RP2040 HID implementation
    hid_esp32.cpp       ← ESP32-S3 HID implementation
    sensors.cpp
    buttons.cpp
    movement.cpp
    debug_print.cpp
    main.cpp            ← setup() + loop() only
```

### 3.1 `config.h`
Contains all user-tuneable settings as `constexpr` or `#define`:
- `DEBUG_LEVEL`, `DEBUG_SAME_LINE`
- `MOVEMENT_3DC`
- `CYCLE_BUTTON`, `CYCLE_INITIAL_BUTTON`
- `BUTTON_DELAY_MS`
- `INV_X/Y/Z/RX/RY/RZ`
- `DEADZONE`
- `hallSensorEnabled[NUM_SENSORS]`
- Pin assignments (default fallbacks, overridden by platformio.ini build flags)

### 3.2 `hid_platform.h` + per-platform `.cpp` files
Declares two functions with no `#ifdef` at the call sites:
```cpp
void setupHID();
void sendHIDReport(uint8_t reportId, const uint8_t* data, uint8_t len);
```
Each platform `.cpp` file contains the HID descriptor, class definition, and implementation — guarded by a single top-level `#ifdef`. `platformio.ini` uses `src_filter` per environment to compile only the matching platform file.

### 3.3 `sensors.h` / `sensors.cpp`
- `void readAllFromSensors(int* rawReads)`
- `void centerAndFilter(const int* rawReads, const int* centerPoints, int* centered, int deadzone)`
- `void captureCenter(int* centerPoints)` — wraps the setup calibration sequence

### 3.4 `buttons.h` / `buttons.cpp`
- Full state machine (keyState 0–6) moved here verbatim after Phase 2 cleanup
- `void readAllFromButtons(uint8_t* buttonValues)`
- State variables (`keyState`, `keyPressed`, `keyTimeOld`) become file-scope statics

### 3.5 `movement.h` / `movement.cpp`
- `void computeMovement(const int* centered, int16_t& tx, int16_t& ty, int16_t& tz, int16_t& rx, int16_t& ry, int16_t& rz)`
- Applies inversion flags (read from `config.h`)

### 3.6 `debug_print.h` / `debug_print.cpp`
- `debugPrintSensors`, `debugPrintMovement` extracted in Phase 2 moved here

### 3.7 `main.cpp` after refactor
```
setup():  setupHID() → Serial.begin → pin modes → configureAnalogReference → captureCenter
loop():   readAllFromSensors → readAllFromButtons → centerAndFilter → computeMovement →
          debug prints → send_command
```

### 3.8 `platformio.ini` updates
- Add `src_filter` per environment to include only the correct `hid_*.cpp`.
- Add `lib_deps = NicoHood/HID` explicitly for the AVR environment.

---

## Phase 4 — EEPROM Persistence

**New files:** `include/persistent_config.h`, `src/persistent_config.cpp`

### Data persisted
```cpp
struct StoredConfig {
  uint8_t  version;          // magic version byte — if mismatch, reset to defaults
  int      centerPoints[8];  // calibration baseline
  bool     invX, invY, invZ, invRX, invRY, invRZ;
  bool     movement3DC;
  bool     cycleButton;
  int      deadzone;
};
```

### API
```cpp
bool loadConfig(StoredConfig& cfg);       // returns false if EEPROM uninitialised/stale
void saveConfig(const StoredConfig& cfg);
void resetToDefaults(StoredConfig& cfg);  // populates from config.h defaults
```

### Platform abstraction
| Platform | Library |
|---|---|
| AVR | `<EEPROM.h>` (Arduino AVR core) |
| RP2040 | `<EEPROM.h>` (Arduino-Pico core, flash-backed) |
| ESP32-S3 | `<Preferences.h>` (NVS-backed) |

Wrap platform differences behind `eepromRead` / `eepromWrite` helpers inside `persistent_config.cpp`, guarded by `#ifdef`.

### Boot flow change in `setup()`
1. `loadConfig(cfg)` — if returns false (first boot / version mismatch): call `resetToDefaults`, `captureCenter`, `saveConfig`.
2. If returns true: apply stored `centerPoints`, inversion flags, deadzone — skip motionless startup requirement for calibration.

---

## Verification

Each phase ends with a full build on all three targets:
```bash
cd pio-migration
pio run -e arduino-pro-micro
pio run -e rp2040
pio run -e esp32s3
```

Functional validation per phase (requires hardware):
- **Phase 1**: Flash AVR build; confirm no spurious button HID events when cycling through all three views. Confirm sensor readings are stable at rest.
- **Phase 2**: Flash AVR; all debug levels 1–6 still produce correct serial output.
- **Phase 3**: Flash all three targets; confirm HID device is recognised by 3DConnexion software and all 6 axes + buttons respond correctly.
- **Phase 4**: Flash AVR; power cycle — confirm settings survive. Change a preference (e.g. `invX`); save; power cycle; confirm it persists.
