# PlatformIO Migration Optimization Plan

## Current State

The `pio-migration` project is a PlatformIO port of `HE_Spacemouse/HE_Spacemouse.ino`.
It currently builds for:

- `arduino-pro-micro`: SparkFun Pro Micro 5V/16 MHz, ATmega32U4.
- `rp2040`: Raspberry Pi Pico placeholder target.
- `esp32s3`: ESP32-S3 DevKitC-1 native USB target.

Validation run from `pio-migration` on 2026-05-17:

- `pio run -e arduino-pro-micro`: success.
- `pio run -e rp2040`: success, with upstream RP2040 framework warnings.
- `pio run -e esp32s3`: success.

Important review notes:

- `platformio.ini` has target-specific pin overrides for RP2040 and ESP32-S3, but the RP2040 target repeats the same four ADC pins for eight Hall sensors. That is compile-ready, not hardware-complete.
- `src/main.cpp` still contains most of the original sketch as one large file. HID setup, board abstraction, sensor reading, button state machine, movement math, debug printing, and report sending are tightly coupled.
- Runtime configuration is currently edited in code through globals such as `debug`, `movement3DC`, inversion flags, `DEADZONE`, and `hallSensorEnabled`.
- `keyChange` is declared as `uint8_t`, but it is compared against a 9-bit button mask that can reach `256`. This can hide changes involving `buttonValues[8]`.
- Debug output is repeated in several long `Serial.print` blocks, which increases change risk when adding/removing sensors or report fields.
- `pio-migration/.pio` build artifacts exist in the working tree, but `.gitignore` already excludes them.

## Goals

1. Apply all improvements, prioritized as: bug fixes and code quality, architecture refactor, then feature improvements.
2. Treat `pio-migration` as the sole going-forward firmware codebase. `HE_Spacemouse/HE_Spacemouse.ino` is legacy/reference material and does not need to stay in sync.
3. Preserve the working Arduino Pro Micro behavior as the baseline while changes are made.
4. Make board-specific pin, ADC, and USB/HID differences explicit.
5. Reduce risk in the button and movement logic before larger refactors.
6. Make configuration changes possible without editing scattered code.
7. Add repeatable validation for build support and pure logic behavior.
8. Persist calibration plus user preferences across power cycles.
9. Document what is compile-verified versus hardware-verified.

## Chosen Scope

- Priority focus: option D, all of the proposed work, prioritized in this order:
  1. Bug fixes and code quality.
  2. Architecture refactor.
  3. Feature improvements.
- Source of truth: `pio-migration` only. Do not spend effort keeping the legacy `.ino` synchronized.
- Architecture split: option C, heavy module treatment with config, platform abstraction, and one file per concern.
- EEPROM/runtime config: option B, persist calibration plus user preferences:
  - `centerPoints[]`
  - `invX`, `invY`, `invZ`, `invRX`, `invRY`, `invRZ`
  - `movement3DC`
  - `cycleButton`
  - `DEADZONE`
- Delivery style: incremental phases. Each phase must leave the firmware compilable and testable.

## Phase 1: Bug Fixes

- Change `keyChange` from `uint8_t` to `uint16_t`, because the current button mask includes `256 * buttonValues[8]`.
- Replace repeated button-mask expressions with a named helper, for example `uint16_t buildButtonStateMask(const uint8_t *buttonValues)`.
- Initialize `buttonReads` in `loop()` before calling `readAllFromButtons()`. The current function writes the real and pseudo slots it uses, but explicit initialization makes future button changes safer.
- Keep the button report output byte-for-byte equivalent for existing button combinations, except for the intentional `buttonValues[8]` tracking fix.
- Keep `arduino-pro-micro` as the baseline behavior target.

Acceptance:

- `pio run -e arduino-pro-micro`
- `pio run -e rp2040`
- `pio run -e esp32s3`
- Manual review confirms the button mask can represent all nine button slots.

## Phase 2: Code Quality Cleanup

- Make `PINLIST`, `BTNLIST`, `_hidReportDescriptor`, and fixed lookup data `const` where platform APIs allow it.
- Extract repeated debug printing into helpers before larger module movement.
- Remove obsolete commented-out code blocks only when their behavior is already captured by current code, comments, tests, or commit history.
- Replace `if (flag == true)` with `if (flag)` only while touching nearby code, not as a standalone churn pass.
- Normalize naming and comments only where they are adjacent to touched logic.
- Add a short `pio-migration/README.md` section that says `pio-migration` is the maintained firmware and the `.ino` is legacy/reference material.

Acceptance:

- All three PlatformIO environments still build.
- Debug output remains functionally equivalent.
- The cleanup does not change movement math or button mapping.

## Phase 3: Heavy Module Extraction

Split `src/main.cpp` into small modules while keeping the Arduino entry points simple:

- `Config.h`: compile-time defaults, constants, pin definitions, feature flags.
- `HidDevice.h/.cpp`: platform-specific HID descriptor setup and `sendHidReport()`.
- `Sensors.h/.cpp`: sensor pin table, center calibration, deadzone filtering.
- `Buttons.h/.cpp`: physical button reads, pseudo-button state machine, button report mask.
- `Motion.h/.cpp`: centered sensor values to `transX`, `transY`, `transZ`, `rotX`, `rotY`, `rotZ`.
- `DebugOutput.h/.cpp`: serial output formatting for debug levels.
- `Settings.h/.cpp`: runtime settings structure and defaults, added before EEPROM persistence.
- `main.cpp`: `setup()`, `loop()`, and orchestration only.

Keep each extraction buildable before moving to the next one. Do not change formulas during the extraction.

