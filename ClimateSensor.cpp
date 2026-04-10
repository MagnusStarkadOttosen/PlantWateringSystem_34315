#include "ClimasteSensor.h"

#define DHTPIN 16
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void dhtBegin() {
  dht.begin();
}

void dhtRead(float &t, float &h) {
  h = dht.readHumidity();
  t = dht.readTemperature();
}
