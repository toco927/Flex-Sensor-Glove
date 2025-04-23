#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "LSM6DS3.h"

// Create an instance of the IMU
LSM6DS3 myIMU(I2C_MODE, 0x6A); // I2C address 0x6A

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
#define LED_PIN       13
#define FLEX_PIN      A0

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Bend thresholds
const int bendLevel1 = 500;
const int bendLevel2 = 350;
const int bendLevel3 = 250;
const int bendLevel4 = 170;
const int bendLevel5 = 100;

int classifyFingerPosition(int flexValue) { 
  if (flexValue > bendLevel1) return 1;
  else if (flexValue > bendLevel2) return 2;
  else if (flexValue > bendLevel3) return 3;
  else if (flexValue > bendLevel4) return 4;
  else if (flexValue > bendLevel5) return 5;
  else return 6;
}

unsigned long lastUpdate = 0;
const unsigned long interval = 100;
bool ledState = false;

void setup() {
  Serial.begin(9600);
  //while (!Serial);

  if (myIMU.begin() != 0) {
    Serial.println("Device error");
  } else {
    Serial.println("Device OK!");
  }

  pinMode(LED_PIN, OUTPUT);
  analogReadResolution(10); // 10-bit ADC

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdate >= interval) {
    lastUpdate = currentMillis;

    // Toggle LED
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);

    // Read flex and classify
    int flexVal = analogRead(FLEX_PIN);
    int bendLevel = classifyFingerPosition(flexVal);

    // Read gyro values
    float gyroX = myIMU.readFloatGyroX();
    float gyroY = myIMU.readFloatGyroY();
    float gyroZ = myIMU.readFloatGyroZ();

    // Update display
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Flex: ");
    display.println(flexVal);
    display.print("Bend: ");
    display.println(bendLevel);

    display.print("Gx:");
    display.println(gyroX, 1);
    display.print("Gy:");
    display.println(gyroY, 1);
    display.print("Gz:");
    display.println(gyroZ, 1);
    display.display();

    // Optional: also log to serial
    Serial.print("Flex=");
    Serial.print(flexVal);
    Serial.print(" Bend=");
    Serial.print(bendLevel);
    Serial.print(" Gyro: X=");
    Serial.print(gyroX, 2);
    Serial.print(" Y=");
    Serial.print(gyroY, 2);
    Serial.print(" Z=");
    Serial.println(gyroZ, 2);
  }
}
