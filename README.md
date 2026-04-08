# Plant Watering System — Rules and Implementation Guide

This document defines **how the project must be structured and implemented**.

The goal is not just to make something that works -
the goal is to make something that is:

* modular
* testable
* easy to integrate
* easy to extend

---

# 1. Core Idea (Read This First)

We should be building a system with **clear separation of responsibilities**:

| Layer       | Responsibility     |
| ----------- | ------------------ |
| Modules     | Talk to hardware   |
| Logic       | Decide behavior    |
| SystemState | Share data         |
| Main        | Connect everything |

---

# 2. Final Architecture Overview

```
Sensors → SystemState → Logic → SystemState → Actuators
```

More explicitly:

1. Sensors update values in `SystemState`
2. Logic reads those values and decides what should happen
3. Logic writes commands back into `SystemState`
4. Main applies those commands to actuators

---

# 3. File Structure (Arduino IDE)

```
PlantWateringSystem/
├── PlantWateringSystem.ino
├── Config.h
├── SystemState.h
├── Logic.h
├── Logic.cpp
├── SoilMoistureSensor.h
├── SoilMoistureSensor.cpp
├── WaterLevelSensor.h
├── WaterLevelSensor.cpp
├── ClimateSensor.h
├── ClimateSensor.cpp
├── PumpController.h
├── PumpController.cpp
├── FanController.h
├── FanController.cpp
└── README.md
```

---

# 4. SystemState (Shared Data)

All shared data lives in `SystemState`.

Example:

```
struct SystemState {
  unsigned long nowMs;

  // Sensor values
  float soilPercent;
  bool waterTankLow;
  float temperatureC;
  float humidityPercent;

  // Commands from logic
  bool pumpCommand;
  bool fanCommand;

  // Optional feedback
  bool pumpActualState;
  bool fanActualState;

  // Logic timing memory (example: fan cycle)
  bool fanCycleEnabled;
  bool fanCycleIsOnPhase;
  unsigned long fanPhaseStartMs;
};
```

### Rule

* Sensors WRITE values
* Logic READS and WRITES
* Actuators READ commands only

---

# 5. Logic Architecture

We use **one public function**:

```
updateLogic(state)
```

But internally it is split into smaller functions.

## Correct structure

```
updateLogic(state)
  ├── updateFanLogic(state)
  ├── updateWateringLogic(state)
  └── applySafetyOverrides(state)
```

---

## Why this matters

### BAD (monolithic logic)

```
updateLogic() {
  if (...) return;
  if (...) return;

  // fan logic
  // pump logic
}
```

Problem:

* early return breaks unrelated logic

---

### GOOD (split logic)

```
updateLogic() {
  updateFanLogic(state);
  updateWateringLogic(state);
}
```

Each function is independent.

---

# 6. Early Return Rule (CRITICAL)

Early returns are allowed, but only inside **single-responsibility functions**.

### GOOD

```
void updateFanLogic(...) {
  if (!enabled) return;
  if (stillInPhase) return;

  // change state
}
```

### BAD

```
void updateLogic(...) {
  if (!enabled) return; // stops everything
}
```

---

# 7. Logic Responsibilities

Logic decides:

* when to water
* when to run fan
* safety rules
* timing behavior

Logic does NOT:

* call digitalWrite
* read pins directly
* know hardware details

---

# 8. Module Responsibilities

Modules:

* control hardware
* read sensors
* apply outputs

Modules do NOT:

* make decisions
* depend on other modules
* read SystemState

---

# 9. Module Rules (MANDATORY)

## Rule 1: No hardcoded pins

Pins only exist in `Config.h`

---

## Rule 2: Required structure

Each module must have:

```
constructor(...)
begin()
update()
getters
setters
```

---

## Rule 3: No delay()

Never use:

```
delay(...)
```

Use:

```
if (nowMs - last < interval) return;
```

---

## Rule 4: No cross-module logic

Modules do NOT:

* read other modules
* call logic
* contain system behavior

---

## Rule 5: Must work standalone

Each module must have:

* `.h`
* `.cpp`
* test sketch

---

# 10. Main File (Orchestrator Only)

Main should look like this:

```
void loop() {
  state.nowMs = millis();

  // Update sensors
  soil.update();
  tank.update();
  climate.update();

  // Copy sensor values into state
  state.soilPercent = soil.getPercent();
  state.waterTankLow = tank.isLow();
  state.temperatureC = climate.getTemperatureC();

  // Run logic
  updateLogic(state);

  // Apply outputs
  pump.setOn(state.pumpCommand);
  fan.setOn(state.fanCommand);
}
```

---

# 11. Timing Rules (millis)

Always use guard + early return:

```
if (nowMs - lastTime < interval) {
  return;
}
```

Never:

```
delay(...)
```

---

# 12. Safety Pattern

Always apply safety LAST:

```
updateFanLogic(state);
updateWateringLogic(state);
applySafetyOverrides(state);
```

Example:

```
if (state.waterTankLow) {
  state.pumpCommand = false;
}
```

---

END OF DOCUMENT
