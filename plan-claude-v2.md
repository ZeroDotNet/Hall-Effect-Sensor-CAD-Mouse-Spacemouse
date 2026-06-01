# Optimization & Improvement Plan v2 — pio-migration

## Context

The `pio-migration/` project is the sole going-forward firmware for the Hall Effect Sensor SpaceMouse. `HE_Spacemouse/HE_Spacemouse.ino` is legacy and will not be kept in sync. The firmware has grown organically to 876 lines in a single `main.cpp` with real bugs, repeated/dead code, and all platform concerns tangled together. This plan fuses the specificity of plan-claude.md (exact line numbers, function signatures) with the completeness of plan-codex.md (tests, board-aware config, HID validation, documentation). Each phase leaves the firmware compilable and testable.

---

## Phase 1 — Bug Fixes

**Files:** `pio-migration/src/main.cpp`, `pio-migration/platformio.ini`

### Bug 1: `keyChange` uint8_t overflow
- Line 537: `uint8_t keyChange = 0;` → `uint16_t keyChange = 0;`
- The bit-pack expression on lines 578 and 592 sums to `256 * buttonValues[8]`, which overflows `uint8_t`. When `buttonValues[8]` is set (third cycle-view pseudo-button), the stored value wraps to 0, corrupting change-detection and causing redundant HID button reports on every loop iteration.

### Bug 2: `buttonReads` not initialised before `readAllFromButtons()`
- In `loop()`, declare `uint8_t buttonReads[9] = {0};` (zero-initialised).
- `readAllFromButtons()` writes only the slots it uses; future changes adding new pseudo-buttons would silently read stale values otherwise.

### Bug 3: `configureAnalogReference()` called after first calibration read
- In `setup()`, move `configureAnalogReference()` to before the first `readAllFromSensors(centerPoints)` call.
- Current order: `readAllFromSensors` → `delay(1000)` → `configureAnalogReference` → `readAllFromSensors` × 2. The first read uses the wrong 5V reference. The correct reference must be active before any calibration read.

### Bug 4: RP2040 silently maps 8 sensor pins to 4 ADC pins
- In `platformio.ini` `[env:rp2040]`, `HES_PIN_6..9` are mapped to the same GPIO 26–29 as `HES_PIN_0..3`. This produces duplicate sensor data with no warning.
- Add compile-time warning in `main.cpp`:
  ```cpp
  #ifdef RP2040_FOUR_ADC_ONLY
  #warning "RP2040: only 4 ADC pins available — HES6-9 disabled"
  #endif
  ```
- Add `-D RP2040_FOUR_ADC_ONLY` to `[env:rp2040]` build_flags in `platformio.ini`.
- In `main.cpp`, initialise `hallSensorEnabled[4..7] = false` when `RP2040_FOUR_ADC_ONLY` is defined.

### Bug 5: Duplicated `keyChange` bit-pack expression
- Lines 578 and 592 contain the identical 9-term sum. Extract to a named helper:
  ```cpp
  static uint16_t computeKeyChange(const uint8_t* bv) {
    return bv[0] + 2*bv[1] + 4*bv[2] + 8*bv[3] + 16*bv[4] +
           32*bv[5] + 64*bv[6] + 128*bv[7] + 256*bv[8];
  }
  ```

**Acceptance:** `pio run -e arduino-pro-micro`, `pio run -e rp2040`, `pio run -e esp32s3` all succeed. Manual review confirms the button mask correctly represents all 9 slots without truncation.

---

## Phase 2 — Code Quality Cleanup

**Files:** `pio-migration/src/main.cpp`

### 2.1 Remove dead commented-out code
- Lines 491–512: old button suppression block comment
- Lines 503–512: old single-button logical switch block comment
- Lines 764–769: removed speed calculation block comment
- Lines 92–93: `speed` variable comment

### 2.2 Extract repeated debug sensor print helper
- The 8-sensor `Serial.print` block appears at debug levels 1, 2, 3, and 5 (4 near-identical copies).
- Extract to:
  ```cpp
  void debugPrintSensors(const int* values, bool sameLine);
  ```
  Prints `HES0:val,HES1:val,...HES9:val` then `\r` or `\n`.

