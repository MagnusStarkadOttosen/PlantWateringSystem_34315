#include "SoilMoistureSensor.h"
#include "Config.h"

SoilMoistureSensor::SoilMoistureSensor(uint8_t digitalPin, uint8_t analogPin, bool signalActiveLow)
  : _digitalPin(digitalPin),
    _analogPin(analogPin),
    _signalActiveLow(signalActiveLow),
    _isDry(false),
    _digitalValue(HIGH),
    _analogValue(0) {
}

void SoilMoistureSensor::begin() {
  pinMode(_digitalPin, INPUT);
}

void SoilMoistureSensor::update() {

  _digitalValue = digitalRead(_digitalPin);
  _analogValue = analogRead(_analogPin);

  if (_signalActiveLow) {
    _isDry = (_digitalValue == LOW);
  } else {
    _isDry = (_digitalValue == HIGH);
  }
}

bool SoilMoistureSensor::isDry() const {
  return _isDry;
}

int SoilMoistureSensor::getDigitalValue() const {
  return _digitalValue;
}

int SoilMoistureSensor::getAnalogValue() const {
  return _analogValue;
}