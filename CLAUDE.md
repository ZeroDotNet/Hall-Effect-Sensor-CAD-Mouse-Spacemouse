## Firmware Build & Flash

- Target platform is the YD-RP2040 (RP2040). Always verify firmware builds for all three RP2040 environments before declaring success.
- After modifying firmware, build and flash to confirm the device boots and reports correct USB identity.

## USB Identity Overrides

- Setting USB VID/PID via BOARD_VENDORID build flags does NOT work on this board; use a symbol-override file plus a linker flag instead.

## Diagram Generation

- The Chrome extension for diagram generation is unavailable; default to generating schematics with matplotlib and avoid emoji/Unicode glyphs (use ASCII text) to prevent rendering warnings.


# .claude/skills/verify-firmware/SKILL.md
Build all three RP2040 environments, flash the YD-RP2040, confirm it boots and reports the correct USB VID/PID (linker symbol override), and report any build errors.

// .claude/settings.json
{
  "hooks": {
    "PostToolUse": [
      {"matcher": "Edit|Write", "command": "pio run -e rp2040 2>&1 | tail -20"}
    ]
  }
}

use an agent to explore the pio-migration codebase and report all RP2040 framework API compatibility issues for USB HID and I2C

Create a HARDWARE.md documenting the YD-RP2040 pinout (WS2812B on GPIO23, ADC pins, I2C for TLV493D), the three PlatformIO build environments, and the USB VID/PID linker override approach.

After firmware builds clean and flashes successfully, stage the changes and create a commit summarizing the fix and which RP2040 environments were verified.


Set up an autonomous verify-fix-deploy loop for my RP2040 SpaceMouse firmware. Build all three PlatformIO environments, parse compiler and linker errors, fix USB HID/ADC/movement-math bugs automatically, re-flash the connected device, and run a smoke test that confirms HID enumeration with the correct VID/PID. Keep iterating until every environment builds and the device responds correctly, then summarize all changes made.

Spawn three parallel agents on separate git worktrees to implement RP2040 SpaceMouse sensor backends: one for TLV493D, one for ADC joystick, one for WS2812B status feedback integration. Each agent should research specs, assign GPIO pins, implement and build the firmware, verify framework API compatibility, and produce an implementation plan. Then merge results into a single comparison matrix recommending the best approach.


Create a documentation pipeline that scans my RP2040 firmware for all GPIO/peripheral assignments (WS2812B, ADC, USB, I2C), then auto-generates a wiring diagram and pinout table. Use a robust matplotlib/graphviz template with ASCII-safe labels to avoid glyph warnings, and export a clean multi-page PDF datasheet. Set it up to regenerate automatically whenever pin definitions change in source.