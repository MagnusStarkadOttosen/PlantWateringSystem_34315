#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>

/*
  MotorController
  =============
  PURPOSE:
  Hardware-only control of the motor output.

  IMPORTANT:
  This module does NOT decide timing.
  This module does NOT know about SystemState.
  This module does NOT know about project logic.

  It only:
  - initializes the output pin
  - turns the motor on or off
  - remembers current output state
*/

class MotorController {
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
  MotorController(uint8_t controlPin, bool activeHigh);

  /*
    begin()
    -------
    Configure output pin and force safe OFF state.
  */
  void begin();

  /*
    setOn()
    -------
    Apply desired motor state to hardware.
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