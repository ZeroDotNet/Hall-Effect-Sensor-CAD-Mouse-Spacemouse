#include <unity.h>

#include "Buttons.h"
#include "Motion.h"
#include "Settings.h"

void test_button_mask_tracks_all_nine_button_slots()
{
  uint8_t buttons[NUM_BUTTON_VALUES] = {};
  buttons[8] = 1;
  TEST_ASSERT_EQUAL_UINT16(0x0100, computeButtonStateMask(buttons));
}

void test_cycle_button_report_maps_third_view()
{
  Settings testSettings = defaultSettings();
  testSettings.cycleButton = true;

  uint8_t buttons[NUM_BUTTON_VALUES] = {};
  uint8_t report[HID_BUTTON_REPORT_BYTES] = {};
  buttons[8] = 1;

  buildButtonReport(buttons, testSettings, report);

  TEST_ASSERT_EQUAL_UINT8(4, report[0]);
  TEST_ASSERT_EQUAL_UINT8(0, report[1]);
  TEST_ASSERT_EQUAL_UINT8(0, report[2]);
  TEST_ASSERT_EQUAL_UINT8(0, report[3]);
}

void test_neutral_motion_is_zero()
{
  Settings testSettings = defaultSettings();
  int centered[NUM_SENSORS] = {};

  const MotionReport motion = computeMotion(centered, testSettings);

  TEST_ASSERT_EQUAL_INT16(0, motion.transX);
  TEST_ASSERT_EQUAL_INT16(0, motion.transY);
  TEST_ASSERT_EQUAL_INT16(0, motion.transZ);
  TEST_ASSERT_EQUAL_INT16(0, motion.rotX);
  TEST_ASSERT_EQUAL_INT16(0, motion.rotY);
  TEST_ASSERT_EQUAL_INT16(0, motion.rotZ);
}

void test_motion_applies_inversion_preferences()
{
  Settings testSettings = defaultSettings();
  testSettings.invX = true;
  int centered[NUM_SENSORS] = {};
  centered[HES1] = 100;

  const MotionReport motion = computeMotion(centered, testSettings);

  TEST_ASSERT_EQUAL_INT16(-50, motion.transX);
}

int main(int, char **)
{
  UNITY_BEGIN();
  RUN_TEST(test_button_mask_tracks_all_nine_button_slots);
  RUN_TEST(test_cycle_button_report_maps_third_view);
  RUN_TEST(test_neutral_motion_is_zero);
  RUN_TEST(test_motion_applies_inversion_preferences);
  return UNITY_END();
}
