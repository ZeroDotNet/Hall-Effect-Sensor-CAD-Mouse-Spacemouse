#include "Settings.h"

#include <stddef.h>
#include <string.h>

#if defined(ARDUINO_ARCH_AVR)
#include <EEPROM.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <Preferences.h>
#endif

namespace
{
constexpr uint32_t STORED_CONFIG_MAGIC = 0x534D4348UL; // SMCH
constexpr uint8_t STORED_CONFIG_VERSION = 1;

struct StoredConfig
{
  uint32_t magic;
  uint8_t version;
  int centerPoints[NUM_SENSORS];
  bool invX;
  bool invY;
  bool invZ;
  bool invRX;
  bool invRY;
  bool invRZ;
  bool movement3DC;
  bool cycleButton;
  int deadzone;
  uint16_t checksum;
};

uint16_t checksumConfig(const StoredConfig &config)
{
  const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&config);
  const size_t checksumOffset = offsetof(StoredConfig, checksum);
  uint16_t checksum = 0xA5A5;
  for (size_t i = 0; i < checksumOffset; i++)
  {
    checksum = static_cast<uint16_t>((checksum << 5) ^ (checksum >> 11) ^ bytes[i]);
  }
  return checksum;
}

void settingsToStored(const Settings &sourceSettings, const int *sourceCenterPoints, StoredConfig &stored)
{
  memset(&stored, 0, sizeof(stored));
  stored.magic = STORED_CONFIG_MAGIC;
  stored.version = STORED_CONFIG_VERSION;
  memcpy(stored.centerPoints, sourceCenterPoints, sizeof(stored.centerPoints));
  stored.invX = sourceSettings.invX;
  stored.invY = sourceSettings.invY;
  stored.invZ = sourceSettings.invZ;
  stored.invRX = sourceSettings.invRX;
  stored.invRY = sourceSettings.invRY;
  stored.invRZ = sourceSettings.invRZ;
  stored.movement3DC = sourceSettings.movement3DC;
  stored.cycleButton = sourceSettings.cycleButton;
  stored.deadzone = sourceSettings.deadzone;
  stored.checksum = checksumConfig(stored);
}

void storedToSettings(const StoredConfig &stored, Settings &targetSettings, int *targetCenterPoints)
{
  targetSettings = defaultSettings();
  memcpy(targetCenterPoints, stored.centerPoints, sizeof(stored.centerPoints));
  targetSettings.invX = stored.invX;
  targetSettings.invY = stored.invY;
  targetSettings.invZ = stored.invZ;
  targetSettings.invRX = stored.invRX;
  targetSettings.invRY = stored.invRY;
  targetSettings.invRZ = stored.invRZ;
  targetSettings.movement3DC = stored.movement3DC;
  targetSettings.cycleButton = stored.cycleButton;
  targetSettings.deadzone = stored.deadzone;
}

bool isValidStoredConfig(const StoredConfig &stored)
{
  return stored.magic == STORED_CONFIG_MAGIC &&
         stored.version == STORED_CONFIG_VERSION &&
         stored.checksum == checksumConfig(stored);
}
}

Settings settings = defaultSettings();
int centerPoints[NUM_SENSORS] = {};

Settings defaultSettings()
{
  Settings defaults = {
      SPACEMOUSE_INV_X != 0,
      SPACEMOUSE_INV_Y != 0,
      SPACEMOUSE_INV_Z != 0,
      SPACEMOUSE_INV_RX != 0,
      SPACEMOUSE_INV_RY != 0,
      SPACEMOUSE_INV_RZ != 0,
      SPACEMOUSE_MOVEMENT_3DC != 0,
      SPACEMOUSE_CYCLE_BUTTON != 0,
      SPACEMOUSE_DEADZONE,
      {}};

  for (uint8_t i = 0; i < NUM_SENSORS; i++)
  {
    defaults.hallSensorEnabled[i] = (SPACEMOUSE_ENABLED_HALL_MASK & (1 << i)) != 0;
  }

  return defaults;
}

bool loadStoredSettings(Settings &targetSettings, int *targetCenterPoints)
{
  StoredConfig stored;

#if defined(ARDUINO_ARCH_AVR)
  EEPROM.get(0, stored);
#elif defined(ARDUINO_ARCH_ESP32)
  Preferences preferences;
  if (!preferences.begin("spacemouse", true))
  {
    return false;
  }
  const size_t storedLength = preferences.getBytesLength("config");
  if (storedLength != sizeof(stored))
  {
    preferences.end();
    return false;
  }
  preferences.getBytes("config", &stored, sizeof(stored));
  preferences.end();
#else
  (void)targetSettings;
  (void)targetCenterPoints;
  return false;
#endif

  if (!isValidStoredConfig(stored))
  {
    return false;
  }

  storedToSettings(stored, targetSettings, targetCenterPoints);
  return true;
}

bool saveStoredSettings(const Settings &sourceSettings, const int *sourceCenterPoints)
{
  StoredConfig stored;
  settingsToStored(sourceSettings, sourceCenterPoints, stored);

#if defined(ARDUINO_ARCH_AVR)
  EEPROM.put(0, stored);
  return true;
#elif defined(ARDUINO_ARCH_ESP32)
  Preferences preferences;
  if (!preferences.begin("spacemouse", false))
  {
    return false;
  }
  const size_t written = preferences.putBytes("config", &stored, sizeof(stored));
  preferences.end();
  return written == sizeof(stored);
#else
  (void)stored;
  return false;
#endif
}
