#include "MotorController.h"

/*
  Constructor
  -----------
  Store config only.
  No hardware access here.
*/
MotorController::MotorController(uint8_t controlPin, bool activeHigh)
  : _controlPin(controlPin),
    _activeHigh(activeHigh),
    _isOn(false) {
}

void MotorController::begin() {
  pinMode(_controlPin, OUTPUT);

  /*
    Safe startup:
    motor must start OFF unless the main application says otherwise.
  */
  applyPhysicalOutput(false);
  _isOn = false;
}

void MotorController::setOn(bool on) {
  /*
    Small guard:
    if requested state is already active, do nothing.

    WHY:
    avoids unnecessary digitalWrite() calls
    keeps the function cheap
  */
  if (_isOn == on) {
    return;
  }

  applyPhysicalOutput(on);
  _isOn = on;
}

bool MotorController::isOn() const {
  return _isOn;
}

void MotorController::applyPhysicalOutput(bool on) {
  /*
    Translate logical motor state into actual output level.

    activeHigh = true:
      ON  -> HIGH
      OFF -> LOW

    activeHigh = false:
      ON  -> LOW
      OFF -> HIGH
  */
  if (_activeHigh) {
    digitalWrite(_controlPin, on ? HIGH : LOW);
    return;
  }

  digitalWrite(_controlPin, on ? LOW : HIGH);
}