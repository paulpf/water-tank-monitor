#include <unity.h>
#include "intervaltimer.h"

void setUp() {}
void tearDown() {}

void test_not_due_before_interval_elapsed()
{
  IntervalTimer timer;
  TEST_ASSERT_FALSE(timer.due(999, 1000));
  TEST_ASSERT_TRUE(timer.due(1000, 1000));
  TEST_ASSERT_EQUAL_UINT32(1000, timer.last);
}

void test_fires_again_one_interval_after_last_fire()
{
  IntervalTimer timer;
  TEST_ASSERT_TRUE(timer.due(1000, 1000));
  TEST_ASSERT_FALSE(timer.due(1999, 1000));
  TEST_ASSERT_TRUE(timer.due(2000, 1000));
}

void test_does_not_catch_up_missed_intervals()
{
  IntervalTimer timer;
  TEST_ASSERT_TRUE(timer.due(10000, 1000));
  TEST_ASSERT_FALSE(timer.due(10001, 1000));
}

void test_is_wrap_safe()
{
  IntervalTimer timer;
  timer.last = 0xFFFFFF00u;
  // 0x10 - 0xFFFFFF00 wraps to 272 ms elapsed on every platform.
  TEST_ASSERT_FALSE(timer.due(0x00000010u, 1000));
  TEST_ASSERT_TRUE(timer.due(0x00000010u, 200));
  TEST_ASSERT_EQUAL_UINT32(0x00000010u, timer.last);
}

void test_reset_restarts_the_interval()
{
  IntervalTimer timer;
  timer.reset(5000);
  TEST_ASSERT_FALSE(timer.due(5999, 1000));
  TEST_ASSERT_TRUE(timer.due(6000, 1000));
}

int main(int, char **)
{
  UNITY_BEGIN();
  RUN_TEST(test_not_due_before_interval_elapsed);
  RUN_TEST(test_fires_again_one_interval_after_last_fire);
  RUN_TEST(test_does_not_catch_up_missed_intervals);
  RUN_TEST(test_is_wrap_safe);
  RUN_TEST(test_reset_restarts_the_interval);
  return UNITY_END();
}
