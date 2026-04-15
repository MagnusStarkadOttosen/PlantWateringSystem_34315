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
