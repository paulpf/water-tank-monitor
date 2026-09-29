#include <unity.h>
#include "config.h"
#include "tanklevel.h"
#include "tankmodel.h"

void setUp() {}
void tearDown() {}

// --- voltage -> height (calibration table) ---

void test_height_at_every_calibration_point()
{
  for (int i = 0; i < SENSOR_CAL_TABLE_SIZE; i++)
  {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, SENSOR_CAL_TABLE[i].heightCm,
                             TankModel::voltageToHeightCm(SENSOR_CAL_TABLE[i].voltageV));
  }
}

void test_height_interpolates_linearly_within_segment()
{
  const SensorCalPoint &p0 = SENSOR_CAL_TABLE[1];
  const SensorCalPoint &p1 = SENSOR_CAL_TABLE[2];
  const float midV = (p0.voltageV + p1.voltageV) / 2.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.01f, (p0.heightCm + p1.heightCm) / 2.0f, TankModel::voltageToHeightCm(midV));
}

void test_height_is_clamped_below_first_point()
{
  TEST_ASSERT_FLOAT_WITHIN(0.001f, TANK_MIN_HEIGHT_CM, TankModel::voltageToHeightCm(-0.1f));
}

void test_height_extrapolates_above_last_point()
{
  const SensorCalPoint &p0 = SENSOR_CAL_TABLE[SENSOR_CAL_TABLE_SIZE - 2];
  const SensorCalPoint &p1 = SENSOR_CAL_TABLE[SENSOR_CAL_TABLE_SIZE - 1];
  const float slope = (p1.heightCm - p0.heightCm) / (p1.voltageV - p0.voltageV);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, p1.heightCm + slope * 0.1f, TankModel::voltageToHeightCm(p1.voltageV + 0.1f));
}

// --- height -> percent / volume / overflow ---

void test_percent_is_zero_at_sensor_and_hundred_at_drain()
{
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, TankModel::heightToPercent(TANK_MIN_HEIGHT_CM));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, TankModel::heightToPercent(TANK_DRAIN_HEIGHT_CM));
}

void test_volume_at_geometry_boundaries()
{
  TEST_ASSERT_FLOAT_WITHIN(0.001f, TANK_MIN_VOLUME_L, TankModel::heightToVolumeLiters(TANK_MIN_HEIGHT_CM));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, TANK_MIN_VOLUME_L, TankModel::heightToVolumeLiters(5.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 6220.35f, TankModel::heightToVolumeLiters(TANK_CYLINDER_HEIGHT_CM));
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 6360.18f, TankModel::heightToVolumeLiters(204.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.5f, TANK_NOMINAL_VOLUME_L, TankModel::heightToVolumeLiters(TANK_DRAIN_HEIGHT_CM));
}

void test_overflow_only_above_nominal_volume()
{
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, TankModel::overflowLiters(TANK_NOMINAL_VOLUME_L));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, TankModel::overflowLiters(1000.0f));
  const float volumeAt222 = TankModel::heightToVolumeLiters(222.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 6779.65f, volumeAt222);
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 279.65f, TankModel::overflowLiters(volumeAt222));
}

// --- voltage -> current, validity ---

void test_current_maps_zero_and_vref_to_4_and_20_mA()
{
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 4.0f, TankModel::voltageToCurrentMa(0.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 20.0f, TankModel::voltageToCurrentMa(SENSOR_VREF));
}

void test_validity_bounds()
{
  TankLevel level = TankLevel::notReady();
  level.currentMa = 3.79f;
  TEST_ASSERT_FALSE(level.isValid());
  level.currentMa = 3.8f;
  TEST_ASSERT_TRUE(level.isValid());
  level.currentMa = 20.5f;
  TEST_ASSERT_TRUE(level.isValid());
  level.currentMa = 20.51f;
  TEST_ASSERT_FALSE(level.isValid());
}

void test_not_ready_is_invalid()
{
  TEST_ASSERT_FALSE(TankLevel::notReady().isValid());
}

// --- regression: values produced by the pre-refactor levelsensor.cpp math ---

struct Reference
{
  float voltageV, currentMa, heightCm, levelPercent, volumeLiters, overflowLiters;
};

void test_matches_pre_refactor_values()
{
  const Reference refs[] = {
      {0.0f, 4.000000f, 13.000000f, 0.000000f, 408.000000f, 0.000000f},
      {0.2277f, 5.155471f, 20.000000f, 3.553300f, 628.318542f, 0.000000f},
      {1.2f, 10.089438f, 111.717316f, 50.110310f, 3509.703125f, 0.000000f},
      {2.2419f, 15.376594f, 210.000000f, 100.000000f, 6500.000000f, 0.000000f},
      {2.9f, 18.716145f, 272.195312f, 131.571228f, 7949.391602f, 1449.391602f},
      {3.4f, 21.253408f, 319.201172f, 155.432068f, 9044.810547f, 2544.810547f},
  };
  for (const Reference &r : refs)
  {
    const TankLevel level = TankModel::fromVoltage(r.voltageV);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, r.voltageV, level.voltageV);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, r.currentMa, level.currentMa);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, r.heightCm, level.heightCm);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, r.levelPercent, level.levelPercent);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, r.volumeLiters, level.volumeLiters);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, r.overflowLiters, level.overflowLiters);
  }
}

int main(int, char **)
{
  UNITY_BEGIN();
  RUN_TEST(test_height_at_every_calibration_point);
  RUN_TEST(test_height_interpolates_linearly_within_segment);
  RUN_TEST(test_height_is_clamped_below_first_point);
  RUN_TEST(test_height_extrapolates_above_last_point);
  RUN_TEST(test_percent_is_zero_at_sensor_and_hundred_at_drain);
  RUN_TEST(test_volume_at_geometry_boundaries);
  RUN_TEST(test_overflow_only_above_nominal_volume);
  RUN_TEST(test_current_maps_zero_and_vref_to_4_and_20_mA);
  RUN_TEST(test_validity_bounds);
  RUN_TEST(test_not_ready_is_invalid);
  RUN_TEST(test_matches_pre_refactor_values);
  return UNITY_END();
}
