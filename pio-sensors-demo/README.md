# Magnetic Field Puppet

Demo PlatformIO project for a Raspberry Pi Pico, a TLV493D 3-axis magnetic
sensor, a hobby servo, and an FZ1556 LED module. A magnet becomes the controller:
the TLV493D Z axis moves the servo pointer while the magnetic-field magnitude
drives LED brightness.

## Build And Run

```sh
pio run
pio run -t upload
pio device monitor -b 115200
```

## Default Wiring

| Part | Pico Pin | Notes |
| --- | --- | --- |
| TLV493D SDA | GP4 | I2C0 data, pullups required if the breakout lacks them |
| TLV493D SCL | GP5 | I2C0 clock |
| TLV493D VCC | 3V3 | Do not power the sensor from 5 V |
| TLV493D GND | GND | Shared ground |
| Servo signal | GP15 | Servo power must come from a separate 5 V rail |
| Servo V+ | External 5 V | Tie the external supply ground to Pico GND |
| Servo GND | GND | Shared with Pico and sensor |
| FZ1556 LED signal | GP16 | PWM brightness output |
| FZ1556 VCC/GND | 3V3/GND | Use a resistor if the LED board does not include one |

## Electrical Notes

- The TLV493D is a 3.3 V I2C device. Keep SDA/SCL pullups on 3.3 V.
- A small servo can pull hundreds of mA when it starts or stalls. Use an
  external 5 V supply and a common ground; do not power it from Pico 3V3.
- If the servo jitters when the magnet moves, add bulk capacitance on the servo
  rail and route servo power away from the I2C wires.
- The firmware reads TLV493D address `0x5E`. If your breakout uses a different
  address, update `TLV493D_ADDRESS` in `src/main.cpp`.
- The LED pin is a single PWM channel. For an RGB FZ1556 module, connect one
  color input or expand the sketch to drive three PWM pins.

## Files

- `src/main.cpp`: complete Arduino firmware with serial logging.
- `docs/wiring.svg`: wiring reference.
- `docs/block-diagram.svg`: data-flow diagram for the demo.
