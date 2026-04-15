#include "Config.h"
#include "Secrets.h"
#include "SystemState.h"
#include "Logic.h"
#include "MotorController.h"
#include "SoilMoistureSensor.h"
#include "ESP8266WiFi.h"

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

SystemState state;
MotorController fan(PIN_FAN, FAN_ACTIVE_HIGH);
MotorController pump(PIN_PUMP, PUMP_ACTIVE_HIGH);
SoilMoistureSensor soilSensor(PIN_SOIL_SENSOR, PIN_SOIL_SENSOR_POWER, SOIL_SENSOR_SIGNAL_ACTIVE_LOW, SOIL_SENSOR_POWER_ACTIVE_HIGH);

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

  /*
    Snapshot current time before initializing logic.
    This gives logic a sane starting timestamp.
  */
  state.nowMs = millis();

  fan.begin();
  pump.begin();
  soilSensor.begin();

  initializeLogic(state);
}

void loop() {
  loopCounter++;
  /*
    One shared time snapshot for this loop iteration.
  */
  state.nowMs = millis();

  /*
    Update sensor values
  */
  soilSensor.update(state.nowMs);

  /*
    Copy sensor values into shared state
  */
  state.soilDry = soilSensor.isDry();

  /*
    Update decision logic.
    This writes desired outputs into SystemState.
  */
  updateLogic(state, client);

  /*
    Apply desired logic output to hardware.
  */
  fan.setOn(state.fanActive);
  pump.setOn(state.pumpActive);

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

  Serial.print("SoilPower:");
  Serial.print(onOff(soilSensor.isPowered()));
  Serial.print("  ");

  Serial.print("SoilDry:");
  Serial.print(yesNo(state.soilDry));
  Serial.print("  ");

  Serial.print("Pump:");
  Serial.print(onOff(state.pumpActive));
  Serial.print("  ");

  Serial.print("SoilRaw:");
  Serial.print(soilSensor.getRawValue() == HIGH ? "HIGH" : "LOW ");
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