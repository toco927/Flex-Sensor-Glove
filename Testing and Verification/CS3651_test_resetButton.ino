void setup() {
  Serial.begin(9600);
  delay(50);
  Serial.println("SETUP");
  delay(1000); // Allow time for serial connection

  Serial.println("Hello! Device will reset in 3 seconds...");
  delay(3000);

  softwareReset();  // Perform a full system reset
  Serial.println("SHOULDNT BE HERE");
}

void loop() {
  // Nothing here — MCU will reset after setup
  Serial.println("LOOPING");
  delay(500);
}

void softwareReset() {
  NVIC_SystemReset();  // Built-in CMSIS function to reset the MCU
}
