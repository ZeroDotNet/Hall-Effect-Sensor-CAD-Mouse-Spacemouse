# Report: Comparison of `plan-claude.md` and `plan-codex.md`

## Executive Summary

`plan-codex.md` is the better base plan to execute. It is more conservative, more aligned with the selected requirements, and safer for firmware work because it keeps each phase buildable/testable, explicitly separates compile verification from hardware verification, and avoids making unchecked assumptions about RP2040/ESP32 storage and USB behavior.

`plan-claude.md` is more concrete and implementation-ready in some areas, especially line-level bug notes, proposed file layout, and Phase 1/2 task detail. However, it also contains several risks and at least one stale or incorrect observation relative to the current codebase. The best path is to use `plan-codex.md` as the governing plan and selectively import the useful concrete tasks from `plan-claude.md` after verifying them against the current source.

## Verdict

Best primary plan: `plan-codex.md`

Recommended execution strategy:

1. Keep `plan-codex.md` as the canonical roadmap.
2. Merge specific implementation details from `plan-claude.md` into task tickets or phase notes.
3. Do not blindly apply `plan-claude.md`; verify each claimed bug and platform assumption before coding.

## Comparison Matrix

| Criterion | `plan-claude.md` | `plan-codex.md` | Better |
|---|---|---|---|
| Alignment with user choices | Good: uses PlatformIO as forward codebase, heavy split, EEPROM option B | Excellent: explicitly records option D, option C, option B, and incremental delivery | `plan-codex.md` |
| Actionability | Very high: line numbers, concrete function names, file layout | Medium-high: phased roadmap with acceptance criteria | `plan-claude.md` |
| Accuracy against current repo | Mixed: contains stale/incorrect details and risky assumptions | Better: reflects current build results and uncertainty boundaries | `plan-codex.md` |
| Firmware safety | Good incremental intent, but fewer test gates before EEPROM | Strong: build gates, logic tests, hardware validation boundaries | `plan-codex.md` |
| Architecture design | Concrete but potentially too aggressive with `src_filter` and per-platform files | Clear heavy split while preserving incremental extraction | Tie, slight edge to `plan-codex.md` |
| EEPROM design | Concrete struct/API, but platform assumptions need verification | More cautious and portable; includes magic/version/checksum and backend uncertainty | `plan-codex.md` |
| Testing strategy | Mostly build + hardware validation | Adds pure logic tests for buttons, movement math, HID packing | `plan-codex.md` |
| Documentation plan | Minimal | Strong: target matrix, pin map, debug modes, persisted settings | `plan-codex.md` |

## Strengths of `plan-claude.md`

- It gives a very concrete implementation path.
- It identifies the `keyChange` overflow clearly and explains the failure mode.
- It proposes useful helpers:
  - `computeKeyChange()`
  - `debugPrintSensors()`
  - `debugPrintMovement()`
- It gives a clear target module layout:
  - `config.h`
  - `hid_platform.h`
  - `sensors.h/.cpp`
  - `buttons.h/.cpp`
  - `movement.h/.cpp`
  - `debug_print.h/.cpp`
  - per-platform HID files
- It correctly treats `pio-migration/` as the future codebase and the `.ino` as legacy.
- Its EEPROM API proposal is useful as a starting point:
  - `loadConfig()`
  - `saveConfig()`
  - `resetToDefaults()`

## Weaknesses and Risks in `plan-claude.md`

### 1. Stale or incorrect setup-order bug

`plan-claude.md` says `configureAnalogReference()` is called after the first sensor read. In the current `pio-migration/src/main.cpp`, `configureAnalogReference()` is already called before the center reads in `setup()`. This means Bug 2 should not be treated as a confirmed issue without rechecking the exact source lines.

Recommendation: remove this from the bug list or downgrade it to "verify setup calibration order".

### 2. RP2040 plan is partially useful but too specific

The RP2040 note is directionally correct: the target repeats four ADC pins across eight Hall sensor definitions, so it is compile-ready but not hardware-complete.

The risky part is prescribing an exact compile flag and behavior immediately:

- `-D RP2040_FOUR_ADC_ONLY`
- disable `hallSensorEnabled[4..7]`

That may be correct for a four-sensor prototype, but it may be wrong if the intended RP2040 design uses an external ADC, mux, or different board. `plan-codex.md` handles this better by documenting the target as placeholder unless a real hardware plan is chosen.

Recommendation: keep the warning/documentation, but do not hard-code four-sensor behavior until the RP2040 hardware goal is confirmed.

### 3. `src_filter` should not be introduced as written

`plan-claude.md` proposes `src_filter` per environment. PlatformIO projects should prefer `build_src_filter`; using the older key risks carrying deprecated configuration forward.

Recommendation: if per-platform HID files are split, use `build_src_filter`, or structure files so only the active platform compiles through preprocessor guards.

### 4. EEPROM platform assumptions need verification

`plan-claude.md` states:

