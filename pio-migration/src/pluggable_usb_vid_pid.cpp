// Override PluggableUSBD() singleton so the device enumerates as SpaceMouse Pro Wireless
// (VID=0x256f, PID=0xc631) instead of the Pico's default Mbed VID/PID (0x2e8a/0x00c0).
//
// The pins_arduino.h for the Pico hard-codes BOARD_VENDORID=0x2e8a, overriding any -D flag.
// Providing PluggableUSBD() here in a .o file takes link-time priority over the version in
// libFrameworkArduino.a, which is the only reliable way to swap VID/PID on this platform.
//
// Arduino.h must be included before the #if so that DEVICE_USBDEVICE gets defined through
// the Mbed target headers before the guard is evaluated.
#include "Arduino.h"
#if defined(TARGET_RP2040) && defined(DEVICE_USBDEVICE) && defined(SERIAL_CDC)
#include "USB/PluggableUSBDevice.h"

arduino::PluggableUSBDevice& PluggableUSBD()
{
    static arduino::PluggableUSBDevice obj(0x256f, 0xc631);
    return obj;
}
#endif
