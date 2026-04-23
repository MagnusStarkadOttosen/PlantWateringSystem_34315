#include "SensorPower.h"

SensorPower::SensorPower(uint8_t powerPin, bool activeHigh)
  : _powerPin(powerPin), _activeHigh(activeHigh), _isOn(false) {
}

void SensorPower::begin() {
  pinMode(_powerPin, OUTPUT);
  setOn(false);
}

void SensorPower::setOn(bool on) {
  _isOn = on;
  digitalWrite(_powerPin, (_activeHigh ? on : !on) ? HIGH : LOW);
}

bool SensorPower::isOn() const {
  return _isOn;
}