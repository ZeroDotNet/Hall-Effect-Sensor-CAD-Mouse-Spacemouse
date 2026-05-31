// This code is the combination of multiple works by others:
// 1. Original code for the Space Mushroom by Shiura on Thingiverse: https://www.thingiverse.com/thing:5739462
//    The next two from the comments on the instructables page: https://www.instructables.com/Space-Mushroom-Full-6-DOFs-Controller-for-CAD-Appl/
//    and the comments of Thingiverse: https://www.thingiverse.com/thing:5739462/comments
// 2. Code to emulate a 3DConnexion Space Mouse by jfedor: https://pastebin.com/gQxUrScV
// 3. This code was then remixed by BennyBWalker to include the above two sketches: https://pastebin.com/erhTgRBH
// 4. Four joystick remix code by fdmakara: https://www.thingiverse.com/thing:5817728
// The work by Teaching Tech involves mixing all of these. The basis is fdmakara's four joystick movement logic, with jfedor/BennyBWalker's HID SpaceMouse emulation.
// The four joystick logic sketch was setup for the joystick library instead of HID, so elements of this were omitted where not needed.
// The outputs were jumbled no matter how He plugged them in, so I spent a lot of time adding debugging code to track exactly what was happening.
// On top of this, He has added more control of speed/direction and comments/links to informative resources to try and explain what is happening in each phase.

// Spacemouse emulation
// Teaching Tech followed the instructions here from nebhead: https://gist.github.com/nebhead/c92da8f1a8b476f7c36c032a0ac2592a
// with two key differences:
// 1. He changed the word 'DaemonBite' to 'Spacemouse' in all references. 2. He changed the VID and PID values as per jfedor's instructions: vid=0x256f, pid=0xc631 (SpaceMouse Pro Wireless (cabled))
// When compiling and uploading, He select Arduino AVR boards (in Sketchbook) > Spacemouse and then the serial port.
// You will also need to download and install the 3DConnexion software: https://3dconnexion.com/us/drivers-application/3dxware-10/
// If all goes well, the 3DConnexion software will show a SpaceMouse Pro wireless when the Arduino is connected.
// *JC - John Crombie - I'd like to add a big thank you to Teaching Tech for this code. If it wasn't for this I wouldn't
// have been tempted to try to implement a Hall Effect sensor version. All my changes will be marked with a *JC comment
// All debug Serial.print statements have had the joystick output text has been renamed as HESx e.g. AX is now HSE0
/**********************************************
 * change log
 * C001 - 18-Jul-24 - Reversed default direction of Z rotation
 * C002 - 22-Jul-24 - Added state machine code to handle pressing two buttons at once to cause a third action and supress
 *                    suppress the default action of the two buttons pressed. Before I didn't supress the actions
 * C003 - 25-Jul-24 - Added define for movement3DC to switch between default 3DConnexion axis movement and Teaching Techs default movement
 * C004 - 04-Aug-24 - bug fix - Changed key reporting so that a zero report is sent when the final key is released.
 *                    Changed the place where duplicate keys reports are supressed. Used to be in the key rutine now in the report routine
 * C006 - 08-Aug-24 - bug fix - After logical button was pressed, all buttons were being sent in state 4. Now corrected.
 * C007 - 07-Aug-25 - Adding two more pseudo buttons. Achieved by pressing front button at the same time as one of the side buttons.
 *                    These give TAB/Rotate lock (Left and fromt button) and Fit to screen (Right and front button) by default
 * C008 - 09-Aug-25 - Remove Speed adjustment left over from TT code - This can be controlled through 3DConections configuration menu.
 * C009 - 12-Aug-25 - Changed centre button to cycle through three views if enabled with cycleButton being true.
 ************************************************/

#include <Arduino.h>
#include <math.h>
#include "movement_math.h"
#include "status_led.h"

#define IO_USERNAME "zerodotnet"
#define IO_KEY "aio_tMPN11gyavXaOs0C52NxIiOqick1"
#ifndef ENABLE_LEDS
#define ENABLE_LEDS 0
#endif

#if !defined(SENSOR_BACKEND_HALL) && !defined(SENSOR_BACKEND_TLV493D)
#define SENSOR_BACKEND_HALL 1
#endif

#if defined(SENSOR_BACKEND_HALL) && defined(SENSOR_BACKEND_TLV493D)
#undef SENSOR_BACKEND_HALL
#endif

#if defined(SENSOR_BACKEND_TLV493D)
#if !defined(TARGET_RP2040)
#error "The TLV493D backend is currently wired for RP2040 builds."
#endif
#include <Wire.h>
#include "TLx493D_inc.hpp"
using namespace ifx::tlx493d;
#ifndef TLV493D_SENSOR_COUNT
#define TLV493D_SENSOR_COUNT 3
#endif
#if TLV493D_SENSOR_COUNT != 1 && TLV493D_SENSOR_COUNT != 3
#error "TLV493D_SENSOR_COUNT must be 1 or 3."
#endif
#ifndef I2C0_SDA
#define I2C0_SDA 4
#endif
#ifndef I2C0_SCL
#define I2C0_SCL 5
#endif
#ifndef I2C1_SDA
#define I2C1_SDA 2
#endif
#ifndef I2C1_SCL
#define I2C1_SCL 3
#endif
#ifndef TLV493D_SCALE
#define TLV493D_SCALE 10.0f
#endif
#ifndef TLV493D_DEADZONE
#define TLV493D_DEADZONE 0.4f
#endif
#endif

