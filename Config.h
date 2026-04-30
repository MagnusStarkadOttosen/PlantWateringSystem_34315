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
  For the example fan:
  ON for 3 seconds
  OFF for 10 seconds
*/
static const unsigned long FAN_ON_DURATION_MS = 3000;
static const unsigned long FAN_OFF_DURATION_MS = 10000;

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
static const unsigned long SENSOR_POWER_SETTLE_MS = 10000;

/*
  Serial print interval for debugging.
*/
static const unsigned long SERIAL_PRINT_INTERVAL_MS = 500;

/*
  Pump settings
*/
static const unsigned long PUMP_ON_DURATION_MS = 3000;

#endif