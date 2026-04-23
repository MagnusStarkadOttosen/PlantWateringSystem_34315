#ifndef WATER_LEVEL_SENSOR_H
#define WATER_LEVEL_SENSOR_H

#include <Arduino.h>

class WaterLevelSensor {
public:
  WaterLevelSensor(uint8_t signalPin, bool activeLow);

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
  void update();

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

private:
  uint8_t _signalPin;
  bool _activeLow;

  bool _isEmpty;
  int _rawValue;
};

#endif
