#ifndef WATER_LEVEL_SENSOR_H
#define WATER_LEVEL_SENSOR_H

#include <Arduino.h>

class WaterLevelSensor {
public:
  WaterLevelSensor(uint8_t signalPin, uint8_t powerPin, bool activeLow);

  /*
    begin()
    -------
    Configure the input pin.
  */
  void begin();

  /*
    update()
    --------
    Read the sensor and store latest interpreted state.
  */
  void update(unsigned long nowMs);

  /*
    isDry()
    -------
    Returns the latest logical dry state.
  */
  bool isEmpty() const;

  /*
    getRawValue()
    -------------
    Returns the last raw digital reading from the pin.
  */
  int getRawValue() const;

  /*
    isPowered()
    -----------
    Returns if the sensor is recieving power
  */
  bool isPowered() const;

private:
  uint8_t _signalPin;
  uint8_t _powerPin;
  bool _activeLow;

  bool _isEmpty;
  int _rawValue;
  
  bool _isPowered;
  unsigned long _lastReadMs;
  unsigned long _powerOnMs;
};

#endif
