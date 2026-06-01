"""
Demo project for RP2040 controlling an FZ1556 LED ring, a servo and a
TLV493D magnetometer.

This script is intended to run under MicroPython on a board based
around the RP2040 (e.g. Raspberry Pi Pico or other boards using the
same microcontroller).  It demonstrates how to read 3D magnetic field
data from a TLV493D sensor, map the X‑axis value to a servo angle and
display the field magnitude on an FZ1556 LED ring (a WS2812/NeoPixel
compatible ring).  The servo is driven using the on‑chip PWM block and
the LED ring is driven using the built‑in neopixel module.

Hardware connections
--------------------

The example assumes the following pin assignments.  You can modify
`SERVO_PIN`, `LED_PIN`, `SDA_PIN` and `SCL_PIN` to match your
hardware setup.

* `SERVO_PIN` (GPIO 0) – output to control the servo motor.
* `LED_PIN`   (GPIO 28) – data line for the WS2812 LED ring.
* `SDA_PIN`   (GPIO 0) – I²C data for the TLV493D (use unique pin if
  servo shares GP0; you can choose any available pin).  Note: you
  cannot share the servo pin with I²C in real hardware; adjust these
  values accordingly.
* `SCL_PIN`   (GPIO 1) – I²C clock for the TLV493D.

The TLV493D operates from 3.3 V and uses the I²C address 0x5E.
Ensure you power the sensor and connect its SDA/SCL pins to the
corresponding RP2040 pins with appropriate pull‑up resistors.  The
FZ1556 ring should be powered from 5 V or 3.3 V (depending on its
specification) and ground connected to the board.  Connect the
data input (DIN) of the ring to `LED_PIN`.

See the accompanying README.md for more details and citations on the
hardware features.
"""

import time
from machine import Pin, PWM, I2C

try:
    import neopixel  # type: ignore
except ImportError:
    # On some MicroPython builds neopixel is a C‑module named "neopixel"
    import neopixel

from tlv493d import TLV493D


# Pin configuration
# Change these pins to match your hardware.  By default we place the
# servo on GP0, the LED ring on GP28 and the I²C bus on GP4/GP5,
# which are the default I²C pins on the Raspberry Pi Pico.  This
# avoids conflicts between the servo and the sensor.
SERVO_PIN = 0     # GPIO pin for servo signal (GP0)
LED_PIN = 28      # GPIO pin for LED ring data (GP28)
SDA_PIN = 4       # I2C SDA (default GP4 on Pico)
SCL_PIN = 5       # I2C SCL (default GP5 on Pico)

# LED ring configuration
NUM_PIXELS = 24    # Number of LEDs on the FZ1556 ring


def setup_servo(pin_num):
    """Initialize and return a PWM object configured for a servo.

    Servos typically expect a 50 Hz PWM signal (20 ms period).  The
    duty cycle controls the angle: ~1 ms corresponds to 0°, ~2 ms to
    180°.  This helper sets the frequency and returns a PWM instance.
    """
    pwm = PWM(Pin(pin_num))
    pwm.freq(50)
    return pwm


def angle_to_duty(angle):
    """Convert a servo angle in degrees to a duty cycle ratio.

    The servo expects pulses between roughly 1 ms (0°) and 2 ms (180°)
    within a 20 ms period.  We map angle ∈ [0, 180] to a duty cycle
    fraction ∈ [0.05, 0.10].

    Parameters
    ----------
    angle : float
        The desired servo angle in degrees.

    Returns
    -------
    float
        Duty cycle ratio between 0 and 1.
    """
    angle = max(0, min(180, angle))
    pulse_min = 1.0  # ms
    pulse_max = 2.0  # ms
    pulse = pulse_min + (pulse_max - pulse_min) * (angle / 180.0)
    duty = pulse / 20.0  # 20 ms period → 50 Hz
    return duty


def set_servo_angle(pwm, angle):
    """Set the servo to the specified angle using a PWM object."""
    duty = angle_to_duty(angle)
    pwm.duty_u16(int(duty * 65535))


def scale_field_to_color(value, max_field=20000.0):
    """Convert a magnetic field component to an 8‑bit color value.

    This function takes a magnetic field component in microteslas and
    scales its absolute value to the range 0–255 based on the
    provided maximum field.  Values above `max_field` are clipped.
    """
    v = abs(value)
    if v > max_field:
        v = max_field
    return int((v / max_field) * 255)


def main():
    # Initialize I2C bus for the TLV493D sensor
    i2c = I2C(0, scl=Pin(SCL_PIN), sda=Pin(SDA_PIN), freq=400_000)
    sensor = TLV493D(i2c)

    # Initialize servo
    servo_pwm = setup_servo(SERVO_PIN)

    # Initialize LED ring
    pixels = neopixel.NeoPixel(Pin(LED_PIN), NUM_PIXELS)

    # Main loop
    while True:
        # Read magnetic field vector (µT)
        try:
            x, y, z = sensor.read_magnetic()
        except OSError:
            # I2C read can fail if the bus is busy; retry next iteration
            time.sleep_ms(10)
            continue

        # Map X component to servo angle.  Use ±16000 µT as the full
        # range.  Clip to ±16000 µT and map to 0–180°.
        max_uT = 16000.0
        # Clip x to ±max_uT
        x_clipped = max(-max_uT, min(max_uT, x))
        # Normalize to [0,1] then map to 0–180
        angle = (x_clipped + max_uT) / (2 * max_uT) * 180.0
        set_servo_angle(servo_pwm, angle)

        # Convert magnetic field magnitude to color.  Use the absolute
        # values for R, G, B components to illustrate field direction.
        r = scale_field_to_color(x, max_field=max_uT)
        g = scale_field_to_color(y, max_field=max_uT)
        b = scale_field_to_color(z, max_field=max_uT)
        color = (r, g, b)

        # Update all pixels to the computed color
        for i in range(NUM_PIXELS):
            pixels[i] = color
        pixels.write()

        # Small delay to avoid overwhelming the I²C bus and neopixel
        time.sleep(0.05)


# Only run main if this file is executed directly
if __name__ == "__main__":
    main()