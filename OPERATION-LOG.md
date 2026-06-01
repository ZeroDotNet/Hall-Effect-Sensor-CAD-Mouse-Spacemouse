# Operation Log

## 2026-05-26

- Added `pio-sensors-demo/`, a Raspberry Pi Pico PlatformIO demo project named
  "Magnetic Field Puppet".
- Implemented `src/main.cpp` with TLV493D I2C reads, serial logging, servo-angle
  mapping from magnetic Z axis, and PWM LED brightness from field magnitude.
- Added `docs/wiring.svg` and `docs/block-diagram.svg` for wiring and runtime
  signal flow.
- Documented power, connectivity, and compatibility notes in
  `pio-sensors-demo/README.md`.
