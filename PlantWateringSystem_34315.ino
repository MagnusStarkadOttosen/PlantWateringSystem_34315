#include "Config.h"
#include "SystemState.h"
#include "Logic.h"
#include "FanController.h"

/*
  MAIN APPLICATION
  ================
  This file should be boring.

  It should only:
  1. initialize modules
  2. refresh shared time/state
  3. run logic
  4. apply logic outputs to actuators
  5. optionally print debug information

  If this file starts containing hardware-specific hacks
  or business logic everywhere, the architecture is breaking.
*/

SystemState state;
FanController fan(PIN_FAN, FAN_ACTIVE_HIGH);

unsigned long lastPrintMs = 0;

void setup() {
  Serial.begin(115200);

  /*
    Snapshot current time before initializing logic.
    This gives logic a sane starting timestamp.
  */
  state.nowMs = millis();

  fan.begin();
  initializeLogic(state);
}

void loop() {
  /*
    One shared time snapshot for this loop iteration.
  */
  state.nowMs = millis();

  /*
    Update decision logic.
    This writes desired outputs into SystemState.
  */
  updateLogic(state);

  /*
    Apply desired logic output to hardware.
  */
  fan.setOn(state.fanCommand);

  /*
    Debug print guard.
  */
  if (state.nowMs - lastPrintMs < SERIAL_PRINT_INTERVAL_MS) {
    return;
  }

  lastPrintMs = state.nowMs;

  Serial.print("Cycle enabled: ");
  Serial.print(state.fanCycleEnabled ? "YES" : "NO");

  Serial.print(" | Phase: ");
  Serial.print(state.fanCycleIsOnPhase ? "ON-PHASE" : "OFF-PHASE");

  Serial.print(" | Command: ");
  Serial.println(state.fanCommand ? "ON" : "OFF");
}