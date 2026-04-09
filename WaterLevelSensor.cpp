// Sensor input taken on pin A0 on ESP12

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(A0, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int sensorValue = analogRead(A0);

  // // Convert the sensor value to a percentage (0 to 100%)
  float waterLevelPercentage = (sensorValue / 4095.0) * 100;

  // Print the sensor value and water level percentage to the serial monitor
  Serial.print("Sensor Value: ");
  Serial.print(sensorValue);
  Serial.print("  Water Level Percentage: ");
  Serial.println(waterLevelPercentage);
  delay(500);
}
