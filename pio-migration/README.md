# PlatformIO Migration

This folder contains a PlatformIO version of the original `HE_Spacemouse.ino`
Arduino sketch.

## Environments

- `arduino-pro-micro`: SparkFun Pro Micro 5V/16MHz, ATmega32U4.
- `rp2040`: Raspberry Pi Pico, RP2040.
- `esp32s3`: Espressif ESP32-S3 DevKitC-1.

Build from this folder with:

```sh
../pio.ps1 run -e arduino-pro-micro
../pio.ps1 run -e rp2040
../pio.ps1 run -e esp32s3
```

Use the repository wrapper `../pio.ps1` on Windows. It runs PlatformIO from the
local Python 3.12 virtual environment and keeps PlatformIO's core/cache local to
this repo, avoiding conflicts with global Python 3.14 installations.

The sketch emulates a USB HID 3DConnexion SpaceMouse-compatible device. Pin
assignments and USB HID support are board/core-specific, so verify wiring and
USB behavior on real hardware after a successful build.

The original project uses eight analog Hall sensor inputs. The generic
Raspberry Pi Pico only exposes four ADC-capable GPIOs, so the `rp2040`
environment repeats those pins as compile-ready placeholders. Update the
`HES_PIN_*` build flags if your RP2040 hardware uses an external ADC, mux, or a
different board layout.

## Sensor backend selection

The analog Hall/SS49e backend remains the default. You can select the sensor
backend at build time with environment variables:

```sh
# Default/current hardware: analog Hall sensors
set SPACEMOUSE_SENSOR_BACKEND=hall
pio run -e rp2040

# TLV493D, 3 sensors
set SPACEMOUSE_SENSOR_BACKEND=tlv493d
set SPACEMOUSE_TLV493D_COUNT=3
pio run -e rp2040

# TLV493D, 1 sensor
set SPACEMOUSE_SENSOR_BACKEND=tlv493d
set SPACEMOUSE_TLV493D_COUNT=1
pio run -e rp2040
```

There are also shortcut environments:

```sh
pio run -e rp2040-tlv493d-3
pio run -e rp2040-tlv493d-1
```

For the 3-sensor TLV493D RP2040 wiring, sensor 1 and sensor 2 use the default
I2C bus on GP4/GP5 with addresses `A0` and `A1`. Sensor 3 uses a second Mbed
I2C instance on GP2/GP3 with address `A0`.

TLV493D scaling can be tuned with build flags:

```ini
-D TLV493D_SCALE=10.0f
-D TLV493D_DEADZONE=0.4f
```
