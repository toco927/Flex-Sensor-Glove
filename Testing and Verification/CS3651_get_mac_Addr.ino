#include <ArduinoBLE.h>

void setup() {
  Serial.begin(9600);
  while (!Serial); // wait for serial monitor to open

  if (!BLE.begin()) {
    Serial.println("BLE init failed");
    while (1);
  }

  Serial.print("Device MAC Address: ");
  Serial.println(BLE.address());
}

void loop() {
  // Nothing needed in loop
}
