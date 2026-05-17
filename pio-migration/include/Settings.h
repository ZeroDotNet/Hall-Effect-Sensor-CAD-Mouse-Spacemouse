#pragma once

#include "Config.h"

struct Settings
{
  bool invX;
  bool invY;
  bool invZ;
  bool invRX;
  bool invRY;
  bool invRZ;
  bool movement3DC;
  bool cycleButton;
  int deadzone;
  bool hallSensorEnabled[NUM_SENSORS];
};

extern Settings settings;
extern int centerPoints[NUM_SENSORS];

Settings defaultSettings();
bool loadStoredSettings(Settings &targetSettings, int *targetCenterPoints);
bool saveStoredSettings(const Settings &sourceSettings, const int *sourceCenterPoints);

