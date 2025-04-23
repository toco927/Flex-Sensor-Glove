#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int counter = 0;

void setup() {
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed. MUST RESET!");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
}

void loop() {
  display.clearDisplay();

  // Large text: Connected
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(F("Connected"));
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Not"));
  display.println(F("Connected"));
  display.println(counter);
  counter++;
  display.display();
  Serial.println("One cycle");
  delay(2000);
}

