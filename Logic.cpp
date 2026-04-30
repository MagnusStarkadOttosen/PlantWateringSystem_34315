#include "Logic.h"
#include "Config.h"
#include "Secrets.h"
#include "ArduinoJson.h" // ArduinoJson by Benoit Blanchon
#include "ThingSpeak.h" // ThingSpeak by MathWorks
#include <string.h>

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
static void generateJSON(StaticJsonDocument<JSON_SIZE>& data, const SystemState& state);
static void sendToThingspeak(const SystemState& state, WiFiClient& client);
static void sendToRestApi(const String& serializedJson, WiFiClient& client, HTTPClient& http);
static bool pumpIsInLockout(const SystemState& state);

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
    Only update the webserver every configured interval.
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

  Fan uses different ON/OFF temperature/humidity thresholds to prevent
  flickering around one exact value.
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
  if (!state.pumpEnabled || state.waterEmpty) {
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

  if (pumpIsInLockout(state)) {
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
  if (state.waterEmpty) {
    state.pumpActive = false;
  }

  if (!state.climateValid) {
    state.fanActive = false;
  }
}

/*
  updateWebserver()
  -----------------
  Sends the latest state to either ThingSpeak or the REST API.
*/
static void updateWebserver(SystemState& state, WiFiClient& client, HTTPClient& http) {
  // Don't try to send data if the WiFi isn't connected.
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  // RSSI belongs in state because both ThingSpeak and REST/debug can use it.
  state.wifiRssi = WiFi.RSSI();

  #if !defined(WEBSERVER_MODE)
    Serial.println("WEBSERVER_MODE not configured");
    return;
  #else
    if (strcmp(WEBSERVER_MODE, "THINGSPEAK") == 0) {
      sendToThingspeak(state, client);
    } else if (strcmp(WEBSERVER_MODE, "REST_API") == 0) {
      StaticJsonDocument<JSON_SIZE> data;
      char serializedJson[JSON_SIZE];

      generateJSON(data, state);
      serializeJson(data, serializedJson, sizeof(serializedJson));

      sendToRestApi(String(serializedJson), client, http);
    }
  #endif
}

/*
  generateJSON()
  --------------
  Generate JSON data from the latest state.
*/
static void generateJSON(StaticJsonDocument<JSON_SIZE>& data, const SystemState& state) {
  data["nowMs"] = state.nowMs;

  data["temperatureC"] = state.temperatureC;
  data["humidityPct"] = state.humidityPct;
  data["climateValid"] = state.climateValid;

  data["soilDry"] = state.soilDry;
  data["soilDigitalValue"] = state.soilDigitalValue;
  data["soilAnalogValue"] = state.soilAnalogValue;

  data["waterEmpty"] = state.waterEmpty;
  data["waterRawValue"] = state.waterRawValue;

  data["fanActive"] = state.fanActive;
  data["pumpEnabled"] = state.pumpEnabled;
  data["pumpActive"] = state.pumpActive;
  data["pumpInLockout"] = pumpIsInLockout(state);

  data["wifiRssi"] = state.wifiRssi;
}

/*
  sendToThingspeak()
  ------------------
  Send the most useful eight numeric values to ThingSpeak.

  ThingSpeak field mapping:
    1 temperatureC
    2 humidityPct
    3 soilAnalogValue
    4 soilDry
    5 waterEmpty
    6 pumpActive
    7 fanActive
    8 wifiRssi

  Extra debug values go into the ThingSpeak status string.
*/
static void sendToThingspeak(const SystemState& state, WiFiClient& client) {
  #if !defined(THINGSPEAK_CHANNEL_ID) || !defined(THINGSPEAK_API_KEY)
    Serial.println("ThingSpeak not configured");
    return;
  #else
    unsigned long channelID = THINGSPEAK_CHANNEL_ID;
    const char* myWriteAPIKey = THINGSPEAK_API_KEY;
    const char* server = "api.thingspeak.com";

    ThingSpeak.begin(client);

    if (client.connect(server, 80)) {
      Serial.print("RSSI: ");
      Serial.println(state.wifiRssi);

      ThingSpeak.setField(1, state.temperatureC);
      ThingSpeak.setField(2, state.humidityPct);
      ThingSpeak.setField(3, state.soilAnalogValue);
      ThingSpeak.setField(4, state.soilDry ? 1 : 0);
      ThingSpeak.setField(5, state.waterEmpty ? 1 : 0);
      ThingSpeak.setField(6, state.pumpActive ? 1 : 0);
      ThingSpeak.setField(7, state.fanActive ? 1 : 0);
      ThingSpeak.setField(8, state.wifiRssi);

      String status =
        "climateValid=" + String(state.climateValid ? 1 : 0) +
        ",pumpEnabled=" + String(state.pumpEnabled ? 1 : 0) +
        ",pumpLockout=" + String(pumpIsInLockout(state) ? 1 : 0) +
        ",soilDig=" + String(state.soilDigitalValue) +
        ",waterRaw=" + String(state.waterRawValue);

      ThingSpeak.setStatus(status);

      int result = ThingSpeak.writeFields(channelID, myWriteAPIKey);
      Serial.print("ThingSpeak result: ");
      Serial.println(result);
    }

    client.stop();
  #endif
}

/*
  sendToRestApi()
  ---------------
  Send data to a webserver.
*/
static void sendToRestApi(const String& data, WiFiClient& client, HTTPClient& http) {
  #if !defined(REST_API_SERVER) || !defined(REST_API_PORT)
    Serial.println("REST_API_SERVER or REST_API_PORT not configured");
    return;
  #else
    const String PATH = "/update";

    http.begin(client, String(REST_API_SERVER) + ":" + String(REST_API_PORT) + PATH);

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Content-Length", String(data.length()));

    int httpResponseCode = http.POST(data);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();
  #endif
}

static bool pumpIsInLockout(const SystemState& state) {
  return state.nowMs < state.pumpLockoutUntilMs;
}
