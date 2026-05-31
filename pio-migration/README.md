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
pio run -e rp2040-tlv493d-1-teleplot-raw
pio run -e rp2040-tlv493d-1-teleplot-centered
pio run -e rp2040-tlv493d-1-teleplot-filtered
pio run -e rp2040-tlv493d-1-teleplot-motion
pio run -e rp2040-tlv493d-1-teleplot-side-by-side
pio run -e rp2040-tlv493d-3-teleplot
pio run -e rp2040-tlv493d-3-teleplot-raw
pio run -e rp2040-tlv493d-3-teleplot-centered
pio run -e rp2040-tlv493d-3-teleplot-filtered
pio run -e rp2040-tlv493d-3-teleplot-motion
pio run -e rp2040-tlv493d-3-teleplot-side-by-side
pio run -e rp2040-led-test
pio run -e rp2040-rgb-led-test
pio run -e rp2040-rgb-led-scan
```

## RP2040 upload on Windows

Use the validated upload command:

```sh
pio run --target upload -e rp2040-tlv493d-3
```

For the Pico environment, the upload step uses a custom UF2 uploader. It will
reset the board over USB serial when a matching COM port is available, then
copy the generated UF2 to the `RPI-RP2` boot drive. This avoids the default
`rp2040load` path, which was failing on Windows.

If the board is already in BOOTSEL mode, the same upload command still works.

If multiple serial devices are connected, pass an explicit port:

```sh
pio run --target upload -e rp2040-tlv493d-3 --upload-port COM9
```

VS Code tasks are available in `.vscode/tasks.json` for build and upload.

## RP2040 onboard LED diagnostics

Two dedicated LED test environments are available:

```sh
pio run --target upload -e rp2040-led-test
pio run --target upload -e rp2040-rgb-led-test
```

If the RGB LED still does not light, run the pin sweep build:

```sh
pio run --target upload -e rp2040-rgb-led-scan
pio device monitor -b 250000
```

That test cycles WS2812 output across GPIO16-GPIO24 and prints the current pin over serial.
If none of those pins lights the onboard RGB LED, the practical next check is the board hardware:
many YD-RP2040 boards leave the RGB LED disconnected until the `RGB`/`R58`/`R68` solder bridge is closed.

## Teleplot debug forwarding

The RP2040 cannot send UDP directly to `127.0.0.1`, so Teleplot support is split
into two parts:

1. The firmware emits debug lines in Teleplot-friendly `name:value` form over USB serial.
2. A host-side PowerShell bridge forwards those serial lines to `127.0.0.1:47269` over UDP.

Use the Teleplot-ready environment to build and upload firmware with debug output enabled. For the current 1-sensor prototype:

```sh
pio run --target upload -e rp2040-tlv493d-1-teleplot-motion
```

Available Teleplot environments are:

- `rp2040-tlv493d-1-teleplot-raw`: `DEBUG_LEVEL=1`
- `rp2040-tlv493d-1-teleplot-centered`: `DEBUG_LEVEL=2`
- `rp2040-tlv493d-1-teleplot-filtered`: `DEBUG_LEVEL=3`
- `rp2040-tlv493d-1-teleplot-motion`: `DEBUG_LEVEL=4`
- `rp2040-tlv493d-1-teleplot-side-by-side`: `DEBUG_LEVEL=5`
- `rp2040-tlv493d-3-teleplot-raw`: `DEBUG_LEVEL=1`
- `rp2040-tlv493d-3-teleplot-centered`: `DEBUG_LEVEL=2`
- `rp2040-tlv493d-3-teleplot-filtered`: `DEBUG_LEVEL=3`
- `rp2040-tlv493d-3-teleplot-motion`: `DEBUG_LEVEL=4`
- `rp2040-tlv493d-3-teleplot-side-by-side`: `DEBUG_LEVEL=5`

Each of them enables:

```ini
-D DEBUG_TELEPLOT=1
```

The compatibility aliases `rp2040-tlv493d-1-teleplot` and `rp2040-tlv493d-3-teleplot` map to the motion view.

Run the host bridge with:

```powershell
pwsh -ExecutionPolicy Bypass -File .\scripts\serial_to_teleplot.ps1 -SerialPort COM9 -BaudRate 250000 -UdpHost 127.0.0.1 -UdpPort 47269
```

To start the bridge and serial monitor together in one action:

```powershell
pwsh -ExecutionPolicy Bypass -File .\scripts\start_teleplot_session.ps1 -SerialPort COM9 -BaudRate 250000 -UdpHost 127.0.0.1 -UdpPort 47269
```

The VS Code tasks now let you pick the Teleplot firmware environment for build/upload, and `Start Teleplot Session` starts both the UDP bridge and the serial monitor.

Stop the monitor/bridge session before uploading a different Teleplot firmware mode, otherwise the USB serial port will stay busy and the RP2040 reset touch cannot open `COM9`.

For the 1-sensor TLV493D RP2040 wiring, sensor 1 uses I2C0 on GP4/GP5 with
address `A0`. For the 3-sensor wiring, sensor 1 stays on GP4/GP5; sensors 2 and
3 share I2C1 on GP2/GP3 with addresses `A1` and `A0`.

TLV493D scaling can be tuned with build flags:

```ini
-D TLV493D_SCALE=10.0f
-D TLV493D_DEADZONE=0.4f
```
