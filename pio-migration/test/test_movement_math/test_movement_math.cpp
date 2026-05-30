#include <unity.h>

#include "movement_math.h"

void test_hall_backend_keeps_existing_mapping()
{
  const int centered[8] = {
      10, 30,
      70, 20,
      90, 40,
      15, 45};

  Movement6D out;
  calcHallMovement(centered, out);

  TEST_ASSERT_EQUAL_INT16(35, out.tx);
  TEST_ASSERT_EQUAL_INT16(40, out.ty);
  TEST_ASSERT_EQUAL_INT16(80, out.tz);
  TEST_ASSERT_EQUAL_INT16(-45, out.rx);
  TEST_ASSERT_EQUAL_INT16(-15, out.ry); // (15+45-70-20)/2 = -30/2 = -15
  TEST_ASSERT_EQUAL_INT16(12, out.rz);
}

void test_tlv493d_backend_maps_three_sensors_to_6dof()
{
  const float centered[9] = {
      3.0f, 6.0f, 9.0f,
      6.0f, 3.0f, 15.0f,
      9.0f, 0.0f, 21.0f};

  Movement6D out;
  calcTlv493dMovement(centered, 10.0f, out);

  TEST_ASSERT_EQUAL_INT16(60, out.tx);
  TEST_ASSERT_EQUAL_INT16(30, out.ty);
  TEST_ASSERT_EQUAL_INT16(150, out.tz);
  TEST_ASSERT_EQUAL_INT16(-90, out.rx);
  TEST_ASSERT_EQUAL_INT16(-60, out.ry);
  TEST_ASSERT_EQUAL_INT16(45, out.rz);
}

int main(int, char **)
{
  UNITY_BEGIN();
  RUN_TEST(test_hall_backend_keeps_existing_mapping);
  RUN_TEST(test_tlv493d_backend_maps_three_sensors_to_6dof);
  return UNITY_END();
}
