#include "Motion.h"

MotionReport computeMotion(const int *centered, const Settings &currentSettings)
{
  MotionReport motion;
  motion.transX = (centered[HES1] - centered[HES0] + centered[HES6] - centered[HES7]) / 2;
  motion.transY = (centered[HES2] - centered[HES3] + centered[HES9] - centered[HES8]) / 2;
  motion.transZ = (centered[HES0] + centered[HES1] + centered[HES2] + centered[HES3] + centered[HES6] + centered[HES7] + centered[HES8] + centered[HES9]) / 4;
  motion.rotX = (centered[HES0] + centered[HES1] - centered[HES6] - centered[HES7]) / 2;
  motion.rotY = (centered[HES8] + centered[HES9] - centered[HES2] - centered[HES3]) / 2;
  motion.rotZ = (centered[HES0] + centered[HES2] + centered[HES6] + centered[HES8] - centered[HES1] - centered[HES3] - centered[HES7] - centered[HES9]) / 4;

  if (currentSettings.invX)
  {
    motion.transX = -motion.transX;
  }
  if (currentSettings.invY)
  {
    motion.transY = -motion.transY;
  }
  if (currentSettings.invZ)
  {
    motion.transZ = -motion.transZ;
  }
  if (currentSettings.invRX)
  {
    motion.rotX = -motion.rotX;
  }
  if (currentSettings.invRY)
  {
    motion.rotY = -motion.rotY;
  }
  if (currentSettings.invRZ)
  {
    motion.rotZ = -motion.rotZ;
  }

  return motion;
}