### 2.3 Extract repeated debug movement print helper
- The 6-axis movement print appears at debug levels 4 and 5.
- Extract to:
  ```cpp
  void debugPrintMovement(int16_t tx, int16_t ty, int16_t tz,
                          int16_t rx, int16_t ry, int16_t rz);
  ```

### 2.4 Style fixes
- Replace all 6 `if (invX == true)` patterns with `if (invX)`.
- Replace `transX = transX * -1` with `transX = -transX` (and equivalent for all 6 axes).

### 2.5 Const-correct globals
- `int PINLIST[8]` → `const int PINLIST[8]`
- `int BTNLIST[3]` → `const int BTNLIST[3]`
- `int DEADZONE = 40` → `const int DEADZONE = 40` (becomes a config entry in Phase 3)

### 2.6 Named constants for magic numbers
```cpp
constexpr uint8_t NUM_SENSORS    = 8;
constexpr uint8_t NUM_BTNS       = 3;
constexpr uint8_t NUM_BTN_VALUES = 9;
```
Replace hardcoded `8`, `3`, `9` array sizes throughout.

### 2.7 Add README note
- Add a short section to `pio-migration/README.md` stating that `pio-migration/` is the maintained firmware and `HE_Spacemouse/HE_Spacemouse.ino` is legacy/reference material.

**Acceptance:** All three environments build. Debug output levels 1–6 remain functionally equivalent. Movement math and button mapping unchanged.

---

## Phase 3 — Module Extraction (Heavy Split)

`main.cpp` becomes a thin orchestrator (~80–100 lines). Each module is extracted and verified to build before the next is started.

### File layout after Phase 3

```
pio-migration/
  include/
    config.h            ← compile-time defaults, constants, feature flags
    hid_platform.h      ← platform-agnostic HID interface declaration
    sensors.h           ← sensor reading, centering, filtering, calibration
    buttons.h           ← button state machine
    movement.h          ← 6-DOF movement calculation
    settings.h          ← runtime settings struct and defaults (pre-EEPROM)
    debug_print.h       ← serial debug helpers
  src/
    hid_avr.cpp         ← AVR HID implementation
    hid_rp2040.cpp      ← RP2040 HID implementation
    hid_esp32.cpp       ← ESP32-S3 HID implementation
    sensors.cpp
    buttons.cpp
    movement.cpp
    settings.cpp
    debug_print.cpp
    main.cpp            ← setup() + loop() only
```

### 3.1 `config.h` — compile-time settings
All user-tuneable values as `constexpr` or `#define`, overridable via `platformio.ini` build_flags:
- `DEBUG_LEVEL`, `DEBUG_SAME_LINE`
- `MOVEMENT_3DC`
- `CYCLE_BUTTON`, `CYCLE_INITIAL_BUTTON`
- `BUTTON_DELAY_MS`
- `INV_X/Y/Z/RX/RY/RZ`
- `DEADZONE`
- `HALL_SENSOR_ENABLED` (bitmask or array initialiser)
- Pin assignment defaults (overridden by platformio.ini)

### 3.2 `hid_platform.h` + per-platform `.cpp` files
Two functions, no `#ifdef` at call sites:
```cpp
void setupHID();
void sendHIDReport(uint8_t reportId, const uint8_t* data, uint8_t len);
```
Each platform `.cpp` contains the HID descriptor, class definition, and implementation guarded by a single top-level `#ifdef`. `platformio.ini` uses `src_filter` per environment to compile only the matching file. AVR environment adds:
```ini
lib_deps = NicoHood/HID
```

### 3.3 `sensors.h` / `sensors.cpp`
```cpp
void readAllFromSensors(int* rawReads);
void centerAndFilter(const int* rawReads, const int* centerPoints,
                     int* centered, int deadzone);
void captureCenter(int* centerPoints);  // calibration sequence
```

### 3.4 `buttons.h` / `buttons.cpp`
- Full state machine (keyState 0–6) moved here after Phase 2 cleanup.
- `void readAllFromButtons(uint8_t* buttonValues);`
- State variables (`keyState`, `keyPressed`, `keyTimeOld`) become file-scope `static`.

### 3.5 `movement.h` / `movement.cpp`
```cpp
void computeMovement(const int* centered,
                     int16_t& tx, int16_t& ty, int16_t& tz,
                     int16_t& rx, int16_t& ry, int16_t& rz);
```
Reads inversion flags from `config.h`.

