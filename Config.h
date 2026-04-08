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
static const uint8_t PIN_FAN = D1;

/*
  Output polarity

  true  = HIGH turns fan ON
  false = LOW turns fan ON
*/
static const bool FAN_ACTIVE_HIGH = true;

/*
  For the example fan:
  ON for 3 seconds
  OFF for 10 seconds
*/
static const unsigned long FAN_ON_DURATION_MS = 3000;
static const unsigned long FAN_OFF_DURATION_MS = 10000;

/*
  Serial print interval for debugging.
*/
static const unsigned long SERIAL_PRINT_INTERVAL_MS = 500;

#endif