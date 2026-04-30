#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>

/*
  SystemState
  ===========
  Shared project state.

  PURPOSE:
  - sensors write readings here
  - logic reads/writes decisions here
  - actuators read commands from here

  This prevents random global variables everywhere.
*/

struct SystemState {
  /*
    Time snapshot for current loop iteration.

    WHY:
    We want one shared "now" value per loop.
    That keeps timing consistent and avoids every file calling millis()
    for unrelated decisions.
  */
  unsigned long nowMs = 0;

/*
  Fan command produced by Logic.cpp.

  This is NOT the hardware state directly.
  It is the desired logical state:
  true  => fan should be ON
  false => fan should be OFF
*/
bool fanActive = false;

/*
  Soil moisture sensor
*/
  bool soilDry = false;

  /*
    Water level sensor
  */
  bool isEmpty = true;
  
/*
  Pump control

  pumpLockoutUntilMs:
    while nowMs is before this time, the pump may not start again.
*/
bool pumpEnabled = true;
unsigned long pumpStartTime = 0;
unsigned long pumpLockoutUntilMs = 0;
bool pumpActive = false;

  /*
    Climate sensor
  */
  float temperatureC = NAN;
  float humidityPct = NAN;
  bool climateValid = false;
};

#endif