#if defined(ARDUINO_ARCH_AVR)
// Include inbuilt Arduino HID library by NicoHood: https://github.com/NicoHood/HID
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

// Debugging
// 0: Debugging off. Set to this once everything is working.
// 1: *JC Output raw Hall Effect Sensor values. 176 - 862 raw ADC 10-bit values. 5V ADC reference
// 2: *JC Output centered Hall Effect Sensor values. Values should be approx -424 to +424, jitter around 0 at idle. ADC reference 2.56v
// 3: *JC Output centered Hall Effect Sensor values. Filtered for deadzone. Approx -424+DEADZONE to +424-DEADZONE, locked to zero at idle. Also button values. ADC reference 2.56v
// 4: Output translation and rotation values. Approx -500 to +500 depending on the parameter. *JC ADC reference 2.56v
// 5: Output debug 3 and 4 side by side for direct cause and effect reference. *JC ADC reference 2.56v
// 6: *JC Output debug info for pseudo key state machine. ( two keys pressed at once to simulate another key press)
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif
#ifndef DEBUG_TELEPLOT
#define DEBUG_TELEPLOT 0
#endif
#ifndef STATUS_LED_TEST_ONLY
#define STATUS_LED_TEST_ONLY 0
#endif
#ifndef STATUS_LED_TEST_RGB_CYCLE
#define STATUS_LED_TEST_RGB_CYCLE 0
#endif
#ifndef STATUS_LED_TEST_RGB_PIN_SWEEP
#define STATUS_LED_TEST_RGB_PIN_SWEEP 0
#endif
int debug = DEBUG_LEVEL;
bool debug1SameLine = DEBUG_TELEPLOT == 0; // Teleplot forwarding needs one sample per line.

// Choose between 3DConnexion default movement or Teaching Tech's
// With 3DConnexion you push the joystick away from you to zoom out and towards you to zoom in.
// lifting up the joystick moves up and pushing down moves down.
// With the Teaching Tech default, these two axis are swapped so that pulling up or pushing down the knob controls zoom
// and pushing away or pulling it towards you controls up and down. I prefer this.
// set to true for 3DConnection movement.
bool movement3DC = true;

// switch between two modes of operation. The original mapping of buttons including pushing two at once or an alternative mapping where
// the front button pretends to be three different buttons mapping to three views.
bool cycleButton = false;
uint8_t cycleInitialButton = 0; // Added to lowest pseudo button value
const uint8_t buttonDelay = 20; // 20ms wait time for second button to be pressed

// Direction
// Modify the direction of translation/rotation depending on preference. This can also be done per application in the 3DConnexion software.
// Switch between true/false as desired.
bool invX = false;  // pan left/right
bool invY = false;  // Zoom in/out or pan up/down // C003 *JC - 3DC default movement or TT default
bool invZ = true;   // pan up/down or zoom in/out // C003 *JC - 3DC default movement or TT default
bool invRX = false; // Rotate around X axis (tilt front/back)
bool invRY = false; // Rotate around Y axis (tilt left/right)
bool invRZ = false; // Rotate around Z axis (twist left/right)

// Speed
// Modify to change sensitibity/speed. Default and maximum 100. Works like a percentage ie. 50 is half as fast as default. This can also be done per application in the 3DConnexion software.
// int16_t speed = 80; C008 - remove this as it can be controlled through 3dConnection software

// Default Assembly when looking from above: *JC modified for Hall Effect Sensors (HES)
//      7 6          Y+
//       |           .
// 8 9 --+--2 3 X-...Z+...X+
//       |           .
//      0 1          Y-
//
// Wiring. Matches the first eight analogue pins of the Arduino Pro Micro (atmega32u4)
// Other environments override these through platformio.ini build flags.
#ifndef HES_PIN_0
#define HES_PIN_0 A0
#endif
#ifndef HES_PIN_1
#define HES_PIN_1 A1
#endif
#ifndef HES_PIN_2
#define HES_PIN_2 A2
#endif
#ifndef HES_PIN_3
#define HES_PIN_3 A3
#endif
#ifndef HES_PIN_6
#define HES_PIN_6 A6
#endif
#ifndef HES_PIN_7
#define HES_PIN_7 A7
#endif
#ifndef HES_PIN_8
#define HES_PIN_8 A8
#endif
#ifndef HES_PIN_9
#define HES_PIN_9 A9
#endif
#ifndef BTN_PIN_0
#define BTN_PIN_0 0
#endif
#ifndef BTN_PIN_1
#define BTN_PIN_1 1
#endif
#ifndef BTN_PIN_2
#define BTN_PIN_2 2
#endif
#ifndef BUTTON_COUNT
#define BUTTON_COUNT 3
#endif
#if BUTTON_COUNT < 0 || BUTTON_COUNT > 3
#error "BUTTON_COUNT must be between 0 and 3."
#endif

int PINLIST[8] = {
    // The positions of the reads *JC comments indicate which Hall Effect sensor is connected
    HES_PIN_0, // HES 6 o'clock left
    HES_PIN_1, // HES 6 o'clock right
    HES_PIN_2, // HES 3 o'clock near
    HES_PIN_3, // HES 3 o'clock far
    HES_PIN_6, // HES 12 o'clock right
    HES_PIN_7, // HES 12 o'clock left
    HES_PIN_8, // HES 9 o'clock far
    HES_PIN_9  // HES 9 o'clock near
};

