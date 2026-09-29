#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// Minimal configuration for WiFi + OTA only

// Serial configuration
#define SERIAL_BAUD_RATE 115200
constexpr uint32_t SERIAL_STARTUP_DELAY_MS = 500;

// System timings (in milliseconds)
#define WATCHDOG_TIMEOUT 30000
#define LONG_INTERVAL 60000    // 1 minute
#define MIDDLE_INTERVAL 10000  // 10 seconds  
#define SHORT_INTERVAL 1000    // 1 second

// Trace level for logging
#define TRACE_LEVEL TraceLevel::INFO

// OTA configuration
#define ENABLE_OTA true
#if defined(ARDUINO_ARCH_ESP8266)
constexpr uint16_t OTA_PORT = 8266;
#else
constexpr uint16_t OTA_PORT = 3232;
#endif
// Security default: fail closed if no OTA password is configured
#define OTA_ALLOW_INSECURE_NO_PASSWORD false

// Device identity
#define DEVICE_NAME "water-tank-monitor"

// WiFi configuration
constexpr int WIFI_CONNECTION_TIMEOUT = 10000; // milliseconds
constexpr uint32_t WIFI_INITIAL_CONNECT_TIMEOUT_MS = 15000;
constexpr uint8_t WIFI_MAX_RECONNECT_ATTEMPTS = 10;
constexpr uint32_t WIFI_RECONNECT_BASE_DELAY_MS = 1000;
constexpr uint32_t WIFI_RECONNECT_MAX_DELAY_MS = 30000;
constexpr uint32_t WIFI_RECONNECT_JITTER_MS = 500;
// Keep device alive for diagnostics instead of forced reboot
#define WIFI_RESTART_ON_RECONNECT_FAILURE false

// Application loop behavior
constexpr uint32_t STATUS_PRINT_INTERVAL_MS = 30000;

// MQTT configuration
constexpr uint32_t MQTT_RETRY_INTERVAL_MS  = 5000;
#define MQTT_TOPIC_LEVEL_PERCENT       DEVICE_NAME "/tank/levelPercent"
#define MQTT_TOPIC_HEIGHT_CM           DEVICE_NAME "/tank/heightCm"
#define MQTT_TOPIC_VOLUME_LITERS       DEVICE_NAME "/tank/volumeLiters"
#define MQTT_TOPIC_VOLUME_OVERFLOW_L   DEVICE_NAME "/tank/volumeOverflowLiters"
#define MQTT_TOPIC_CURRENT_MA          DEVICE_NAME "/tank/currentMa"
#define MQTT_TOPIC_VOLTAGE_V           DEVICE_NAME "/tank/voltageV"
#define MQTT_TOPIC_VALID               DEVICE_NAME "/tank/valid"
#define MQTT_TOPIC_READ_INTERVAL_MS    DEVICE_NAME "/config/readIntervalMs"        // state (retained, device publishes)
#define MQTT_TOPIC_READ_INTERVAL_SET   DEVICE_NAME "/config/readIntervalMs/set"    // command (write here to change it)
#define MQTT_TOPIC_PUBLISH_INTERVAL_MS  DEVICE_NAME "/config/publishIntervalMs"     // state (retained, device publishes)
#define MQTT_TOPIC_PUBLISH_INTERVAL_SET DEVICE_NAME "/config/publishIntervalMs/set" // command (write here to change it)
#define MQTT_TOPIC_CALIBRATION_MODE     DEVICE_NAME "/config/calibrationMode"       // state (retained, "1"/"0")
#define MQTT_TOPIC_CALIBRATION_MODE_SET DEVICE_NAME "/config/calibrationMode/set"   // command ("1"=on, "0"=off)
constexpr uint32_t CALIBRATION_INTERVAL_MS = 500; // read+publish cadence while calibrationMode is on
#define MQTT_TOPIC_RSSI                DEVICE_NAME "/system/rssi"
#define MQTT_TOPIC_IP                  DEVICE_NAME "/system/ip"
#define MQTT_TOPIC_HEALTH              DEVICE_NAME "/system/health"  // JSON: sensor + system diagnostics
#define MQTT_TOPIC_COMMAND_RESET       DEVICE_NAME "/command/reset"  // any payload triggers ESP.restart()
constexpr uint32_t MQTT_RSSI_INTERVAL_MS = 5000;

// TL-136 4-20mA level sensor via signal conditioner + ADS1115 (I2C, 16-bit ADC)
// Hardware: USB 5V → Boost converter → 24V loop → TL-136 → 4-20mA receiver
//           → ADS1115 A0 (I2C) → D1 Mini D1/SCL + D2/SDA
// Signal conditioner calibrated: 4mA → 0V, 20mA → 3.3V (SPAN/ZERO trimmer)
// ADS1115: ADDR pin to GND → I2C address 0x48
//          GAIN_ONE = ±4.096V range → covers 0–3.3V with 0.125 mV resolution
constexpr uint8_t  SENSOR_ADS_I2C_ADDR      = 0x48;
constexpr uint8_t  SENSOR_ADS_CHANNEL       = 0;      // ADS1115 A0
constexpr float    SENSOR_VREF              = 3.153f; // signal conditioner measured max output [V]
constexpr uint32_t SENSOR_READ_INTERVAL_MS  = 500;     // Default read cadence (calibration); runtime value lives in SystemConfig.sensorReadIntervalMs
constexpr uint32_t MQTT_PUBLISH_INTERVAL_MS = 1000;    // Default publish cadence; runtime value lives in SystemConfig.publishIntervalMs

// Tank geometry (Zisterne Family F 6500)
// Cylinder: 0–198 cm, r=1.0m
// Cone: 198–210 cm (12 cm height)
// Empty (0%): 13 cm height = 408 Liters (sensor position, prevents clogging)
// Full (100%): 210 cm height = 6500 Liters (drain outlet level)
// Critical: 266 cm height (electrical socket danger point)
constexpr float TANK_CYLINDER_HEIGHT_CM = 198.0f;
constexpr float TANK_CONE_START_CM      = 198.0f;
constexpr float TANK_CONE_END_CM        = 210.0f;
constexpr float TANK_RADIUS_M           = 1.0f;
constexpr float TANK_MIN_HEIGHT_CM      = 13.0f;    // Sensor position (0%)
constexpr float TANK_DRAIN_HEIGHT_CM    = 210.0f;   // Drain outlet (100% normal operation)
constexpr float TANK_CRITICAL_HEIGHT_CM = 250.0f;   // Electrical socket danger point

// heightCm below is the total physical water height from the tank bottom
// (matches direct tape-measure readings), not the water column above the
// sensor - the sensor sits at TANK_MIN_HEIGHT_CM, so 0 V maps there.
struct SensorCalPoint { float voltageV; float heightCm; };
constexpr SensorCalPoint SENSOR_CAL_TABLE[] = {
    { 0.000f,  13.0f },   // 0 V = 13 cm (sensor position, no water above sensor)
    { 0.2277f, 20.0f },   // 0,2277 V = 20 cm (direct measurement)
    { 2.2419f, 210.0f },  // 2,2419 V = 210 cm (direct measurement)
    { 2.3448f, 220.0f },  // 2,3448 V = 220 cm (extrapolated: signal conditioner full-scale output, 20 mA)
    { 3.300f,  309.8f }   // 3,300 V = 309,8 cm (extrapolated: signal conditioner full-scale output, 20 mA)
};
constexpr int SENSOR_CAL_TABLE_SIZE =
    static_cast<int>(sizeof(SENSOR_CAL_TABLE) / sizeof(SENSOR_CAL_TABLE[0]));

#endif // CONFIG_H