- RP2040: `<EEPROM.h>` from Arduino-Pico core
- ESP32-S3: `<Preferences.h>`

But the current `rp2040` build uses the PlatformIO `raspberrypi` platform with Arduino Mbed, not necessarily the Arduino-Pico core. Storage support must be checked before committing to that API.

Recommendation: keep `plan-codex.md`'s cautious backend approach. Implement AVR first, then add/stub platform backends intentionally.

### 5. `DEADZONE` const conflicts with EEPROM persistence

`plan-claude.md` proposes changing `DEADZONE` to `const int` in Phase 2, then later persisting it in EEPROM. If `DEADZONE` becomes a runtime setting, it should move into a settings/config object instead of becoming a hard constant.

Recommendation: make compile-time default `DEFAULT_DEADZONE` const, and runtime value `settings.deadzone`.

### 6. No pure logic test phase

`plan-claude.md` relies mostly on build and hardware validation. That is not enough for the button state machine and movement math once the code is split.

Recommendation: keep `plan-codex.md` Phase 4 logic tests before EEPROM work.

## Strengths of `plan-codex.md`

- It explicitly records the selected decisions:
  - priority option D
  - PlatformIO as the sole forward codebase
  - architecture option C
  - EEPROM option B
  - incremental delivery
- It separates bug fixes, cleanup, module extraction, tests, EEPROM, configuration, HID validation, and documentation into safer phases.
- It treats build success as distinct from USB/HID hardware success.
- It includes logic tests before EEPROM persistence.
- It avoids making unverified assumptions about RP2040 storage and USB behavior.
- It requires invalid EEPROM data to fall back safely using magic/version/checksum or CRC.
- It includes documentation deliverables that will matter later:
  - target support matrix
  - pin mapping table
  - debug mode table
  - persisted settings table
  - hardware validation checklist

## Weaknesses of `plan-codex.md`

- It is less concrete than `plan-claude.md` for immediate implementation.
- It does not mention the possible setup-order issue, though that issue appears stale against the current source.
- It does not specify exact module filenames as consistently as `plan-claude.md`.
- It does not include the concrete EEPROM API shape from `plan-claude.md`, which is useful.
- It could import the exact helper names and implementation details from `plan-claude.md` to make Phase 1 and Phase 2 easier to execute.

## Recommended Hybrid Plan

Use `plan-codex.md` as the master plan, with these additions from `plan-claude.md`:

### Add to Phase 1

- Implement `uint16_t computeKeyChange(const uint8_t *buttonValues)`.
- Replace both repeated key-mask expressions with `computeKeyChange()`.
- Verify the `configureAnalogReference()` setup order, but do not list it as a confirmed bug unless the current code changes.
- Document the RP2040 duplicated ADC mapping. Add a warning or README note first; only disable sensors by build flag if that is the intended RP2040 hardware mode.

### Add to Phase 2

- Add `debugPrintSensors()`.
- Add `debugPrintMovement()`.
- Add named constants:
  - `NUM_SENSORS`
  - `NUM_BTNS`
  - `NUM_BTN_VALUES`
- Use `DEFAULT_DEADZONE` instead of making runtime `DEADZONE` permanently const.

### Add to Phase 3

Adopt this file layout, with naming adjusted to the repo style:

```text
pio-migration/
  include/
    Config.h
    HidDevice.h
    Sensors.h
    Buttons.h
    Motion.h
    DebugOutput.h
    Settings.h
  src/
    HidDeviceAvr.cpp
    HidDeviceRp2040.cpp
    HidDeviceEsp32.cpp
    Sensors.cpp
    Buttons.cpp
    Motion.cpp
    DebugOutput.cpp
    Settings.cpp
    main.cpp
```

Use `build_src_filter` if only one platform HID implementation should compile per environment.

### Add to Phase 5

Use a versioned persisted settings struct similar to:

```cpp
struct StoredConfig {
  uint32_t magic;
  uint8_t version;
  int centerPoints[8];
  bool invX;
  bool invY;
  bool invZ;
  bool invRX;
  bool invRY;
  bool invRZ;
  bool movement3DC;
  bool cycleButton;
  int deadzone;
  uint16_t checksum;
};
```

Keep the API shape:

```cpp
bool loadConfig(StoredConfig &cfg);
void saveConfig(const StoredConfig &cfg);
void resetToDefaults(StoredConfig &cfg);
```

But implement platform storage backends only after checking each active PlatformIO core.

## Final Recommendation

`plan-codex.md` should remain the canonical plan because it is safer and more accurate for this firmware migration. `plan-claude.md` should be treated as an implementation-detail supplement, not as the controlling roadmap.

The immediate next work should be:

1. Implement Phase 1 from `plan-codex.md`.
2. Import `computeKeyChange()` from `plan-claude.md`.
3. Rebuild all three environments.
4. Then continue with Phase 2 cleanup using `debugPrintSensors()` and `debugPrintMovement()`.