// *JC added button list for digital inputs. BUTTON_COUNT can temporarily disable
// the button logic without leaving out-of-bounds accesses behind.
int BTNLIST[3] = {
    BTN_PIN_0,
    BTN_PIN_1,
    BTN_PIN_2};

// Deadzone to filter out unintended movements.
// Increase if the mouse has small movements when it should be idle or the mouse is too senstive to subtle movements.
// Note that the 3d Connections also has its own deadzone processes
int DEADZONE = 40;

// This portion sets up the communication with the 3DConnexion software. The communication protocol is created here.
// hidReportDescriptor webpage can be found here: https://eleccelerator.com/tutorial-about-usb-hid-report-descriptors/
static const uint8_t _hidReportDescriptor[] PROGMEM = {
    0x05, 0x01,       //  Usage Page (Generic Desktop)
    0x09, 0x08,       //  0x08: Usage (Multi-Axis)
    0xa1, 0x01,       //  Collection (Application)
    0xa1, 0x00,       // Collection (Physical)
    0x85, 0x01,       //  Report ID
    0x16, 0x00, 0x80, // logical minimum (-500)
    0x26, 0xff, 0x7f, // logical maximum (500)
    0x36, 0x00, 0x80, // Physical Minimum (-32768)
    0x46, 0xff, 0x7f, // Physical Maximum (32767)
    0x09, 0x30,       //    Usage (X)
    0x09, 0x31,       //    Usage (Y)
    0x09, 0x32,       //    Usage (Z)
    0x75, 0x10,       //    Report Size (16)
    0x95, 0x03,       //    Report Count (3)
    0x81, 0x02,       //    Input (variable,absolute)
    0xC0,             //  End Collection
    0xa1, 0x00,       // Collection (Physical)
    0x85, 0x02,       //  Report ID
    0x16, 0x00, 0x80, // logical minimum (-500)
    0x26, 0xff, 0x7f, // logical maximum (500)
    0x36, 0x00, 0x80, // Physical Minimum (-32768)
    0x46, 0xff, 0x7f, // Physical Maximum (32767)
    0x09, 0x33,       //    Usage (RX)
    0x09, 0x34,       //    Usage (RY)
    0x09, 0x35,       //    Usage (RZ)
    0x75, 0x10,       //    Report Size (16)
    0x95, 0x03,       //    Report Count (3)
    0x81, 0x02,       //    Input (variable,absolute)
    0xC0,             //  End Collection

    0xa1, 0x00, // Collection (Physical)
    0x85, 0x03, //  Report ID
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //    Logical Maximum (1)
    0x75, 0x01, //    Report Size (1)
    0x95, 32,   //    Report Count (24) // *JC - I dont undwerstand what the comment says 24 but gives a value of 32
    0x05, 0x09, //    Usage Page (Button)
    0x19, 1,    //    Usage Minimum (Button #1)
    0x29, 32,   //    Usage Maximum (Button #24) // *JC - same comment as above
    0x81, 0x02, //    Input (variable,absolute)
    0xC0,
    0xC0};

#if defined(TARGET_RP2040)
class SpaceMouseHID : public arduino::USBHID
{
public:
  // output_report_length = max bytes the device SENDS (Mbed naming: output = device→host).
  // Our largest report is 7 bytes (1 reportId + 6 data). input_report_length = host→device,
  // which we don't use.
  SpaceMouseHID() : arduino::USBHID(false, 8, 0, 0x256f, 0xc631, 0x0001) {}

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
    return _hidReportDescriptor;
  }

  uint16_t report_desc_length() override
  {
    return sizeof(_hidReportDescriptor);
  }
};

static SpaceMouseHID SpaceMouseUsbHid;
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
      HID.addDevice(this, sizeof(_hidReportDescriptor));
    }
  }

  void begin()
  {
    HID.begin();
  }

  uint16_t _onGetDescriptor(uint8_t *buffer) override
  {
    memcpy(buffer, _hidReportDescriptor, sizeof(_hidReportDescriptor));
    return sizeof(_hidReportDescriptor);
  }

  bool sendReport(uint8_t reportId, const uint8_t *data, uint8_t len)
  {
    return HID.SendReport(reportId, data, len);
  }
};

static SpaceMouseHIDDevice SpaceMouseUsbHid;
#endif

void setupSpaceMouseHid()
{
#if defined(ARDUINO_ARCH_AVR)
  static HIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor));
  HID().AppendDescriptor(&node);
#elif defined(TARGET_RP2040)
  PluggableUSBD().begin();
  SpaceMouseUsbHid.wait_ready(); // block until host enumerates the device
#elif defined(ARDUINO_ARCH_ESP32)
  SpaceMouseUsbHid.begin();
  USB.begin();
#endif
}

void sendHidReport(uint8_t reportId, const uint8_t *data, uint8_t len)
{
#if defined(ARDUINO_ARCH_AVR)
  HID().SendReport(reportId, data, len);
#else
  SpaceMouseUsbHid.sendReport(reportId, data, len);
#endif
}

void configureAnalogReference()
{
#if defined(ARDUINO_ARCH_AVR)
  //*JC - reduce ADC reference voltage from 5V to 2.56 if not using debug = 1
  if (debug == 1)
  {
    analogReference(DEFAULT);
  }
  else
  {
    analogReference(INTERNAL);
  }
#endif
}