### 3.6 `settings.h` / `settings.cpp`
Runtime settings struct populated from `config.h` defaults. Acts as the single source of truth for values that will be persisted in Phase 5:
```cpp
struct Settings {
  int  centerPoints[NUM_SENSORS];
  bool invX, invY, invZ, invRX, invRY, invRZ;
  bool movement3DC;
  bool cycleButton;
  int  deadzone;
};
void initSettings(Settings& s);  // populates from config.h defaults
```

### 3.7 `debug_print.h` / `debug_print.cpp`
`debugPrintSensors` and `debugPrintMovement` from Phase 2 moved here.

### 3.8 `main.cpp` after refactor
```
setup():  setupHID() → Serial.begin → pin modes → configureAnalogReference →
          initSettings → captureCenter
loop():   readAllFromSensors → readAllFromButtons → centerAndFilter →
          computeMovement → debug prints → send_command
```

**Acceptance:** Each module extracted in its own commit. All three environments build after every extraction. No formula or button mapping changes.

---

## Phase 4 — Logic Tests

Add PlatformIO native tests for pure functions extracted in Phase 3. Run with `pio test -e native` (add `[env:native]` to `platformio.ini`).

### Button state machine tests
- No button pressed → no button values set.
- Single left press held past `BUTTON_DELAY_MS` → `buttonValues[1]` set.
- Single right press held past delay → `buttonValues[3]` set.
- Single front press held past delay → `buttonValues[2]` set (or cycle pseudo-buttons when `cycleButton` true).
- Left + right within delay → `buttonValues[0]` (pseudo button 0).
- Front + left within delay → `buttonValues[4]` (pseudo button 1).
- Front + right within delay → `buttonValues[5]` (pseudo button 2).
- Cycling front button → `buttonValues[6]`, `[7]`, `[8]` on successive presses.

### Movement math tests
- All centered values zero → all translation and rotation outputs zero.
- Single-axis push (e.g. HES0 + HES1 positive) → expected `transY` only.
- Twist pattern (alternating sensor signs) → expected `rotZ` only, zero translation.
- Disabled Hall sensor (`hallSensorEnabled[i] = false`) → sensor contributes zero.
- Inversion flag → output sign flipped.

### HID packing tests
- Signed 16-bit value split into correct little-endian bytes via `lowByte16` / `highByte16`.
- Button mask for each of the 9 logical buttons maps to the correct bit in the 4-byte HID report.
- `computeKeyChange` returns correct `uint16_t` for all 9 button slots.

**Acceptance:** `pio test -e native` passes. Firmware environments still build.

---

## Phase 5 — EEPROM Persistence

**New files:** `include/persistent_config.h`, `src/persistent_config.cpp`

### Data persisted
```cpp
struct StoredConfig {
  uint8_t  magic;            // 0xSE — detect uninitialised EEPROM
  uint8_t  version;          // schema version — increment on struct changes
  uint8_t  crc;              // CRC-8 of remaining fields — detect partial corruption
  int      centerPoints[NUM_SENSORS];
  bool     invX, invY, invZ, invRX, invRY, invRZ;
  bool     movement3DC;
  bool     cycleButton;
  int      deadzone;
};
```

### API
```cpp
bool loadConfig(StoredConfig& cfg);       // false if uninitialised, wrong version, or bad CRC
void saveConfig(const StoredConfig& cfg); // call only on explicit user action, not every loop
void resetToDefaults(StoredConfig& cfg);  // populates from config.h defaults
```

### Platform abstraction
| Platform | Storage | Library |
|---|---|---|
| AVR | EEPROM hardware | `<EEPROM.h>` (Arduino AVR core) |
| RP2040 | Flash-backed emulation | `<EEPROM.h>` (Arduino-Pico core) |
| ESP32-S3 | NVS flash | `<Preferences.h>` |

Wrap platform differences behind internal `eepromRead` / `eepromWrite` helpers inside `persistent_config.cpp`, guarded by `#ifdef`. Non-validated platforms get a documented stub that compiles and logs a warning at boot.

