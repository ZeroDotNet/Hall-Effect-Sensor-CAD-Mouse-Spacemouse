#ifndef MOVEMENT_MATH_H
#define MOVEMENT_MATH_H

#include <stdint.h>

struct Movement6D
{
  int16_t tx;
  int16_t ty;
  int16_t tz;
  int16_t rx;
  int16_t ry;
  int16_t rz;
};

inline int16_t toInt16(float value)
{
  return static_cast<int16_t>(value);
}

inline void calcHallMovement(const int centered[8], Movement6D &out)
{
  out.tx = (centered[1] - centered[0] + centered[4] - centered[5]) / 2;
  out.ty = (centered[2] - centered[3] + centered[7] - centered[6]) / 2;
  out.tz = (centered[0] + centered[1] + centered[2] + centered[3] + centered[4] + centered[5] + centered[6] + centered[7]) / 4;
  out.rx = (centered[0] + centered[1] - centered[4] - centered[5]) / 2;
  out.ry = (centered[6] + centered[7] - centered[2] - centered[3]) / 2;
  out.rz = (centered[0] + centered[2] + centered[4] + centered[6] - centered[1] - centered[3] - centered[5] - centered[7]) / 4;
}

inline void calcTlv493dMovement(const float centered[9], float scale, Movement6D &out)
{
  out.tx = toInt16(((centered[0] + centered[3] + centered[6]) / 3.0f) * scale);
  out.ty = toInt16(((centered[1] + centered[4] + centered[7]) / 3.0f) * scale);
  out.tz = toInt16(((centered[2] + centered[5] + centered[8]) / 3.0f) * scale);
  out.rx = toInt16((centered[2] - ((centered[5] + centered[8]) / 2.0f)) * scale);
  out.ry = toInt16((centered[5] - centered[8]) * scale);
  out.rz = toInt16((centered[1] - (0.5f * centered[4]) - (0.5f * centered[7])) * scale);
}

inline void calcTlv493dSingleSensorMovement(const float centered[3], float scale, Movement6D &out)
{
  out.tx = toInt16(centered[0] * scale);
  out.ty = toInt16(centered[1] * scale);
  out.tz = toInt16(centered[2] * scale);
  out.rx = 0;
  out.ry = 0;
  // For a single-sensor dial, Z-field change (axial push/tilt) drives rotation-Z.
  // Replace with atan2(centered[1], centered[0]) for a proper twist angle if needed.
  out.rz = toInt16(centered[2] * scale);
}

#endif
