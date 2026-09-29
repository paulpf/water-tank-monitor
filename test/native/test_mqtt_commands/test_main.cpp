#include <unity.h>
#include <string.h>
#include "config.h"
#include "mqttcommands.h"

void setUp() {}
void tearDown() {}

static bool interval(const char *text, uint32_t &out)
{
  return MqttCommands::parseInterval(reinterpret_cast<const uint8_t *>(text), strlen(text), out);
}

static bool flag(const char *text, bool &out)
{
  return MqttCommands::parseFlag(reinterpret_cast<const uint8_t *>(text), strlen(text), out);
}

void test_interval_accepts_plain_decimal()
{
  uint32_t out = 0;
  TEST_ASSERT_TRUE(interval("5000", out));
  TEST_ASSERT_EQUAL_UINT32(5000, out);
}

void test_interval_accepts_lower_bound_and_uint32_max()
{
  uint32_t out = 0;
  TEST_ASSERT_TRUE(interval("500", out));
  TEST_ASSERT_EQUAL_UINT32(CALIBRATION_INTERVAL_MS, out);
  TEST_ASSERT_TRUE(interval("4294967295", out));
  TEST_ASSERT_EQUAL_UINT32(4294967295u, out);
}

void test_interval_rejects_invalid_payloads()
{
  const char *rejected[] = {"", "0", "499", "0500", "abc", "12abc", " 12", "12 ", "-5", "+500",
                            "4294967296", "9999999999", "12345678901"};
  for (const char *text : rejected)
  {
    uint32_t out = 42;
    TEST_ASSERT_FALSE_MESSAGE(interval(text, out), text);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(42, out, text);
  }
}

void test_interval_does_not_read_past_length()
{
  // Payloads from PubSubClient are not null-terminated.
  const uint8_t payload[] = {'1', '0', '0', '0', 'x'};
  uint32_t out = 0;
  TEST_ASSERT_TRUE(MqttCommands::parseInterval(payload, 4, out));
  TEST_ASSERT_EQUAL_UINT32(1000, out);
}

void test_flag_accepts_zero_and_one()
{
  bool out = true;
  TEST_ASSERT_TRUE(flag("0", out));
  TEST_ASSERT_FALSE(out);
  TEST_ASSERT_TRUE(flag("1", out));
  TEST_ASSERT_TRUE(out);
}

void test_flag_rejects_everything_else()
{
  const char *rejected[] = {"", "01", "10", "2", "true", "on", " 1"};
  for (const char *text : rejected)
  {
    bool out = true;
    TEST_ASSERT_FALSE_MESSAGE(flag(text, out), text);
    TEST_ASSERT_TRUE_MESSAGE(out, text);
  }
}

int main(int, char **)
{
  UNITY_BEGIN();
  RUN_TEST(test_interval_accepts_plain_decimal);
  RUN_TEST(test_interval_accepts_lower_bound_and_uint32_max);
  RUN_TEST(test_interval_rejects_invalid_payloads);
  RUN_TEST(test_interval_does_not_read_past_length);
  RUN_TEST(test_flag_accepts_zero_and_one);
  RUN_TEST(test_flag_rejects_everything_else);
  return UNITY_END();
}
