# PlatformIO Migration

This folder contains a PlatformIO version of the original `HE_Spacemouse.ino`
Arduino sketch.

## Environments

- `arduino-pro-micro`: SparkFun Pro Micro 5V/16MHz, ATmega32U4.
- `rp2040`: Raspberry Pi Pico, RP2040.
- `esp32s3`: Espressif ESP32-S3 DevKitC-1.

Build from this folder with:

```sh
pio run -e arduino-pro-micro
pio run -e rp2040
pio run -e esp32s3
```

The sketch emulates a USB HID 3DConnexion SpaceMouse-compatible device. Pin
assignments and USB HID support are board/core-specific, so verify wiring and
USB behavior on real hardware after a successful build.

The original project uses eight analog Hall sensor inputs. The generic
Raspberry Pi Pico only exposes four ADC-capable GPIOs, so the `rp2040`
environment repeats those pins as compile-ready placeholders. Update the
`HES_PIN_*` build flags if your RP2040 hardware uses an external ADC, mux, or a
different board layout.
