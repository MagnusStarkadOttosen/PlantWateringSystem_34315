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
  bool fanCommand = false;

  /*
    Timing memory for the fan cycle logic.
    Since the cycle behavior belongs in Logic.cpp,
    the timing state also belongs here.

    fanPhaseStartMs:
      remembers when the current ON or OFF phase began
  */
  unsigned long fanPhaseStartMs = 0;

  /*
    Current phase of the repeating pattern.

    true  => currently in ON phase
    false => currently in OFF phase
  */
  bool fanCycleIsOnPhase = false;

  /*
    Optional master enable.
    Lets you stop the cycle from main or from future logic.
  */
  bool fanCycleEnabled = true;
};

#endif