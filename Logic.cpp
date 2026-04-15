#include "Logic.h"
#include "Config.h"
#include "Secrets.h"
#include "ArduinoJson.h"
#include "ThingSpeak.h"

/*
  INTERNAL HELPER FUNCTIONS
  =========================
  These are private to Logic.cpp.
  PlantWateringSystem_34315.ino does not know they exist.
  Other files do not need to call them directly.
*/

static void updateFanLogic(SystemState& state);
static void updateWateringLogic(SystemState& state);
static void applySafetyOverrides(SystemState& state);
static void updateWebserver(SystemState& state, WiFiClient& client);
static void generateJSON(StaticJsonDocument<JSON_SIZE>& data, SystemState& state);
static void sendToThingspeak(StaticJsonDocument<JSON_SIZE>& data, WiFiClient& client);

/*
  initializeLogic()
  -----------------
  Sets up initial values for logic-owned state.

  IMPORTANT:
  Only initialize things here that belong to the logic layer,
  not hardware setup.
*/
void initializeLogic(SystemState& state) {
  /*
    Fan example cycle starts enabled.
    Start in ON phase so the example is visible immediately.
  */
  state.fanCycleEnabled = true;
  state.fanCycleIsOnPhase = true;
  state.fanPhaseStartMs = state.nowMs;
  state.fanCommand = true;

  /*
    Pump defaults.
    Doesn't exist yet.
  */
  // state.pumpCommand = false;
  // state.pumpCycleEnabled = true;
  // state.pumpPhaseStartMs = state.nowMs;
}

/*
  updateLogic()
  -------------
  This is the ONLY function PlantWateringSystem_34315.ino needs to call.

  It should stay short and boring.
  Its job is orchestration, not detailed logic.

  ORDER MATTERS:
  1. compute normal behavior
  2. apply safety overrides last
*/
void updateLogic(SystemState& state, WiFiClient& client) {
  updateFanLogic(state);
  updateWateringLogic(state);
  applySafetyOverrides(state);
  updateWebserver(state, client);
}

/*
  updateFanLogic()
  ----------------
  Handles fan behavior only.

  For now:
  - ON for FAN_ON_DURATION_MS
  - OFF for FAN_OFF_DURATION_MS
  - repeat

  Early returns are safe here because this function owns only fan logic.
*/
static void updateFanLogic(SystemState& state) {
  /*
    GUARD 1:
    If fan cycling is disabled, fan should be OFF.
  */
  if (!state.fanCycleEnabled) {
    state.fanCommand = false;
    return;
  }

  /*
    GUARD 2:
    Stay in ON phase until ON duration expires.
  */
  if (state.fanCycleIsOnPhase &&
      (state.nowMs - state.fanPhaseStartMs < FAN_ON_DURATION_MS)) {
    state.fanCommand = true;
    return;
  }

  /*
    GUARD 3:
    Stay in OFF phase until OFF duration expires.
  */
  if (!state.fanCycleIsOnPhase &&
      (state.nowMs - state.fanPhaseStartMs < FAN_OFF_DURATION_MS)) {
    state.fanCommand = false;
    return;
  }

  /*
    If we got here, the current phase expired.
    Toggle phase and restart timer.
  */
  state.fanCycleIsOnPhase = !state.fanCycleIsOnPhase;
  state.fanPhaseStartMs = state.nowMs;
  state.fanCommand = state.fanCycleIsOnPhase;
}

/*
  updateWateringLogic()
  ---------------------
  Handles pump/watering behavior only.

  Right now this is just a placeholder so the architecture is ready.
*/
static void updateWateringLogic(SystemState& state) {
  /*
    Placeholder version:
    force pump OFF until real watering logic is implemented.
  */
  // state.pumpCommand = false;

  /*
    Example structure for later:

    if (!state.wateringEnabled) {
      state.pumpCommand = false;
      return;
    }

    if (state.waterTankLow) {
      state.pumpCommand = false;
      return;
    }

    if (state.soilPercent >= SOIL_DRY_THRESHOLD_PERCENT) {
      state.pumpCommand = false;
      return;
    }

    state.pumpCommand = true;
  */
  (void)state;
}

/*
  applySafetyOverrides()
  ----------------------
  Final safety pass.

  This function should override normal commands if something unsafe happens.

  WHY LAST:
  Because safety should have final authority.

  Example future rules:
  - no pump if tank is empty
  - no fan if sensor invalid
  - shut everything down on fault
*/
static void applySafetyOverrides(SystemState& state) {
  /*
    Placeholder version for now.
    Add overrides as real state fields become available.

    Example later:

    if (state.waterTankLow) {
      state.pumpCommand = false;
    }

    if (!state.climateValid) {
      state.fanCommand = false;
    }
  */
  (void)state;
}

/*
  updateWebserver()
  ----------------
  Update the webserver with the latest state.
*/
static unsigned long lastPrintMs = 0;

static void updateWebserver(SystemState& state, WiFiClient& client) {
  StaticJsonDocument<JSON_SIZE> data;

  generateJSON(data, state);

  sendToThingspeak(data, client);

  /*
    Debug print guard.
  */
  if (state.nowMs - lastPrintMs < SERIAL_PRINT_INTERVAL_MS) {
    return;
  }

  lastPrintMs = state.nowMs;
  /* This is not needed for Thingspeak, but will be if we deccide on doing it with a real webserver.

  serializeJson(data, Serial);
  Serial.println();
  */
}

/*
  generateJSON()
  --------------
  Generate JSON data from the latest state.
*/


static void generateJSON(StaticJsonDocument<JSON_SIZE>& data, SystemState& state) {
  data["nowMs"] = state.nowMs; // 12345
  data["fanCommand"] = state.fanCommand; // false
  data["fanPhaseStartMs"] = state.fanPhaseStartMs; // 12345
  data["fanCycleIsOnPhase"] = state.fanCycleIsOnPhase; // false
  data["fanCycleEnabled"] = state.fanCycleEnabled; // false
  data["soilDry"] = state.soilDry; // false
}

/*
  sendToThingspeak()
  --------------
  Send data to Thingspeak.
*/

static void sendToThingspeak(StaticJsonDocument<JSON_SIZE>& data, WiFiClient& client) {

  unsigned long channelID = THINGSPEAK_CHANNEL_ID; // Thingspeak channel
  const char * myWriteAPIKey = THINGSPEAK_API_KEY; // API key
  const char* server = "api.thingspeak.com";

    ThingSpeak.begin(client);
  if (client.connect(server, 80)) {
    
    // Measure Signal Strength (RSSI) of Wi-Fi connection
    long rssi = WiFi.RSSI();

    Serial.print("RSSI: ");
    Serial.println(rssi); 


    ThingSpeak.setField(4,rssi);

    // Fetch values.
    float nowMs = data["nowMs"];
    bool fanCommand = data["fanCommand"];
    float fanPhaseStartMs = data["fanPhaseStartMs"];
    bool fanCycleIsOnPhase = data["fanCycleIsOnPhase"];
    bool fanCycleEnabled = data["fanCycleEnabled"];
    bool soilDry = data["soilDry"];

    // Set thingspeak fields
    ThingSpeak.setField(1, nowMs);
    ThingSpeak.setField(2, fanCommand);
    ThingSpeak.setField(3, fanPhaseStartMs);
    ThingSpeak.setField(4, fanCycleIsOnPhase);
    ThingSpeak.setField(5, fanCycleEnabled);
    ThingSpeak.setField(6, soilDry);
  
    // Write to thingspeak
    ThingSpeak.writeFields(channelID, myWriteAPIKey);
  }
    client.stop();
}