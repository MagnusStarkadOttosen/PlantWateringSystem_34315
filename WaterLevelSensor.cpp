// // Sensor input taken on pin A0 on ESP12, Power provided by digital pin D1 so that the sensor is not on all the time

// void setup() {
//   // put your setup code here, to run once:
//   Serial.begin(115200);
//   pinMode(A0, INPUT);
//   pinMode(D1, OUTPUT);
//   digitalWrite(D1, LOW);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   digitalWrite(D1, HIGH);
//   delay(50);
//   int sensorValue = analogRead(A0);
//   digitalWrite(D1, LOW);

//   // // Convert the sensor value to a percentage (0 to 100%)
//   float waterLevelPercentage = (sensorValue / 4095.0) * 100;

//   // Print the sensor value and water level percentage to the serial monitor
//   Serial.print("Sensor Value: ");
//   Serial.print(sensorValue);
//   Serial.print("  Water Level Percentage: ");
//   Serial.println(waterLevelPercentage);
//   delay(2000);
// }

#include "WaterLevelSensor.h"
#include "Config.h"

WaterLevelSensor::WaterLevelSensor(uint8_t signalPin, uint8_t powerPin, bool activeLow)
  : _signalPin(signalPin),
    _powerPin(powerPin),
    _activeLow(activeLow),
    _isEmpty(false),
    _rawValue(4095),
    _isPowered(false),
    _lastReadMs(0),
    _powerOnMs(0) {
}

void WaterLevelSensor::begin() {
  pinMode(_signalPin, INPUT);
  pinMode(_powerPin, OUTPUT);

  digitalWrite(_powerPin, LOW);
  _isPowered = false;
}

void WaterLevelSensor::update(unsigned long nowMs) {
  /*
    PHASE 1:
    If sensor is OFF, only power it on when it is time for a new reading.
  */
  if (!_isPowered) {
    if (nowMs - _lastReadMs < WATER_SENSOR_READ_INTERVAL_MS) {
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
  if (nowMs - _powerOnMs < WATER_SENSOR_POWER_SETTLE_MS) {
    return;
  }

  /*
    PHASE 3:
    Read once, store result, then power sensor back OFF.
  */
  _rawValue = analogRead(_signalPin);

  if (_activeLow) {
    _isEmpty = (_rawValue <= 2);
  } else {
    _isEmpty = (_rawValue >= 4070);
  }

  digitalWrite(_powerPin, LOW);
  _isPowered = false;
  _lastReadMs = nowMs;
}

bool WaterLevelSensor::isEmpty() const {
  return _isEmpty;
}

int WaterLevelSensor::getRawValue() const {
  return _rawValue;
}

bool WaterLevelSensor::isPowered() const {
  return _isPowered;
}
