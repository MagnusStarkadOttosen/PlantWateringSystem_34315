#include "FanController.h"

/*
  Constructor
  -----------
  Store config only.
  No hardware access here.
*/
FanController::FanController(uint8_t controlPin, bool activeHigh)
  : _controlPin(controlPin),
    _activeHigh(activeHigh),
    _isOn(false) {
}

void FanController::begin() {
  pinMode(_controlPin, OUTPUT);

  /*
    Safe startup:
    fan must start OFF unless the main application says otherwise.
  */
  applyPhysicalOutput(false);
  _isOn = false;
}

void FanController::setOn(bool on) {
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

bool FanController::isOn() const {
  return _isOn;
}

void FanController::applyPhysicalOutput(bool on) {
  /*
    Translate logical fan state into actual output level.

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