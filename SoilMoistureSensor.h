#ifndef SOIL_MOISTURE_SENSOR_H
#define SOIL_MOISTURE_SENSOR_H

#include <Arduino.h>

class SoilMoistureSensor {
public:
  SoilMoistureSensor(uint8_t signalPin, uint8_t powerPin, bool signalActiveLow, bool powerActiveHigh);

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
  bool isDry() const;

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
  bool _signalActiveLow;
  bool _powerActiveHigh;

  bool _isDry;
  int _rawValue;
  
  bool _isPowered;
  unsigned long _lastReadMs;
  unsigned long _powerOnMs;
};

#endif