#include "SoilMoistureSensor.h"
#include "Config.h"

SoilMoistureSensor::SoilMoistureSensor(uint8_t signalPin, uint8_t powerPin, bool signalActiveLow, bool powerActiveHigh)
  : _signalPin(signalPin),
    _powerPin(powerPin),
    _signalActiveLow(signalActiveLow),
    _powerActiveHigh(powerActiveHigh),
    _isDry(false),
    _rawValue(HIGH),
    _isPowered(false),
    _lastReadMs(0),
    _powerOnMs(0) {
}

void SoilMoistureSensor::begin() {
  pinMode(_signalPin, INPUT);
  pinMode(_powerPin, OUTPUT);

  digitalWrite(_powerPin, _powerActiveHigh ? LOW : HIGH);
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

    digitalWrite(_powerPin, _powerActiveHigh ? HIGH : LOW);
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

  if (_signalActiveLow) {
    _isDry = (_rawValue == LOW);
  } else {
    _isDry = (_rawValue == HIGH);
  }

  digitalWrite(_powerPin, _powerActiveHigh ? LOW : HIGH);
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