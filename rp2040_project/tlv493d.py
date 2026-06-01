"""
MicroPython driver for the TLV493D triple‑axis magnetometer.

This module provides a simple, dependency‑free driver that can be used
on the RP2040 (or any MicroPython board) to read three‑dimensional
magnetic field data from the Infineon TLV493D sensor.  The sensor is
capable of measuring magnetic fields along the X, Y and Z axes with
12‑bit resolution and extremely low power consumption (around
10 µA)【796762022328058†L26-L46】.  It communicates over an I²C
interface and supports data rates up to 1 Mbit/s【796762022328058†L26-L46】.

This driver is derived from the decoding logic used in the Adafruit
CircuitPython library but written in pure MicroPython so it can run
without external dependencies.  It exposes a single method to read
the magnetic field vector in microteslas.

Example:

    from machine import I2C, Pin
    from tlv493d import TLV493D

    # initialise I2C on pins GP0 (SDA) and GP1 (SCL)
    i2c = I2C(0, scl=Pin(1), sda=Pin(0), freq=400_000)
    sensor = TLV493D(i2c)
    x, y, z = sensor.read_magnetic()
    print("Magnetic field:", x, y, z)

"""

import struct


class TLV493D:
    """Minimal TLV493D magnetometer driver.

    Parameters
    ----------
    i2c : machine.I2C
        An initialized I2C bus object.
    address : int
        The I2C address of the sensor.  Defaults to 0x5E.

    Notes
    -----
    The TLV493D returns 10 bytes of data when performing a multi‑byte
    read starting from register 0.  The raw data must be unpacked
    into 12‑bit signed values for each axis.  The algorithm below
    follows the reference implementation from the Adafruit
    CircuitPython library【160945116428272†L169-L181】.
    """

    # Default 7‑bit I2C address (0b1011110)
    DEFAULT_ADDR = 0x5E

    def __init__(self, i2c, address=DEFAULT_ADDR):
        self.i2c = i2c
        self.address = address

        # initial measurement configuration: normal mode (no temperature)
        # configure control registers: default values for normal measurement
        try:
            # write 0x01 to register 0x10 (Master Control)
            self.i2c.writeto_mem(self.address, 0x10, b"\x01")
        except Exception:
            # if write fails (e.g. bus busy), just ignore. The sensor
            # starts up in a known measurement mode anyway.
            pass

    def _read_raw(self):
        """Read the 10‑byte raw data frame from the sensor.

        Returns
        -------
        bytes
            A length‑10 bytes object containing the sensor output.
        """
        # The sensor auto‑increments the register pointer when reading
        # from address 0x00.  Reading 10 bytes returns the complete
        # measurement frame (6 data bytes + 4 status/config bytes).
        return self.i2c.readfrom_mem(self.address, 0x00, 10)

    @staticmethod
    def _unpack_12bit(value, extra_bits):
        """Combine 8‑bit and 4‑bit values into a signed 12‑bit integer.

        Parameters
        ----------
        value : int
            The 8‑bit base value.
        extra_bits : int
            The upper 4 bits for the 12‑bit result.

        Returns
        -------
        int
            A signed integer from –2048 to +2047.
        """
        raw = ((extra_bits & 0xF) << 8) | value
        # Convert to signed 12‑bit integer
        if raw & 0x800:  # sign bit
            raw -= 0x1000
        return raw

    def read_magnetic(self):
        """Read the magnetic field vector.

        Returns
        -------
        tuple(float, float, float)
            A 3‑tuple containing the X, Y and Z components of the
            magnetic field in microteslas (µT).
        """
        data = self._read_raw()
        # Extract bytes as unsigned ints
        bx1, by1, bz1, bx2, by2, bz2, temp_lo, temp_hi, _, _ = data

        # Each axis is encoded as 12 bits spread across two bytes.  The
        # lower 8 bits come from the first byte (bx1/by1/bz1) and the
        # upper 4 bits come from the high nibble of bx2/by2/bz2.
        bx = self._unpack_12bit(bx1, bx2 >> 4)
        by = self._unpack_12bit(by1, by2 >> 4)
        bz = self._unpack_12bit(bz1, bz2 >> 4)

        # According to the datasheet and reference implementation, the
        # raw 12‑bit values must be multiplied by 98.0 to obtain a
        # measurement in microteslas【160945116428272†L169-L181】.
        scale = 98.0
        return (bx * scale, by * scale, bz * scale)