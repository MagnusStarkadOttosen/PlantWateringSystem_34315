#include "Logic.h"
#include "Config.h"

/*
  INTERNAL HELPER FUNCTIONS
  =========================
  These are private to Logic.cpp.
  PlantWateringSystem_34315.ino does not know they exist.
  Other files do not need to call them directly.
*/

static void updateFanLogic(SystemState& state);
static void updateWateringLogic(SystemState& state);
static void applySafetyOverrides(SystemState& state);

/*
  initializeLogic()
  -----------------
  Sets up initial values for logic-owned state.

  IMPORTANT:
  Only initialize things here that belong to the logic layer,
  not hardware setup.
*/
void initializeLogic(SystemState& state) {
  /*
    Fan example cycle starts enabled.
    Start in ON phase so the example is visible immediately.
  */
  state.fanCycleEnabled = true;
  state.fanCycleIsOnPhase = true;
  state.fanPhaseStartMs = state.nowMs;
  state.fanCommand = true;

  /*
    Pump defaults.
    Doesn't exist yet.
  */
  // state.pumpCommand = false;
  // state.pumpCycleEnabled = true;
  // state.pumpPhaseStartMs = state.nowMs;
}

/*
  updateLogic()
  -------------
  This is the ONLY function PlantWateringSystem_34315.ino needs to call.

  It should stay short and boring.
  Its job is orchestration, not detailed logic.

  ORDER MATTERS:
  1. compute normal behavior
  2. apply safety overrides last
*/
void updateLogic(SystemState& state) {
  updateFanLogic(state);
  updateWateringLogic(state);
  applySafetyOverrides(state);
}

/*
  updateFanLogic()
  ----------------
  Handles fan behavior only.

  For now:
  - ON for FAN_ON_DURATION_MS
  - OFF for FAN_OFF_DURATION_MS
  - repeat

  Early returns are safe here because this function owns only fan logic.
*/
static void updateFanLogic(SystemState& state) {
  /*
    GUARD 1:
    If fan cycling is disabled, fan should be OFF.
  */
  if (!state.fanCycleEnabled) {
    state.fanCommand = false;
    return;
  }

  /*
    GUARD 2:
    Stay in ON phase until ON duration expires.
  */
  if (state.fanCycleIsOnPhase &&
      (state.nowMs - state.fanPhaseStartMs < FAN_ON_DURATION_MS)) {
    state.fanCommand = true;
    return;
  }

  /*
    GUARD 3:
    Stay in OFF phase until OFF duration expires.
  */
  if (!state.fanCycleIsOnPhase &&
      (state.nowMs - state.fanPhaseStartMs < FAN_OFF_DURATION_MS)) {
    state.fanCommand = false;
    return;
  }

  /*
    If we got here, the current phase expired.
    Toggle phase and restart timer.
  */
  state.fanCycleIsOnPhase = !state.fanCycleIsOnPhase;
  state.fanPhaseStartMs = state.nowMs;
  state.fanCommand = state.fanCycleIsOnPhase;
}

/*
  updateWateringLogic()
  ---------------------
  Handles pump/watering behavior only.

  Right now this is just a placeholder so the architecture is ready.
*/
static void updateWateringLogic(SystemState& state) {
  /*
    Placeholder version:
    force pump OFF until real watering logic is implemented.
  */
  // state.pumpCommand = false;

  /*
    Example structure for later:

    if (!state.wateringEnabled) {
      state.pumpCommand = false;
      return;
    }

    if (state.waterTankLow) {
      state.pumpCommand = false;
      return;
    }

    if (state.soilPercent >= SOIL_DRY_THRESHOLD_PERCENT) {
      state.pumpCommand = false;
      return;
    }

    state.pumpCommand = true;
  */
  (void)state;
}

/*
  applySafetyOverrides()
  ----------------------
  Final safety pass.

  This function should override normal commands if something unsafe happens.

  WHY LAST:
  Because safety should have final authority.

  Example future rules:
  - no pump if tank is empty
  - no fan if sensor invalid
  - shut everything down on fault
*/
static void applySafetyOverrides(SystemState& state) {
  /*
    Placeholder version for now.
    Add overrides as real state fields become available.

    Example later:

    if (state.waterTankLow) {
      state.pumpCommand = false;
    }

    if (!state.climateValid) {
      state.fanCommand = false;
    }
  */
  (void)state;
}