// Sensors are matched to pin order.
// *JC - Note HES0 and BTN0 are not the same pin. HSE0 is Analog input 0 and BTN0 is digital input 0
#define HES0 0
#define HES1 1
#define HES2 2
#define HES3 3
#define HES6 4
#define HES7 5
#define HES8 6
#define HES9 7
#define BTN0 0
#define BTN1 1
#define BTN2 2

// Set any Hall sensor to false to report it as zero and remove its contribution
// from centered values and movement calculations.
bool hallSensorEnabled[8] = {
    true, // HES0
    true, // HES1
    true, // HES2
    true, // HES3
    true, // HES6
    true, // HES7
    true, // HES8
    true  // HES9
};

#if defined(SENSOR_BACKEND_TLV493D)
static arduino::MbedI2C TlvWire0(I2C0_SDA, I2C0_SCL);
TLx493D_A1B6 tlvSensor1(TlvWire0, TLx493D_IIC_ADDR_A0_e);
#if TLV493D_SENSOR_COUNT == 3
static arduino::MbedI2C TlvWire1(I2C1_SDA, I2C1_SCL);
TLx493D_A1B6 tlvSensor2(TlvWire1, TLx493D_IIC_ADDR_A1_e);
TLx493D_A1B6 tlvSensor3(TlvWire1, TLx493D_IIC_ADDR_A0_e);
#endif
float tlvCenterPoints[TLV493D_SENSOR_COUNT * 3];
#else
// Centerpoint variable to be populated during setup routine.
int centerPoints[8];
#endif

// Function to read and store analogue voltages for each joystick axis.
#if defined(SENSOR_BACKEND_HALL)
void readAllFromSensors(int *rawReads)
{
  for (int i = 0; i < 8; i++)
  {
    if (hallSensorEnabled[i])
    {
      rawReads[i] = analogRead(PINLIST[i]);
    }
    else
    {
      rawReads[i] = 0;
    }
  }
}
#else
bool setupTlv493dSensors()
{
  bool ok = tlvSensor1.begin();
#if TLV493D_SENSOR_COUNT == 3
  ok = tlvSensor2.begin() && ok;
  ok = tlvSensor3.begin() && ok;
#endif

  if (!ok)
  {
    Serial.println("TLV493D init failed; check I2C pins, address, power, and sensor count build flag.");
  }
  return ok;
}

bool readTlvSensor(TLx493D_A1B6 &sensor, float *readings, const float *fallbackReadings = nullptr)
{
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  const bool ok = sensor.getMagneticField(&x, &y, &z);
  if (ok)
  {
    readings[0] = static_cast<float>(x);
    readings[1] = static_cast<float>(y);
    readings[2] = static_cast<float>(z);
  }
  else if (fallbackReadings != nullptr)
  {
    readings[0] = fallbackReadings[0];
    readings[1] = fallbackReadings[1];
    readings[2] = fallbackReadings[2];
  }
  else
  {
    readings[0] = 0.0f;
    readings[1] = 0.0f;
    readings[2] = 0.0f;
  }
  return ok;
}

bool readAllFromSensors(float *rawReads, const float *fallbackReads = nullptr)
{
  bool ok = readTlvSensor(tlvSensor1, &rawReads[0], fallbackReads == nullptr ? nullptr : &fallbackReads[0]);
#if TLV493D_SENSOR_COUNT == 3
  ok = readTlvSensor(tlvSensor2, &rawReads[3], fallbackReads == nullptr ? nullptr : &fallbackReads[3]) && ok;
  ok = readTlvSensor(tlvSensor3, &rawReads[6], fallbackReads == nullptr ? nullptr : &fallbackReads[6]) && ok;
#endif
  return ok;
}

void printTlvReadings(const float *readings)
{
  for (int i = 0; i < TLV493D_SENSOR_COUNT; i++)
  {
    if (i > 0)
    {
      Serial.print(",");
    }
    Serial.print("TLV");
    Serial.print(i + 1);
    Serial.print("X:");
    Serial.print(readings[i * 3 + 0], 3);
    Serial.print(",TLV");
    Serial.print(i + 1);
    Serial.print("Y:");
    Serial.print(readings[i * 3 + 1], 3);
    Serial.print(",TLV");
    Serial.print(i + 1);
    Serial.print("Z:");
    Serial.print(readings[i * 3 + 2], 3);
  }
}
#endif

void printButtonReadings(const uint8_t *buttonReads, bool endLine)
{
  Serial.print(",");
  Serial.print("But0:");
  Serial.print(buttonReads[0]);
  Serial.print(",");
  Serial.print("But1:");
  Serial.print(buttonReads[1]);
  Serial.print(",");
  Serial.print("But2:");
  Serial.print(buttonReads[2]);
  Serial.print(",");
  Serial.print("But3:");
  if (endLine)
  {
    Serial.println(buttonReads[3]);
  }
  else
  {
    Serial.print(buttonReads[3]);
  }
}

void printMovementReadings(const Movement6D &movement)
{
  Serial.print("TX:");
  Serial.print(movement.tx);
  Serial.print(",");
  Serial.print("TY:");
  Serial.print(movement.ty);
  Serial.print(",");
  Serial.print("TZ:");
  Serial.print(movement.tz);
  Serial.print(",");
  Serial.print("RX:");
  Serial.print(movement.rx);
  Serial.print(",");
  Serial.print("RY:");
  Serial.print(movement.ry);
  Serial.print(",");
  Serial.print("RZ:");
  Serial.print(movement.rz);
}

