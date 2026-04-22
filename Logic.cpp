#include "Logic.h"
#include "Config.h"
#include "Secrets.h"
#include "ArduinoJson.h" // ArduinoJson by Benoit Blanchon
#include "ThingSpeak.h" // Thingspeak by MathWorks

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
static void sendToWebserver(char* jsonBuffer[JSON_SIZE], WiFiClient& client);
static void sendToThingspeak(StaticJsonDocument<JSON_SIZE>& data, WiFiClient& client);
static void sendToRestApi(String serializedJson, WiFiClient& client);

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
  state.fanActive = true;

  /*
    Pump defaults.
    Doesn't exist yet.
  */
  state.pumpActive = false;
  state.pumpEnabled = true;
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
static unsigned long lastWebserverUpdateMs = 0;

void updateLogic(SystemState& state, WiFiClient& client) {
  updateFanLogic(state);
  updateWateringLogic(state);
  applySafetyOverrides(state);
  /*
    Only update the webserver every 20 seconds
  */
  if (state.nowMs - lastWebserverUpdateMs < WEBSERVER_UPDATE_INTERVAL_MS) {
    return;
  }

  lastWebserverUpdateMs = state.nowMs;
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
    state.fanActive = false;
    return;
  }

  /*
    GUARD 2:
    Stay in ON phase until ON duration expires.
  */
  if (state.fanCycleIsOnPhase &&
      (state.nowMs - state.fanPhaseStartMs < FAN_ON_DURATION_MS)) {
    state.fanActive = true;
    return;
  }

  /*
    GUARD 3:
    Stay in OFF phase until OFF duration expires.
  */
  if (!state.fanCycleIsOnPhase &&
      (state.nowMs - state.fanPhaseStartMs < FAN_OFF_DURATION_MS)) {
    state.fanActive = false;
    return;
  }

  /*
    If we got here, the current phase expired.
    Toggle phase and restart timer.
  */
  state.fanCycleIsOnPhase = !state.fanCycleIsOnPhase;
  state.fanPhaseStartMs = state.nowMs;
  state.fanActive = state.fanCycleIsOnPhase;
}

/*
  updateWateringLogic()
  ---------------------
  Handles pump/watering behavior only.

  Right now this is just a placeholder so the architecture is ready.
*/
static void updateWateringLogic(SystemState& state) {
  if (!state.pumpEnabled) {
    state.pumpActive = false;
    return;
  }
  
  if (state.isEmpty) {
    state.pumpActive = false;
    return;
  }
  
  if (state.soilDry && !state.pumpActive) {
    state.pumpActive = true;
    state.pumpStartTime = state.nowMs;
  }

  if(state.pumpActive) {
    if (state.nowMs - state.pumpStartTime >= PUMP_ON_DURATION_MS) {
      state.pumpActive = false;
    } else {
      state.pumpActive = true;
    }
    return;
  }

  state.pumpActive = false;
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
static void updateWebserver(SystemState& state, WiFiClient& client) {
  StaticJsonDocument<JSON_SIZE> data;

  generateJSON(data, state);

  // Don't try to send data if the WiFi isn't connected.
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  #if !defined(WEBSERVER_MODE)
    Serial.println("WEBSERVER_MODE not configured");
    return;
  #else
    if (WEBSERVER_MODE == "THINGSPEAK") {
      sendToThingspeak(data, client);
    } else if (WEBSERVER_MODE == "REST_API") {
      char serializedJson[JSON_SIZE];

      serializeJson(data, serializedJson);

      sendToRestApi(serializedJson, client);
    }

  #endif

}

/*
  generateJSON()
  --------------
  Generate JSON data from the latest state.
*/
static void generateJSON(StaticJsonDocument<JSON_SIZE>& data, SystemState& state) {
  data["fanActive"] = state.fanActive; // false
  data["fanPhaseStartMs"] = state.fanPhaseStartMs; // 12345
  data["fanCycleIsOnPhase"] = state.fanCycleIsOnPhase; // false
  data["fanCycleEnabled"] = state.fanCycleEnabled; // false
  data["soilDry"] = state.soilDry; // false
  data["pumpEnabled"] = state.pumpEnabled; // false
  data["pumpStartTime"] = state.pumpStartTime; // 12345
  data["pumpActive"] = state.pumpActive; // false
}

/*
  sendToThingspeak()
  --------------
  Send data to Thingspeak.
*/
static void sendToThingspeak(StaticJsonDocument<JSON_SIZE>& data, WiFiClient& client) {
  #if !defined(THINGSPEAK_CHANNEL_ID) || !defined(THINGSPEAK_API_KEY) 
    Serial.println("Thingspeak not configured");
    return;
  #else
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
      bool fanActive = data["fanActive"];
      float fanPhaseStartMs = data["fanPhaseStartMs"];
      bool fanCycleIsOnPhase = data["fanCycleIsOnPhase"];
      bool fanCycleEnabled = data["fanCycleEnabled"];
      bool soilDry = data["soilDry"];
      bool pumpEnabled = data["pumpEnabled"];
      float pumpStartTime = data["pumpStartTime"];
      bool pumpActive = data["pumpActive"];

      // Set thingspeak fields
      ThingSpeak.setField(1, soilDry);
      ThingSpeak.setField(2, fanActive);
      ThingSpeak.setField(3, fanPhaseStartMs);
      ThingSpeak.setField(4, fanCycleIsOnPhase);
      ThingSpeak.setField(5, fanCycleEnabled);
      ThingSpeak.setField(6, pumpEnabled);
      ThingSpeak.setField(7, pumpStartTime);
      ThingSpeak.setField(8, pumpActive);
    
      // Write to thingspeak
      ThingSpeak.writeFields(channelID, myWriteAPIKey);
    }
    client.stop();
  #endif
}

/*
  sendToRestApi()
  --------------
  Send data to a webserver.
*/
static void sendToRestApi(String data, WiFiClient& client) {
  #if !defined(REST_API_SERVER) || !defined(REST_API_PORT)
    Serial.println("REST_API_SERVER or REST_API_PORT not configured");
    return;
  #else
    const String PATH = "/update";

    if (client.connect(REST_API_SERVER, REST_API_PORT)) {
      // HTTP header
      client.println("POST" + PATH + " HTTP/1.1");
      client.println("Host: " + String(REST_API_SERVER));
      client.println("Connection: close");
      client.println("Content-Type: application/json");
      client.print("Content-Length: ");
      client.println(data.length());
      client.println(); // end HTTP header

      // HTTP body
      client.println(data);
      Serial.println("SENT DATA: " + data);


      while(client.connected()) {
        if(client.available()){
          // read an incoming byte from the server and print it to serial monitor:
          char c = client.read();
          Serial.print(c);
        }
      }
      Serial.println();

      // the server's disconnected, stop the client:
      client.stop();
    } else {
      Serial.println("connection failed");
    }
  #endif

}