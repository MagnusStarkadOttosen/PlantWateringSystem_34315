#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/*
  CENTRAL CONFIG FILE
  ===================
  All board-specific values go here.

  If a pin changes, timing changes, or output polarity changes,
  nobody should edit the module source code.
*/

/*
  Hardware pins
*/
static const uint8_t PIN_FAN = D5;
static const uint8_t PIN_PUMP = D6;
static const uint8_t PIN_SOIL_SENSOR = D0;
static const uint8_t PIN_SOIL_SENSOR_POWER = D7;
static const uint8_t PIN_WATER_SENSOR = D8;
// static const uint8_t PIN_WATER_SENSOR_POWER = D4;
static const uint8_t PIN_ANALOG = A0;
static const uint8_t PIN_CLIMATE_SENSOR = D3;

/*
  Output polarity

  true  = HIGH turns ON
  false = LOW turns ON
*/
static const bool FAN_ACTIVE_HIGH = true;
static const bool PUMP_ACTIVE_HIGH = true;
static const bool SOIL_SENSOR_POWER_ACTIVE_HIGH = true;
static const bool SOIL_SENSOR_SIGNAL_ACTIVE_LOW = false;
static const bool WATER_SENSOR_SIGNAL_ACTIVE_LOW = true;

/*
  Fan climate thresholds

  Different on/off values prevents the fan from rapidly flickering ON/OFF
  around one exact temperature or humidity value.
*/
static const float FAN_TEMP_ON_C = 28.0;
static const float FAN_TEMP_OFF_C = 25.0;
static const float FAN_HUMIDITY_ON_PCT = 80.0;
static const float FAN_HUMIDITY_OFF_PCT = 70.0;

/*
  Climate sensor timing

  DHT11 sensors should not be read every loop.
*/
static const unsigned long CLIMATE_SENSOR_READ_INTERVAL_MS = 2000;

/*
  Soil sensor timing

  SOIL_SENSOR_READ_INTERVAL_MS:
    how often we take a new reading

  SOIL_SENSOR_POWER_SETTLE_MS:
    how long to wait after powering the module
    before reading the sensor
*/
static const unsigned long SOIL_SENSOR_READ_INTERVAL_MS = 10000;
static const unsigned long SOIL_SENSOR_POWER_SETTLE_MS = 1000;
static const unsigned long WATER_SENSOR_READ_INTERVAL_MS = 10000;
static const unsigned long WATER_SENSOR_POWER_SETTLE_MS = 1000;

static const unsigned long SENSOR_READ_INTERVAL_MS = 10000;
static const unsigned long SENSOR_POWER_SETTLE_MS = 1000;

/*
  Serial print interval for debugging.
*/
static const unsigned long SERIAL_PRINT_INTERVAL_MS = 500;

/*
  Pump settings

  Pump runs briefly, then waits before it is allowed to run again.
  This gives water time to spread through the soil before re-checking.
*/
static const unsigned long PUMP_ON_DURATION_MS = 3000;
static const unsigned long PUMP_LOCKOUT_MS = 60000;

/*
  JSON data size
*/
static const unsigned long JSON_SIZE = 512;

/*
  Webserver update interval  
*/
static const int WEBSERVER_UPDATE_INTERVAL_MS = 20 * 1000; // post data every 20 seconds

#endif