### Boot flow change in `setup()`
```
loadConfig(cfg)
  → false: resetToDefaults → captureCenter → saveConfig
  → true:  apply stored centerPoints, flags, deadzone (skip motionless calibration)
```

### Write wear policy
- `saveConfig()` is called only through explicit code paths (future: button hold, serial command).
- Never called inside `loop()` unconditionally.

**Acceptance:** Invalid/blank EEPROM boots with safe defaults. Valid saved settings survive power cycle on AVR hardware. All three environments build; non-AVR platforms compile with stubs documented as unverified.

---

## Phase 6 — Board-Aware Configuration via `platformio.ini`

Move compile-time defaults behind `platformio.ini` build_flags so per-board tuning requires no C++ edits:

```ini
[env:arduino-pro-micro]
build_flags =
  -D SPACEMOUSE_DEBUG=0
  -D SPACEMOUSE_MOVEMENT_3DC=1
  -D SPACEMOUSE_DEADZONE=40
  -D SPACEMOUSE_ENABLED_HALL_MASK=0x0F
  -D SPACEMOUSE_CYCLE_BUTTON=1
```

`config.h` reads these macros with fallback defaults:
```cpp
#ifndef SPACEMOUSE_DEADZONE
  #define SPACEMOUSE_DEADZONE 40
#endif
```

Keep runtime state (cycle index, button state machine) as mutable variables — only compile-time defaults move to build_flags.

**Acceptance:** Changing `SPACEMOUSE_DEADZONE` in `platformio.ini` without editing C++ produces a build with the new deadzone. Default Pro Micro build matches original hardware behavior.

---

## Phase 7 — HID Portability Validation

Build success is not USB compatibility. This phase validates real enumeration per platform.

- Confirm AVR still requires custom board identity for 3DConnexion driver to recognize VID/PID `0x256f:0xc631`. Document the custom board setup requirement in `pio-migration/README.md`.
- For ESP32-S3: verify native USB enumerates with correct VID/PID. The ESP32 `hid_esp32.cpp` currently does not set `0x256f:0xc631` (unlike RP2040 which sets it in the class constructor). Fix if needed.
- For RP2040: verify the Arduino-Pico USB stack reliably exposes the HID descriptor and VID/PID on the Pico board.
- Record results in `pio-migration/README.md` per board: `✅ verified`, `⚠️ untested`, or `❌ known issue`.

**Acceptance:** README has an explicit hardware validation matrix. No platform is silently assumed compatible based on build success alone.

---

## Phase 8 — Documentation

Update `pio-migration/README.md` with:

- **Quick start:** build, upload, and serial monitor commands for each environment.
- **Target support matrix:** per-platform build status and hardware enumeration status.
- **Pin mapping table:** HES and button pin assignments per board.
- **Debug mode table:** what each debug level 0–6 outputs.
- **Persisted settings table:** which settings survive power cycle and how to reset.
- **Hardware validation checklist:** sensor test procedure at debug 1, calibration at debug 2, axis verification at debug 4.
- **Configuration reference:** all `SPACEMOUSE_*` build_flags with defaults and valid ranges.
- Statement that `pio-migration/` is the maintained firmware; `HE_Spacemouse/HE_Spacemouse.ino` is historical reference.

**Acceptance:** A new contributor can build, flash, and verify the device using the README alone without reading source code.

---

## Verification — Per Phase

Run after every phase from `pio-migration/`:
```bash
pio run -e arduino-pro-micro
pio run -e rp2040
pio run -e esp32s3
```

Run after Phase 4:
```bash
pio test -e native
```

Hardware checks (AVR baseline, record results per platform):
```bash
pio run -e arduino-pro-micro -t upload
pio device monitor -b 250000
```

| Check | Phase |
|---|---|
| No spurious HID button events during cycle-view button presses | 1 |
| Sensor readings stable at rest, debug level 2/3 | 1 |
| Debug levels 1–6 produce correct serial output | 2 |
| All logic tests pass | 4 |
| HID device recognised by 3DConnexion software, all 6 axes respond | 3, 7 |
| Settings survive power cycle | 5 |
| Blank EEPROM boots with safe defaults | 5 |
| Deadzone change via platformio.ini only, no C++ edit | 6 |
| VID/PID enumeration confirmed per platform | 7 |