// *JC Function to read and store button values
// When pressing two buttons at once for a different function, one button is usually pressed slightly before the other.
// To prevent the first buttons function being triggered, we wait 15ms to see if another button is pressed in the meantime.
// if so we send the pseudo button value. if not we send the first button value.
// keyState 0 - no button pressed
// keyState 1 - 1 or 3 pressed
// keystate 2 - 1&3 pressed within time limit
// keystate 3 = 1&3 not pressed within time limit
// keyState 4 = Wait until physical buttons released to reset state.
// C007 changed logic from above now waits for any two buttons to be pressed and keyState 5 is called when button 1 & 2 are pressed together and keyState 6 when buttons 2 & 3 are pressed
unsigned long keyTimeNew, keyTimeOld = 0;
uint8_t keyState = 0, keyPressed = 0; // C004 - *JC - keyPresed added to keep track of last key pressed (in state machine).
// uint8_t oldButtonValues[6] = {0,0,0,0}; no longer used with state machine

void readAllFromButtons(uint8_t *buttonValues)
{
  for (int i = 0; i < 9; i++)
  {
    buttonValues[i] = false;
  }

#if BUTTON_COUNT == 0
  keyState = 0;
  keyPressed = 0;
  return;
#endif

  for (int i = 1; i < 4; i++)
  { // read real button values
    buttonValues[i] = (i <= BUTTON_COUNT) ? !digitalRead(BTNLIST[i - 1]) : false;
  }

  // C002 - *JC changed logic for handling pseudo/logical switch (two buttons pressed at once gives different function)
  // C007 - *JC added entries 4 and 5 for new pseudo buttons. 0 is the existing one.
  // C009 - *JC if CycleButton is set to true then middle button (2) will set pseudo buttons 6, 7 and 8
  keyTimeNew = millis();
  switch (keyState)
  {
  case 0: // no button pressed so far
    if (buttonValues[1] || buttonValues[3] || buttonValues[2])
    { // C007 - *JC - added buttonValues[2]
      if (debug == 6)
        Serial.println("keyState 0 - button pressed move to keyState 1");
      keyState = 1;
      keyTimeOld = keyTimeNew;
      buttonValues[1] = buttonValues[3] = buttonValues[2] = false; // don't send button values yet. C007 - *JC added ButtonValues 2 to the list
    }
    break;

  case 1: // button 1 or 3 pressed - what has happened with the elapsed time
    if (debug == 6)
      Serial.println("keyState 1 - one button pressed");
    if (keyTimeNew - keyTimeOld > buttonDelay)
    {               // C007  - changed the waiting time from 15 to 20 as 15 seemed to short for ackward double button presses C009 changed number to a constant defined elsewhere
      keyState = 3; // second button not pressed
    }
    else if (buttonValues[1] && buttonValues[3])
    {
      keyState = 2; // second button pressed pseudo button 1
    }
    else if (buttonValues[1] && buttonValues[2])
    {               // start of C007 changes - this introduces 2 new states for the two new pseudo buttons
      keyState = 5; // second button pressed pseudo button 2
    }
    else if (buttonValues[2] && buttonValues[3])
    {
      keyState = 6; // second button pressed pseudo button 3
    } // end of C007 changes

    buttonValues[1] = buttonValues[3] = buttonValues[2] = false; // don't send button values yet - C007 - *JC added buttonValues[2] to the list
    break;

  case 2: // second button pressed - set logical button
    if (debug == 6)
      Serial.println("keyState 2 - second button pressed - set logical button");
    buttonValues[0] = true;
    keyState = 4;
    keyPressed = 0; // C004 - *JC - record button 0 pressed
    buttonValues[1] = buttonValues[3] = false;
    break;

  case 3: // second button not pressed, send the original button
    if (debug == 6)
      Serial.println("keyState 3 - second button not pressed in time");
    keyState = 4;
    if (buttonValues[1])
    { // C004 - *JC - record which button was pressed and will be reported
      keyPressed = 1;
    }
    else if (buttonValues[2])
    { // C007 - *JC - added extra button to possible two button presses
      // C009 - *JC - if the flag cycleButton is set to true then button 2 will set one of three pseudo buttons
      //              that will then be used to display one of three views on rotation
      if (cycleButton)
      {
        buttonValues[6 + cycleInitialButton] = true;
        keyPressed = 6 + cycleInitialButton;
        cycleInitialButton = (cycleInitialButton + 1) % 3;
        if (debug == 6)
        {
          Serial.print("cycleInitialButton = ");
          Serial.println(keyPressed);
        }
      }
      else
      {
        keyPressed = 2;
      }
    }
    else
    {
      keyPressed = 3;
    }
    break;

  case 4: // wait until buttons released to reset state
    // if (debug == 6) Serial.println("keyState 4 - wait for buttons to be released before resetting state");

    if (!buttonValues[1] && !buttonValues[3] && !buttonValues[2])
    { // C007 - *JC added buttonValues[2]
      keyState = 0;
    }
    buttonValues[0] = buttonValues[1] = buttonValues[3] = buttonValues[2] = false; // C005 - *JC - bug fix. Was here before but was removed for the last release
    buttonValues[keyPressed] = true;                                               // C004 - *JC - keep the keys pressed.

    break;

  case 5: // C007 -*JC - second pseudo button
    if (debug == 6)
      Serial.println("keyState 5 - second button pressed - set logical button");
    buttonValues[4] = true;
    keyState = 4;
    keyPressed = 4; // C004 - *JC - record button 0 pressed
    buttonValues[1] = buttonValues[2] = buttonValues[3] = false;
    break;

  case 6: // C007 - *JC - third pseudo button
    if (debug == 6)
      Serial.println("keyState 6 - second button pressed - set logical button");
    buttonValues[5] = true;
    keyState = 4;
    keyPressed = 5; // C004 - *JC - record button 0 pressed
    buttonValues[1] = buttonValues[2] = buttonValues[3] = false;
    break;
  }

  /* C004 - *JC - move supression of sending multiple key reports to report sending routine.
  // *JC - only send button value once
    for (int i=0;i<4;i++) {
      if (buttonValues[i] == oldButtonValues[i]) {
        buttonValues[i]=0; // send only once
      } else {
        if (debug == 6) {
          Serial.print("Button "); Serial.print(i); Serial.print(" changed - Old Value ");Serial.print(oldButtonValues[i]); Serial.print(" New Value ");Serial.println(buttonValues[i]);
        }
        oldButtonValues[i] = buttonValues[i];
      }
     }
  */
  //  based on real values set logical switch
  /*  Old code to handle logical button - doesn't supress first button pressed action
  if( buttonValues[1] && buttonValues[3]) {
    buttonValues[0] = true;
    buttonValues[1] = buttonValues[3] = false;
  }
  else
  {
    buttonValues[0] = false;
  }
  */
}

