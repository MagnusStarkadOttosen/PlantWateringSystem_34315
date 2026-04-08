#ifndef FAN_CONTROLLER_H
#define FAN_CONTROLLER_H

#include <Arduino.h>

/*
  FanController
  =============
  PURPOSE:
  Hardware-only control of the fan output.

  IMPORTANT:
  This module does NOT decide timing.
  This module does NOT know about SystemState.
  This module does NOT know about project logic.

  It only:
  - initializes the output pin
  - turns the fan on or off
  - remembers current output state
*/

class FanController {
public:
  /*
    Constructor
    -----------
    controlPin:
      pin that drives transistor base resistor or relay input

    activeHigh:
      true  => HIGH means ON
      false => LOW means ON
  */
  FanController(uint8_t controlPin, bool activeHigh);

  /*
    begin()
    -------
    Configure output pin and force safe OFF state.
  */
  void begin();

  /*
    setOn()
    -------
    Apply desired fan state to hardware.
  */
  void setOn(bool on);

  /*
    isOn()
    ------
    Return last applied logical state.
  */
  bool isOn() const;

private:
  /*
    applyPhysicalOutput()
    ---------------------
    Translate logical ON/OFF to actual pin level.
  */
  void applyPhysicalOutput(bool on);

  uint8_t _controlPin;
  bool _activeHigh;
  bool _isOn;
};

#endif