Acceptance:

- Each module has one clear responsibility.
- `main.cpp` becomes short enough to audit quickly.
- All three environments build after every module extraction.

## Phase 4: Add Logic Tests

Add native or embedded PlatformIO tests for pure functions after Phase 3:

- Button state-machine tests:
  - no button pressed.
  - single left/right/front press.
  - left + right pseudo button.
  - front + left pseudo button.
  - front + right pseudo button.
  - cycling front button mode.
- Movement math tests:
  - neutral centered values produce zero translation and rotation.
  - single-axis push creates expected translation.
  - twist pattern creates expected `rotZ`.
  - disabled Hall sensors do not contribute.
- HID packing tests:
  - signed 16-bit values are split into expected little-endian bytes.
  - button bit masks map to expected report bytes.

Acceptance:

- Tests run with `pio test -e native` if a native test environment is added, or with target-specific `pio test` if kept embedded.
- Build verification still passes for all firmware environments.

## Phase 5: EEPROM Persistence

- Add a versioned settings record for persisted values.
- Persist calibration:
  - `centerPoints[8]`
- Persist user preferences:
  - `invX`, `invY`, `invZ`, `invRX`, `invRY`, `invRZ`
  - `movement3DC`
  - `cycleButton`
  - `DEADZONE`
- Include a magic value, schema version, and checksum/CRC so invalid EEPROM data falls back to compiled defaults.
- Load settings during `setup()` before sensor processing.
- Save settings only through explicit code paths to avoid unnecessary EEPROM wear.
- Do not add recalibration-on-demand yet. That was not selected for this pass.
- Account for platform storage differences:
  - AVR: `EEPROM` library.
  - ESP32-S3: Arduino `Preferences` or EEPROM emulation, chosen after a quick compatibility check.
  - RP2040: EEPROM emulation or file/flash-backed storage, chosen after confirming the active Arduino core support.

Acceptance:

- Invalid or blank storage boots with safe defaults.
- Valid saved settings override compiled defaults.
- Calibration and preferences survive a power cycle on hardware for the baseline target.
- All three environments build, even if non-AVR persistence is initially implemented as a backend stub with explicit documentation.

## Phase 6: Make Configuration Board-Aware

- Move defaults such as `debug`, `movement3DC`, `cycleButton`, inversion flags, deadzone, enabled Hall sensors, and persistence defaults behind named macros or a config struct.
- Allow `platformio.ini` to override configuration with `build_flags`, for example:
  - `-D SPACEMOUSE_DEBUG=0`
  - `-D SPACEMOUSE_MOVEMENT_3DC=1`
  - `-D SPACEMOUSE_DEADZONE=40`
  - `-D SPACEMOUSE_ENABLED_HALL_MASK=0x0F`
- Keep runtime globals only where state changes during operation, such as the cycle index and button state machine.
- For RP2040, either document the repeated ADC pins as a compile placeholder or define a real hardware plan using an external ADC, analog mux, or a board with enough analog inputs.

Acceptance:

- A user can change common tuning values in `platformio.ini` without editing C++.
- The default Pro Micro build matches the original intended hardware.
- RP2040 and ESP32-S3 configurations are clearly marked as tested, untested, or placeholder.

## Phase 7: Improve HID Portability

- Keep one shared HID descriptor definition, but hide each platform implementation behind a small common interface.
- Confirm whether the AVR target still needs custom board identity outside PlatformIO for the 3DConnexion driver to recognize VID/PID `0x256f:0xc631`.
- For ESP32-S3, document native USB requirements and test actual enumeration with 3DConnexion software.
- For RP2040, verify whether the Mbed USB stack can expose the intended VID/PID and report descriptor reliably on the target board.

Acceptance:

- Build success is not treated as USB compatibility proof.
- README records actual enumeration results per board when hardware testing is done.

## Phase 8: Documentation and Cleanup

- Update `pio-migration/README.md` with:
  - quick build commands.
  - upload commands per environment.
  - serial monitor command.
  - target support matrix.
  - pin mapping table.
  - debug mode table.
  - persisted settings table.
  - hardware validation checklist.
- State that `pio-migration` is the primary maintained firmware location.
- Remove or ignore generated `.pio` artifacts from git if any were accidentally staged.
- Add a root-level note that the original Arduino sketch is historical reference and `pio-migration` is the maintained PlatformIO port.

Acceptance:

- A new contributor can build the firmware from the README alone.
- The repository makes clear which firmware source should be edited for new work.

## Suggested Execution Order

1. Phase 1 bug fixes.
2. Phase 2 code quality cleanup.
3. Phase 3 heavy module extraction in small commits.
4. Phase 4 tests for extracted pure logic.
5. Phase 5 EEPROM persistence for calibration and preferences.
6. Phase 6 board-aware configuration cleanup.
7. Phase 7 hardware USB validation and HID portability.
8. Phase 8 final docs and repository cleanup.

## Verification Checklist

Run from `pio-migration` after each implementation phase:

```sh
pio run -e arduino-pro-micro
pio run -e rp2040
pio run -e esp32s3
```

When hardware is connected:

```sh
pio run -e arduino-pro-micro -t upload
pio device monitor -b 250000
```

Hardware checks to record:

- Device enumerates as the intended SpaceMouse-compatible HID.
- 3DConnexion software detects the device.
- Debug level 1 reports all enabled Hall sensors.
- Debug level 2/3 confirms centered and deadzone-filtered values.
- Debug level 4/5 confirms expected axis mapping.
- All physical and pseudo buttons generate the intended report actions.
