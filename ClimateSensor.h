#ifndef CLIMATESENSOR_H
#define CLIMATESENSOR_H

#include <Arduino.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

void dhtBegin();
void dhtRead(float &t, float &h);
