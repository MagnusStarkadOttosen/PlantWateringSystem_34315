#ifndef SENSOR_POWER_H
#define SENSOR_POWER_H

#include <Arduino.h>

class SensorPower {
public:
  SensorPower(uint8_t powerPin, bool activeHigh);

  void begin();
  void setOn(bool on);
  bool isOn() const;

private:
  uint8_t _powerPin;
  bool _activeHigh;
  bool _isOn;
};

#endif