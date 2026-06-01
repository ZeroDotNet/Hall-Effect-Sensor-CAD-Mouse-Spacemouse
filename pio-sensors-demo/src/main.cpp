#include <Arduino.h>
#include <mbed.h>
#include <Wire.h>
#include <math.h>

namespace
{
constexpr uint8_t TLV493D_ADDRESS = 0x5E;
constexpr uint8_t SDA_PIN = 4;
constexpr uint8_t SCL_PIN = 5;
constexpr uint8_t SERVO_PIN = 15;
constexpr uint8_t LED_PIN = 16;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint16_t SAMPLE_INTERVAL_MS = 35;
constexpr int SERVO_MIN_US = 600;
constexpr int SERVO_MAX_US = 2400;
constexpr int SERVO_HOME_DEG = 90;
constexpr int SERVO_PERIOD_US = 20000;
// Practical full-scale for a close hand-held magnet; lower than the TLV493D raw limit for visible motion.
constexpr float MAGNET_FULL_SCALE = 900.0f;

struct MagneticField
{
  int16_t x;
  int16_t y;
  int16_t z;
  float magnitude;
  bool ok;
};

arduino::MbedI2C *tlvBus = nullptr;
mbed::PwmOut *servoPwm = nullptr;
uint32_t lastSampleMs = 0;

int16_t signExtend12(uint16_t value)
{
  value &= 0x0FFF;
  if (value & 0x0800)
  {
    value |= 0xF000;
  }
  return static_cast<int16_t>(value);
}

float clampFloat(float value, float low, float high)
{
  if (value < low)
  {
    return low;
  }
  if (value > high)
  {
    return high;
  }
  return value;
}

uint8_t clampByte(int value)
{
  if (value < 0)
  {
    return 0;
  }
  if (value > 255)
  {
    return 255;
  }
  return static_cast<uint8_t>(value);
}

bool pingTlv493d()
{
  if (tlvBus == nullptr)
  {
    return false;
  }
  tlvBus->beginTransmission(TLV493D_ADDRESS);
  return tlvBus->endTransmission() == 0;
}

bool configureTlv493d()
{
  if (tlvBus == nullptr)
  {
    return false;
  }

  tlvBus->beginTransmission(TLV493D_ADDRESS);
  tlvBus->write(static_cast<uint8_t>(0x02));
  tlvBus->write(static_cast<uint8_t>(0x00));
  return tlvBus->endTransmission() == 0;
}

MagneticField readMagneticField()
{
  MagneticField field = {0, 0, 0, 0.0f, false};
  uint8_t data[7] = {};

  if (tlvBus == nullptr)
  {
    return field;
  }

  const uint8_t received = tlvBus->requestFrom(TLV493D_ADDRESS, static_cast<uint8_t>(7));
  if (received < 6)
  {
    return field;
  }

  for (uint8_t i = 0; i < received && i < sizeof(data); ++i)
  {
    data[i] = tlvBus->read();
  }

  field.x = signExtend12((static_cast<uint16_t>(data[0]) << 4) | (data[4] >> 4));
  field.y = signExtend12((static_cast<uint16_t>(data[1]) << 4) | (data[4] & 0x0F));
  field.z = signExtend12((static_cast<uint16_t>(data[2]) << 4) | (data[5] & 0x0F));
  field.magnitude = sqrtf(static_cast<float>(field.x) * field.x +
                          static_cast<float>(field.y) * field.y +
                          static_cast<float>(field.z) * field.z);
  field.ok = true;
  return field;
}

int magneticFieldToServoAngle(const MagneticField &field)
{
  const float normalized = clampFloat(static_cast<float>(field.z) / MAGNET_FULL_SCALE, -1.0f, 1.0f);
  return SERVO_HOME_DEG + static_cast<int>(normalized * 80.0f);
}

void writeServoAngle(int angle)
{
  if (servoPwm == nullptr)
  {
    return;
  }

  angle = constrain(angle, 0, 180);
  const int pulseUs = map(angle, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
  servoPwm->pulsewidth_us(pulseUs);
}

void updateLed(const MagneticField &field)
{
  const float normalized = clampFloat(field.magnitude / MAGNET_FULL_SCALE, 0.0f, 1.0f);
  analogWrite(LED_PIN, clampByte(static_cast<int>(normalized * 255.0f)));
}

void logField(const MagneticField &field, int angle)
{
  Serial.print("tlv493d=");
  Serial.print(field.ok ? "ok" : "missing");
  Serial.print(" x=");
  Serial.print(field.x);
  Serial.print(" y=");
  Serial.print(field.y);
  Serial.print(" z=");
  Serial.print(field.z);
  Serial.print(" magnitude=");
  Serial.print(field.magnitude, 1);
  Serial.print(" servo_deg=");
  Serial.print(angle);
  Serial.print(" led_pwm=");
  Serial.println(field.ok ? clampByte(static_cast<int>(clampFloat(field.magnitude / MAGNET_FULL_SCALE, 0.0f, 1.0f) * 255.0f)) : 0);
}
}

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0);

  Serial.begin(SERIAL_BAUD);
  delay(600);
  Serial.println();
  Serial.println("Magnetic Field Puppet demo");
  Serial.println("Move a magnet near the TLV493D: Z axis steers the servo, magnitude brightens the LED.");

  tlvBus = new arduino::MbedI2C(digitalPinToPinName(SDA_PIN), digitalPinToPinName(SCL_PIN));
  tlvBus->begin();
  tlvBus->setClock(400000);

  servoPwm = new mbed::PwmOut(digitalPinToPinName(SERVO_PIN));
  servoPwm->period_us(SERVO_PERIOD_US);
  writeServoAngle(SERVO_HOME_DEG);

  Serial.print("TLV493D address 0x");
  Serial.print(TLV493D_ADDRESS, HEX);
  const bool sensorPresent = pingTlv493d();
  Serial.println(sensorPresent ? " detected" : " not detected");
  if (sensorPresent)
  {
    Serial.println(configureTlv493d() ? "TLV493D configured" : "TLV493D config write failed");
  }
}

void loop()
{
  const uint32_t now = millis();
  if (now - lastSampleMs < SAMPLE_INTERVAL_MS)
  {
    return;
  }
  lastSampleMs = now;

  const MagneticField field = readMagneticField();
  if (!field.ok)
  {
    writeServoAngle(SERVO_HOME_DEG);
    analogWrite(LED_PIN, 0);
    logField(field, SERVO_HOME_DEG);
    return;
  }

  const int angle = magneticFieldToServoAngle(field);
  writeServoAngle(angle);
  updateLed(field);
  logField(field, angle);
}
