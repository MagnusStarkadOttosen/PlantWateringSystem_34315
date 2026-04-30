#include "Config.h"
#include "Secrets.h"
#include "SystemState.h"
#include "Logic.h"
#include "MotorController.h"
#include "SoilMoistureSensor.h"
#include "ESP8266WiFi.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "WaterLevelSensor.h"
#include "DisplayController.h"
#include <Wire.h>
#include "SensorPower.h"
#include "ClimateSensor.h"

/*
  MAIN APPLICATION
  ================
  This file should be boring.

  It should only:
  1. initialize modules
  2. refresh shared time/state
  3. run logic
  4. apply logic outputs to actuators
  5. optionally print debug information

  If this file starts containing hardware-specific hacks
  or business logic everywhere, the architecture is breaking.
*/
WiFiClient client;
HTTPClient http;

SystemState state;
SensorPower sensorPower(PIN_SOIL_SENSOR_POWER, SOIL_SENSOR_POWER_ACTIVE_HIGH);
MotorController fan(PIN_FAN, FAN_ACTIVE_HIGH);
MotorController pump(PIN_PUMP, PUMP_ACTIVE_HIGH);
SoilMoistureSensor soilSensor(PIN_SOIL_SENSOR, PIN_ANALOG, SOIL_SENSOR_SIGNAL_ACTIVE_LOW);
WaterLevelSensor waterLevelSensor(PIN_WATER_SENSOR, WATER_SENSOR_SIGNAL_ACTIVE_LOW);

unsigned long lastPrintMs = 0;
unsigned long loopCounter = 0;
unsigned long loopsPerSecond = 0;

static const char* onOff(bool value) {
  return value ? "ON " : "OFF";
}

static const char* yesNo(bool value) {
  return value ? "YES" : "NO ";
}

void setup() {
  Serial.begin(115200);

  Serial.println();

  #if !defined(WIFI_SSID) || !defined(WIFI_PASSWORD)  
    Serial.println("WiFi credentials missing!");

    Serial.println("Starting without WiFi functionality...");

  #else

    // Connecting to WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
      if (state.nowMs - lastPrintMs < SERIAL_PRINT_INTERVAL_MS) {
        return;
      }

      lastPrintMs = state.nowMs;
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
  #endif
  
  Wire.begin(); 
  /*
    Snapshot current time before initializing logic.
    This gives logic a sane starting timestamp.
  */
  state.nowMs = millis();

  fan.begin();
  pump.begin();

  sensorPower.begin();
  soilSensor.begin();
  waterLevelSensor.begin();
  dhtBegin();

  initializeLogic(state);
  initDisplay();
}

void loop() {
  loopCounter++;
  /*
    One shared time snapshot for this loop iteration.
  */
  state.nowMs = millis();

  /*
    Water and soil
    is like this because it shares power pin
  */
  static unsigned long lastSensorReadMs = 0;
  static bool sensorPowerOnPhase = false;
  static unsigned long sensorPowerOnMs = 0;

  if (!sensorPowerOnPhase) {
    if (state.nowMs - lastSensorReadMs >= SENSOR_READ_INTERVAL_MS) {
      sensorPower.setOn(true);
      sensorPowerOnMs = state.nowMs;
      sensorPowerOnPhase = true;
    }
  } else {
    if (state.nowMs - sensorPowerOnMs >= SENSOR_POWER_SETTLE_MS) {
      soilSensor.update();
      waterLevelSensor.update();

      state.soilDry = soilSensor.isDry();
      state.isEmpty = waterLevelSensor.isEmpty();

      sensorPower.setOn(false);
      sensorPowerOnPhase = false;
      lastSensorReadMs = state.nowMs;
    }
  }

  {
    float temperatureC = NAN;
    float humidityPct = NAN;

    dhtRead(temperatureC, humidityPct);

    state.temperatureC = temperatureC;
    state.humidityPct = humidityPct;
    state.climateValid = !isnan(temperatureC) && !isnan(humidityPct);
  }

  /*
    Update decision logic.
    This writes desired outputs into SystemState.
  */
  updateLogic(state, client, http);

  /*
    Apply desired logic output to hardware.
  */
  fan.setOn(state.fanActive);
  pump.setOn(state.pumpActive);

   /*
    Updates LCD Display
  */
  updateDisplay(state);

  /*
    Debug print guard.
  */
  if (state.nowMs - lastPrintMs < SERIAL_PRINT_INTERVAL_MS) {
    return;
  }

  loopsPerSecond = loopCounter * (1000/SERIAL_PRINT_INTERVAL_MS);
  loopCounter = 0;
  lastPrintMs = state.nowMs;

  Serial.print("[");
  Serial.print(loopsPerSecond);
  Serial.print(" UPS] ");

  Serial.print("Fan:");
  Serial.print(onOff(state.fanActive));
  Serial.print("  ");

  Serial.print("Power water/soil:");
  Serial.print(onOff(sensorPowerOnPhase));
  Serial.print("  ");

  Serial.print("SoilDry:");
  Serial.print(yesNo(state.soilDry));
  Serial.print("  ");

  Serial.print("WaterEmpty:");
  Serial.print(yesNo(state.isEmpty));
  Serial.print("  ");

  Serial.print("Pump:");
  Serial.print(onOff(state.pumpActive));
  Serial.print("  ");

  Serial.print("SoilDig:");
  Serial.print(soilSensor.getDigitalValue() == HIGH ? "HIGH" : "LOW ");
  Serial.print("  ");

  Serial.print("SoilAna:");
  Serial.print(soilSensor.getAnalogValue());
  Serial.print("  ");

  Serial.print("Temp:");
  if (state.climateValid) {
    Serial.print(state.temperatureC);
    Serial.print(" C");
  } else {
    Serial.print("ERR ");
  }
  Serial.print("  ");

  Serial.print("Humidity:");
  if (state.climateValid) {
    Serial.print(state.humidityPct);
    Serial.print(" %");
  } else {
    Serial.print("ERR ");
  }
  Serial.print("  ");

  Serial.print("PumpTimer:");
  if (state.pumpActive) {
    unsigned long elapsedMs = state.nowMs - state.pumpStartTime;
    Serial.print(elapsedMs);
    Serial.print(" / ");
    Serial.print(PUMP_ON_DURATION_MS);
    Serial.print(" ms");
  } else {
    Serial.print("-        ");
  }

  Serial.println();
}