#include "HidDevice.h"

#include <Arduino.h>
#include <string.h>

#if defined(ARDUINO_ARCH_AVR)
#include "HID.h"
#elif defined(TARGET_RP2040)
#include "PluggableUSBHID.h"
#elif defined(ARDUINO_ARCH_ESP32)
#if !defined(ARDUINO_USB_MODE) || ARDUINO_USB_MODE != 0
#error "ESP32-S3 native USB HID requires ARDUINO_USB_MODE=0"
#endif
#include "USB.h"
#include "USBHID.h"
#else
#error "This PlatformIO migration currently supports AVR, RP2040 Mbed, and ESP32-S3 Arduino cores."
#endif

static const uint8_t hidReportDescriptor[] PROGMEM = {
    0x05, 0x01,
    0x09, 0x08,
    0xa1, 0x01,
    0xa1, 0x00,
    0x85, 0x01,
    0x16, 0x00, 0x80,
    0x26, 0xff, 0x7f,
    0x36, 0x00, 0x80,
    0x46, 0xff, 0x7f,
    0x09, 0x30,
    0x09, 0x31,
    0x09, 0x32,
    0x75, 0x10,
    0x95, 0x03,
    0x81, 0x02,
    0xC0,
    0xa1, 0x00,
    0x85, 0x02,
    0x16, 0x00, 0x80,
    0x26, 0xff, 0x7f,
    0x36, 0x00, 0x80,
    0x46, 0xff, 0x7f,
    0x09, 0x33,
    0x09, 0x34,
    0x09, 0x35,
    0x75, 0x10,
    0x95, 0x03,
    0x81, 0x02,
    0xC0,
    0xa1, 0x00,
    0x85, 0x03,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 32,
    0x05, 0x09,
    0x19, 1,
    0x29, 32,
    0x81, 0x02,
    0xC0,
    0xC0};

#if defined(TARGET_RP2040)
class SpaceMouseHID : public arduino::USBHID
{
public:
  SpaceMouseHID() : arduino::USBHID(false, 0, 0, 0x256f, 0xc631, 0x0001) {}

  bool sendReport(uint8_t reportId, const uint8_t *data, uint8_t len)
  {
    HID_REPORT report;
    report.length = len + 1;
    report.data[0] = reportId;
    memcpy(&report.data[1], data, len);
    return send(&report);
  }

protected:
  const uint8_t *report_desc() override
  {
    return hidReportDescriptor;
  }

  uint16_t report_desc_length() override
  {
    return sizeof(hidReportDescriptor);
  }
};

static SpaceMouseHID spaceMouseUsbHid;
#elif defined(ARDUINO_ARCH_ESP32)
static USBHID HID;

class SpaceMouseHIDDevice : public USBHIDDevice
{
public:
  SpaceMouseHIDDevice()
  {
    static bool initialized = false;
    if (!initialized)
    {
      initialized = true;
      HID.addDevice(this, sizeof(hidReportDescriptor));
    }
  }

  void begin()
  {
    HID.begin();
  }

  uint16_t _onGetDescriptor(uint8_t *buffer) override
  {
    memcpy(buffer, hidReportDescriptor, sizeof(hidReportDescriptor));
    return sizeof(hidReportDescriptor);
  }

  bool sendReport(uint8_t reportId, const uint8_t *data, uint8_t len)
  {
    return HID.SendReport(reportId, data, len);
  }
};

static SpaceMouseHIDDevice spaceMouseUsbHid;
#endif

void setupSpaceMouseHid()
{
#if defined(ARDUINO_ARCH_AVR)
  static HIDSubDescriptor node(hidReportDescriptor, sizeof(hidReportDescriptor));
  HID().AppendDescriptor(&node);
#elif defined(TARGET_RP2040)
  PluggableUSBD().begin();
#elif defined(ARDUINO_ARCH_ESP32)
  spaceMouseUsbHid.begin();
  USB.begin();
#endif
}

void sendHidReport(uint8_t reportId, const uint8_t *data, uint8_t len)
{
#if defined(ARDUINO_ARCH_AVR)
  HID().SendReport(reportId, data, len);
#else
  spaceMouseUsbHid.sendReport(reportId, data, len);
#endif
}

