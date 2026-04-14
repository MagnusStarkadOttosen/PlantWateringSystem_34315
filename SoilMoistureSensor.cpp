#include "SoilMoistureSensor.h"

SoilMoistureSensor::SoilMoistureSensor(uint8_t signalPin, bool activeLow)
  : _signalPin(signalPin),
    _activeLow(activeLow),
    _isDry(false),
    _rawValue(HIGH) {
}

void SoilMoistureSensor::begin() {
  pinMode(_signalPin, INPUT);

  update();
}

void SoilMoistureSensor::update() {
  _rawValue = digitalRead(_signalPin);

  /*
    Translate electrical level into logical "dry" meaning.

    activeLow = true:
      LOW  => dry
      HIGH => not dry

    activeLow = false:
      HIGH => dry
      LOW  => not dry
  */
  if (_activeLow) {
    _isDry = (_rawValue == LOW);
    return;
  }

  _isDry = (_rawValue == HIGH);
}

bool SoilMoistureSensor::isDry() const {
  return _isDry;
}

int SoilMoistureSensor::getRawValue() const {
  return _rawValue;
}