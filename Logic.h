#ifndef LOGIC_H
#define LOGIC_H

#include <Arduino.h>
#include "SystemState.h"

/*
  Logic layer
  ===========
  Public API stays simple:
  PlantWateringSystem_34315.ino only calls updateLogic(state)

  Internally, updateLogic() fans out into smaller logic functions.
  This keeps main simple while avoiding one giant monolithic logic block.
*/

/*
  initializeLogic()
  -----------------
  Optional one-time setup for logic-owned state.
*/
void initializeLogic(SystemState& state);

/*
  updateLogic()
  -------------
  Main entry point called from loop().
  This function should:
  - call smaller logic functions
  - keep orchestration centralized
  - avoid becoming one giant nested block
*/
void updateLogic(SystemState& state);

#endif