#include "SoilMoistureSensor.h"
#include "Config.h"

SoilMoistureSensor::SoilMoistureSensor(uint8_t signalPin, uint8_t powerPin, bool activeLow)
  : _signalPin(signalPin),
    _powerPin(powerPin),
    _activeLow(activeLow),
    _isDry(false),
    _rawValue(HIGH),
    _isPowered(false),
    _lastReadMs(0),
    _powerOnMs(0) {
}

void SoilMoistureSensor::begin() {
  pinMode(_signalPin, INPUT);
  pinMode(_powerPin, OUTPUT);

  digitalWrite(_powerPin, LOW);
  _isPowered = false;
}

void SoilMoistureSensor::update(unsigned long nowMs) {
  /*
    PHASE 1:
    If sensor is OFF, only power it on when it is time for a new reading.
  */
  if (!_isPowered) {
    if (nowMs - _lastReadMs < SOIL_SENSOR_READ_INTERVAL_MS) {
      return;
    }

    digitalWrite(_powerPin, HIGH);
    _isPowered = true;
    _powerOnMs = nowMs;
    return;
  }

  /*
    PHASE 2:
    Sensor is ON, wait for it to stabilize.
  */
  if (nowMs - _powerOnMs < SOIL_SENSOR_POWER_SETTLE_MS) {
    return;
  }

  /*
    PHASE 3:
    Read once, store result, then power sensor back OFF.
  */
  _rawValue = digitalRead(_signalPin);

  if (_activeLow) {
    _isDry = (_rawValue == LOW);
  } else {
    _isDry = (_rawValue == HIGH);
  }

  digitalWrite(_powerPin, LOW);
  _isPowered = false;
  _lastReadMs = nowMs;
}

bool SoilMoistureSensor::isDry() const {
  return _isDry;
}

int SoilMoistureSensor::getRawValue() const {
  return _rawValue;
}

bool SoilMoistureSensor::isPowered() const {
  return _isPowered;
}