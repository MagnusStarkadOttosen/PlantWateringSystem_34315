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
static void updateWebserver(SystemState& state, WiFiClient& client, HTTPClient& http);
static void generateJSON(StaticJsonDocument<JSON_SIZE>& data, SystemState& state);
static void sendToWebserver(char* jsonBuffer[JSON_SIZE], WiFiClient& client);
static void sendToThingspeak(StaticJsonDocument<JSON_SIZE>& data, WiFiClient& client);
static void sendToRestApi(String serializedJson, WiFiClient& client, HTTPClient& http);

/*
  initializeLogic()
  -----------------
  Sets up initial values for logic-owned state.

  IMPORTANT:
  Only initialize things here that belong to the logic layer,
  not hardware setup.
*/
void initializeLogic(SystemState& state) {
  state.fanActive = false;
  state.pumpActive = false;
  state.pumpEnabled = true;
  state.pumpStartTime = 0;
  state.pumpLockoutUntilMs = 0;
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

void updateLogic(SystemState& state, WiFiClient& client, HTTPClient& http) {
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
  updateWebserver(state, client, http);
}

/*
  updateFanLogic()
  ----------------
  Handles fan behavior only.

  Fan uses two different temperature/humidity to prevent fluctuation:
  - turn ON when temperature or humidity is high
  - stay ON until both are safely low again
*/
static void updateFanLogic(SystemState& state) {
  if (!state.climateValid) {
    state.fanActive = false;
    return;
  }

  if (!state.fanActive) {
    if (state.temperatureC >= FAN_TEMP_ON_C ||
        state.humidityPct >= FAN_HUMIDITY_ON_PCT) {
      state.fanActive = true;
    }
    return;
  }

  if (state.temperatureC <= FAN_TEMP_OFF_C &&
      state.humidityPct <= FAN_HUMIDITY_OFF_PCT) {
    state.fanActive = false;
  }
}

/*
  updateWateringLogic()
  ---------------------
  Handles pump/watering behavior only.

  Pump behavior:
  - start only when soil is dry, water is available, and lockout is over
  - run for PUMP_ON_DURATION_MS
  - then stay locked out for PUMP_LOCKOUT_MS
*/
static void updateWateringLogic(SystemState& state) {
  if (!state.pumpEnabled || state.isEmpty) {
    state.pumpActive = false;
    return;
  }

  if (state.pumpActive) {
    if (state.nowMs - state.pumpStartTime >= PUMP_ON_DURATION_MS) {
      state.pumpActive = false;
      state.pumpLockoutUntilMs = state.nowMs + PUMP_LOCKOUT_MS;
    }
    return;
  }

  if (state.nowMs < state.pumpLockoutUntilMs) {
    state.pumpActive = false;
    return;
  }

  if (state.soilDry) {
    state.pumpActive = true;
    state.pumpStartTime = state.nowMs;
  }
}

/*
  applySafetyOverrides()
  ----------------------
  Final safety pass.

  Safety has final authority over normal logic.
*/
static void applySafetyOverrides(SystemState& state) {
  if (state.isEmpty) {
    state.pumpActive = false;
  }

  if (!state.climateValid) {
    state.fanActive = false;
  }
}

/*
  updateWebserver()
  ----------------
  Update the webserver with the latest state.
*/
static void updateWebserver(SystemState& state, WiFiClient& client, HTTPClient& http) {
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

      sendToRestApi(serializedJson, client, http);
    }

  #endif

}

/*
  generateJSON()
  --------------
  Generate JSON data from the latest state.
*/
static void generateJSON(StaticJsonDocument<JSON_SIZE>& data, SystemState& state) {
  data["fanActive"] = state.fanActive;
  data["temperatureC"] = state.temperatureC;
  data["humidityPct"] = state.humidityPct;
  data["climateValid"] = state.climateValid;
  data["soilDry"] = state.soilDry;
  data["waterEmpty"] = state.isEmpty;
  data["pumpEnabled"] = state.pumpEnabled;
  data["pumpStartTime"] = state.pumpStartTime;
  data["pumpLockoutUntilMs"] = state.pumpLockoutUntilMs;
  data["pumpActive"] = state.pumpActive;
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
float temperatureC = data["temperatureC"];
float humidityPct = data["humidityPct"];
bool soilDry = data["soilDry"];
bool waterEmpty = data["waterEmpty"];
bool pumpEnabled = data["pumpEnabled"];
bool pumpActive = data["pumpActive"];

// Set thingspeak fields
ThingSpeak.setField(1, soilDry);
ThingSpeak.setField(2, fanActive);
ThingSpeak.setField(3, temperatureC);
ThingSpeak.setField(4, humidityPct);
ThingSpeak.setField(5, waterEmpty);
ThingSpeak.setField(6, pumpEnabled);
ThingSpeak.setField(7, pumpActive);
ThingSpeak.setField(8, rssi);
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
static void sendToRestApi(String data, WiFiClient& client, HTTPClient& http) {
  #if !defined(REST_API_SERVER) || !defined(REST_API_PORT)
    Serial.println("REST_API_SERVER or REST_API_PORT not configured");
    return;
  #else

    const String PATH = "/update";

    http.begin(client, String(REST_API_SERVER) + ":" + String(REST_API_PORT) + PATH);// REST_API_SERVER + ':' + REST_API_PORT + PATH);

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Content-Length", String(data.length()));

    int httpResponseCode = http.POST(data);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();
  #endif

}