void setup()
{
  ledBegin();

#if STATUS_LED_TEST_ONLY
  Serial.begin(250000);
  delay(100);
  return;
#endif

  // HID protocol is set.  On RP2040 this blocks until the host enumerates the device.
  setupSpaceMouseHid();

  // Begin Serial for debugging
  Serial.begin(115200);
  delay(100);
  // *JC - setup button pins for digitalRead
  for (int i = 0; i < BUTTON_COUNT; i++)
  {
    pinMode(BTNLIST[i], INPUT_PULLUP);
  }
  configureAnalogReference();

#if defined(SENSOR_BACKEND_TLV493D)
  setupTlv493dSensors();
  readAllFromSensors(tlvCenterPoints);
  readAllFromSensors(tlvCenterPoints);
  readAllFromSensors(tlvCenterPoints);
#else
  // Read idle/centre positions for Sensors.
  // *JC - First read gives unpredictable values so do it twice
  readAllFromSensors(centerPoints);
  readAllFromSensors(centerPoints);
  readAllFromSensors(centerPoints);
#endif
}

uint8_t keyChange = 0; // C004 - *JC - variable to determine if new key report needs to be sent.
uint8_t lowByte16(int16_t value)
{
  return static_cast<uint8_t>(value & 0xFF);
}

uint8_t highByte16(int16_t value)
{
  return static_cast<uint8_t>((value >> 8) & 0xFF);
}

// Function to send translation and rotation data to the 3DConnexion software using the HID protocol outlined earlier. Two sets of data are sent: translation and then rotation.
// For each, a 16bit integer is split into two using bit shifting. The first is mangitude and the second is direction.
// *JC - Added button report
void send_command(int16_t rx, int16_t ry, int16_t rz, int16_t x, int16_t y, int16_t z, uint8_t *buttonValues)
{
  uint8_t trans[6] = {lowByte16(x), highByte16(x), lowByte16(y), highByte16(y), lowByte16(z), highByte16(z)};
  sendHidReport(1, trans, 6);
  uint8_t rot[6] = {lowByte16(rx), highByte16(rx), lowByte16(ry), highByte16(ry), lowByte16(rz), highByte16(rz)};
  sendHidReport(2, rot, 6);
  // *JC - Button Report
  // these are the button functions for first byte in Fusion 360. For other functions see the GitHub repositry
  //  bit 0 - bring up configuration dialog - logical button (press BTN0 and BTN2 at the same time) rotaee 45 degrees
  //  bit 1 - fit to screen
  //  bit 2 - plan view
  //  bit 3 - no function?
  //  bit 4 - right view hide
  //  bit 5 - front view File
  //  bit 6 - no function?
  //  bit 7 - no function?
  uint8_t btn[4] = {
      static_cast<uint8_t>(32 * buttonValues[3] + 16 * buttonValues[2] + 4 * buttonValues[1] + buttonValues[0] + 2 * buttonValues[5]),
      0,
      0,
      static_cast<uint8_t>(4 * buttonValues[4])}; // C007 added 2nd Pseudo button as Fit to Screen
  if (cycleButton)
  { // C009 use pseudo buttons to select views - button 2 controls which view is selected.
    btn[0] = 32 * buttonValues[6] + 16 * buttonValues[7] + 4 * buttonValues[8] + buttonValues[0] + 2 * buttonValues[1];
    btn[1] = buttonValues[4] + 16 * buttonValues[5];
    btn[3] = 4 * buttonValues[3];
  }
  if (buttonValues[0] + 2 * buttonValues[1] + 4 * buttonValues[2] + 8 * buttonValues[3] + 16 * buttonValues[4] + 32 * buttonValues[5] + 64 * buttonValues[6] + 128 * buttonValues[7] + 256 * buttonValues[8] != keyChange)
  { // C004 - *JC - changed operation *JC - only send report if a button is pressed C007 added new pseudo buttons to check
    if (debug == 6)
    {
      Serial.print("btn[0] = ");
      Serial.print(btn[0]);
      Serial.print(" btn[1] = ");
      Serial.print(btn[1]);
      Serial.print(" btn[2] = ");
      Serial.print(btn[2]);
      Serial.print(" btn[3] = ");
      Serial.println(btn[3]);
    }
    sendHidReport(3, btn, 4);
    keyChange = buttonValues[0] + 2 * buttonValues[1] + 4 * buttonValues[2] + 8 * buttonValues[3] + 16 * buttonValues[4] + 32 * buttonValues[5] + 64 * buttonValues[6] + 128 * buttonValues[7] + 256 * buttonValues[8]; // C004 - *JC - record keys pressed for next time through the loop C007 added new pseudo buttons to keychange value
    if (debug == 6)
    {
      Serial.print("keyChange = ");
      Serial.println(keyChange);
    } // C005 - *JC - to help debug key press issues
  }
}

