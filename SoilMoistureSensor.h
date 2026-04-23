#ifndef SOIL_MOISTURE_SENSOR_H
#define SOIL_MOISTURE_SENSOR_H

#include <Arduino.h>

class SoilMoistureSensor {
public:
  SoilMoistureSensor(uint8_t digitalPin, uint8_t analogPin, bool signalActiveLow);

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
  bool isDry() const;

  /*
    getDigitalValue()
    -------------
    Returns the last raw digital reading from the pin.
  */
  int getDigitalValue() const;

  int getAnalogValue() const;

private:
  uint8_t _digitalPin;
  uint8_t _analogPin;
  bool _signalActiveLow;

  bool _isDry;
  int _digitalValue;
  int _analogValue;
};

#endif