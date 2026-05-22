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
#define MQTT_TOPIC_LEVEL_PERCENT  DEVICE_NAME "/tank/levelPercent"
#define MQTT_TOPIC_CURRENT_MA     DEVICE_NAME "/tank/currentMa"
#define MQTT_TOPIC_RSSI           DEVICE_NAME "/system/rssi"
constexpr uint32_t MQTT_RSSI_INTERVAL_MS = 5000;

// TL-136 4-20mA level sensor via signal conditioner + ADS1115 (I2C, 16-bit ADC)
// Hardware: USB 5V → Boost converter → 12V loop → TL-136 → 4-20mA receiver
//           → ADS1115 A0 (I2C) → D1 Mini D1/SCL + D2/SDA
// Signal conditioner calibrated: 4mA → 0V, 20mA → 3.3V (SPAN/ZERO trimmer)
// ADS1115: ADDR pin to GND → I2C address 0x48
//          GAIN_ONE = ±4.096V range → covers 0–3.3V with 0.125 mV resolution
constexpr uint8_t  SENSOR_ADS_I2C_ADDR    = 0x48;
constexpr uint8_t  SENSOR_ADS_CHANNEL     = 0;      // ADS1115 A0
constexpr float    SENSOR_VREF            = 3.3f;   // signal conditioner max output [V]
constexpr uint32_t SENSOR_READ_INTERVAL_MS = 60000;

// Piecewise linear calibration table: { voltage [V], actual fill level [%] }
// Points must be sorted by voltage (ascending).
// Add more points for better accuracy — two points minimum.
struct SensorCalPoint { float voltageV; float actualPercent; };
constexpr SensorCalPoint SENSOR_CAL_TABLE[] = {
    { 0.00f,  0.0f },   // 4 mA  — empty tank (ZERO trimmer, measured)
    { 2.09f, 71.0f },   // measured reference point
    { 2.94f, 100.0f },  // 20 mA — full tank (calculated, replace with measured value!)
};
constexpr int SENSOR_CAL_TABLE_SIZE =
    static_cast<int>(sizeof(SENSOR_CAL_TABLE) / sizeof(SENSOR_CAL_TABLE[0]));

#endif // CONFIG_H