void loop()
{
#if ENABLE_LEDS
  ledUpdate();
#endif

#if STATUS_LED_TEST_ONLY
  return;
#endif

#if defined(SENSOR_BACKEND_TLV493D)
  float rawReads[TLV493D_SENSOR_COUNT * 3], centeredTlv[TLV493D_SENSOR_COUNT * 3];
#else
  int rawReads[8], centered[8];
#endif
  uint8_t buttonReads[9]; // C007 - *JC added two more values for two extra pseudo buttons C009 added another 3 pseudo switches to cycle views when button 2 (front) pressed

  // sensor values are read. range should be 176 - 1024 for debug levels other than 1 and 88-860 for debug 1
#if defined(SENSOR_BACKEND_TLV493D)
  readAllFromSensors(rawReads, tlvCenterPoints);
#else
  readAllFromSensors(rawReads);
#endif
  // button values true or false
  readAllFromButtons(buttonReads);

  // Report back 0-1023 raw ADC 10-bit values if enabled
  if (debug == 1)
  {
#if defined(SENSOR_BACKEND_TLV493D)
    if (debug1SameLine)
    {
      Serial.print('\r');
    }
    printTlvReadings(rawReads);
    if (debug1SameLine)
    {
      Serial.print("    ");
    }
    else
    {
      Serial.println();
    }
#else
    if (debug1SameLine)
    {
      Serial.print('\r');
    }

    Serial.print("HES0:");
    Serial.print(rawReads[0]);
    Serial.print(",");
    Serial.print("HES1:");
    Serial.print(rawReads[1]);
    Serial.print(",");
    Serial.print("HES2:");
    Serial.print(rawReads[2]);
    Serial.print(",");
    Serial.print("HES3:");
    Serial.print(rawReads[3]);
    Serial.print(",");
    Serial.print("HES6:");
    Serial.print(rawReads[4]);
    Serial.print(",");
    Serial.print("HES7:");
    Serial.print(rawReads[5]);
    Serial.print(",");
    Serial.print("HES8:");
    Serial.print(rawReads[6]);
    Serial.print(",");
    Serial.print("HES9:");
    Serial.print(rawReads[7]);

    if (debug1SameLine)
    {
      Serial.print("    ");
    }
    else
    {
      Serial.println();
    }
#endif
  }

#if defined(SENSOR_BACKEND_TLV493D)
  for (int i = 0; i < TLV493D_SENSOR_COUNT * 3; i++)
  {
    centeredTlv[i] = rawReads[i] - tlvCenterPoints[i];
    if (centeredTlv[i] < TLV493D_DEADZONE && centeredTlv[i] > -TLV493D_DEADZONE)
    {
      centeredTlv[i] = 0.0f;
    }
    else
    {
      const float sgn = centeredTlv[i] < 0.0f ? -1.0f : 1.0f;
      centeredTlv[i] = sgn * (fabsf(centeredTlv[i]) - TLV493D_DEADZONE);
    }
  }
#else
  // Subtract centre position from measured position to determine movement.
  // *JC - As we are going negative with the readings, we make them positive
  // by subtraction them from the recorded centerPoints rather than the other was around.
  // C0004 - changed back to the original TT version to match the code from AndunHH
  for (int i = 0; i < 8; i++)
  {
    centered[i] = centerPoints[i] - rawReads[i]; //
  }
#endif
  // Report centered Sensor values if enabled. Values should be approx -256 to +256, jitter around 0 at idle.
  if (debug == 2)
  {
#if defined(SENSOR_BACKEND_TLV493D)
    printTlvReadings(centeredTlv);
    Serial.println();
#else
    Serial.print("HES0:");
    Serial.print(centered[0]);
    Serial.print(",");
    Serial.print("HES1:");
    Serial.print(centered[1]);
    Serial.print(",");
    Serial.print("HES2:");
    Serial.print(centered[2]);
    Serial.print(",");
    Serial.print("HES3:");
    Serial.print(centered[3]);
    Serial.print(",");
    Serial.print("HES6:");
    Serial.print(centered[4]);
    Serial.print(",");
    Serial.print("HES7:");
    Serial.print(centered[5]);
    Serial.print(",");
    Serial.print("HES8:");
    Serial.print(centered[6]);
    Serial.print(",");
    Serial.print("HES9:");
    Serial.println(centered[7]);
#endif
  }
#if defined(SENSOR_BACKEND_HALL)
  // Filter movement values. Set to zero if movement is below deadzone threshold.
  // *JC - Changed operation so there isn't a sudden jump when the value first falls outside deadzone
  for (int i = 0; i < 8; i++)
  {
    if (centered[i] < DEADZONE && centered[i] > -DEADZONE)
    {
      centered[i] = 0;
    }
    else
    {
      int sgn = centered[i] / abs(centered[i]);
      centered[i] = sgn * (abs(centered[i]) - DEADZONE);
    }
  }
#endif
  // Report centered Sensor values. Filtered for deadzone. Approx -500 to +500, locked to zero at idle
  if (debug == 3)
  {
#if defined(SENSOR_BACKEND_TLV493D)
    printTlvReadings(centeredTlv);
#else
    Serial.print("HES0:");
    Serial.print(centered[0]);
    Serial.print(",");
    Serial.print("HES1:");
    Serial.print(centered[1]);
    Serial.print(",");
    Serial.print("HES2:");
    Serial.print(centered[2]);
    Serial.print(",");
    Serial.print("HES3:");
    Serial.print(centered[3]);
    Serial.print(",");
    Serial.print("HES6:");
    Serial.print(centered[4]);
    Serial.print(",");
    Serial.print("HES7:");
    Serial.print(centered[5]);
    Serial.print(",");
    Serial.print("HES8:");
    Serial.print(centered[6]);
    Serial.print(",");
    Serial.print("HES9:");
    Serial.print(centered[7]);
#endif
    printButtonReadings(buttonReads, true);
  }

  // Doing all through arithmetic contribution by fdmakara
  // Integer has been changed to 16 bit int16_t to match what the HID protocol expects.
  Movement6D movement;
  // Original fdmakara calculations
  // transX = (-centered[AX] +centered[CX])/1;
  // transY = (-centered[BX] +centered[DX])/1;
  // transZ = (-centered[AY] -centered[BY] -centered[CY] -centered[DY])/2;
  // rotX = (-centered[AY] +centered[CY])/2;
  // rotY = (+centered[BY] -centered[DY])/2;
  // rotZ = (+centered[AX] +centered[BX] +centered[CX] +centered[DX])/4;

  // *JC - Replaced Joystick calculations with ones for the Hall Effect Sensors.
  // The TLV493D backend keeps this Hall path intact and selects the alternate math at build time.
#if defined(SENSOR_BACKEND_TLV493D)
#if TLV493D_SENSOR_COUNT == 1
  calcTlv493dSingleSensorMovement(centeredTlv, TLV493D_SCALE, movement);
#else
  calcTlv493dMovement(centeredTlv, TLV493D_SCALE, movement);
#endif
#else
  calcHallMovement(centered, movement);
#endif

  // *JC - modified speed calculation to allow for the fact that this is integer calculations
  // so do multiplications prior to divisions to maintain maximum accuracy.
  // C008 now removed
  /*  transX = (transX*speed)/100;
    transY = (transY*speed)/100;
    transZ = (transZ*speed)/100;
    rotX = (rotX*speed)/100;
    rotY = (rotY*speed)/100;
    rotZ = (rotZ*speed)/100; */

  // Invert directions if needed
  if (invX == true)
  {
    movement.tx = movement.tx * -1;
  };
  if (invY == true)
  {
    movement.ty = movement.ty * -1;
  };
  if (invZ == true)
  {
    movement.tz = movement.tz * -1;
  };
  if (invRX == true)
  {
    movement.rx = movement.rx * -1;
  };
  if (invRY == true)
  {
    movement.ry = movement.ry * -1;
  };
  if (invRZ == true)
  {
    movement.rz = movement.rz * -1;
  };

  // Report translation and rotation values if enabled. Approx -800 to 800 depending on the parameter.
  if (debug == 4)
  {
    printMovementReadings(movement);
    Serial.println();
  }
  // Report debug 4 and 5 info side by side for direct reference if enabled. Very useful if you need to alter which inputs are used in th arithmatic above.
  if (debug == 5)
  {
#if defined(SENSOR_BACKEND_TLV493D)
    printTlvReadings(centeredTlv);
    Serial.print("||");
#else
    Serial.print("HES0:");
    Serial.print(centered[0]);
    Serial.print(",");
    Serial.print("HES1:");
    Serial.print(centered[1]);
    Serial.print(",");
    Serial.print("HES2:");
    Serial.print(centered[2]);
    Serial.print(",");
    Serial.print("HES3:");
    Serial.print(centered[3]);
    Serial.print(",");
    Serial.print("HES6:");
    Serial.print(centered[4]);
    Serial.print(",");
    Serial.print("HES7:");
    Serial.print(centered[5]);
    Serial.print(",");
    Serial.print("HES8:");
    Serial.print(centered[6]);
    Serial.print(",");
    Serial.print("HES9:");
    Serial.print(centered[7]);
    Serial.print("||");
#endif
    printMovementReadings(movement);
    Serial.println();
  }

  // Send data to the 3DConnexion software.
  // The correct order for me was determined after trial and error - Teaching Tech
  // *JC - Added buttons for button report
  // *JC C003 Allowing swap between TT movement and 3DC movement defaults.
  if (movement3DC)
  {
    send_command(movement.rx, movement.ry, movement.rz, movement.tx, movement.ty, movement.tz, buttonReads); // 3DC default
  }
  else
  {
    send_command(movement.rx, movement.ry, movement.rz, movement.tx, movement.tz, movement.ty, buttonReads); // TT default
  }
}
