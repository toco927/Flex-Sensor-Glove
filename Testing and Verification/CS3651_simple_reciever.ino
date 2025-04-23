// Imports
#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// Screen setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// BLE Setup
BLECharacteristic counterCharacteristic;
bool counterSubscribed = false;

void setup() {
  Serial.begin(9600);
  delay(10); // wait for serial to start

  // BLE Init
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed! MUST RESET!");
    while (true);
  }

  // Screen Init
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED failed! MUST RESET!");
    while (true);
  }

  // Display settings
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);

  // Display Booying
  display.setCursor(0, 0);
  display.println(F("RECIEVER"));
  display.println(F("BOOTING!"));
  display.display();
  delay(2000);

  // Searching
  BLE.scanForUuid("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
  Serial.println("Scanning for FlexGlove!");
}

void loop() {
  BLEDevice peripheral = BLE.available();
  
  if (peripheral) {
    Serial.println("Found a peripheral!");
    if (peripheral.localName() == "counterPeripheral") { // connected
      Serial.println("Found target peripheral!");
      BLE.stopScan();
      connectAndRead(peripheral);

      counterSubscribed = false;
      BLE.scanForUuid("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
    }
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("RECIEVER"));
    display.println(F("SCANNING"));
    display.display();
  }
}

void connectAndRead(BLEDevice peripheral) {
  Serial.println("Connecting to peripheral!");
  if (!peripheral.connect()) {
    Serial.println("Failed to connect");
    return;
  }

  if (!peripheral.discoverAttributes()) {
    Serial.println("Discovery failed");
    peripheral.disconnect();
    return;
  }

  counterCharacteristic = peripheral.characteristic("473dab7c-c93a-11e9-a32f-2a2ae2dbcce4");

  if (counterCharacteristic.canSubscribe()) {
    counterCharacteristic.subscribe();
    Serial.println("Subscribed to counter");
  }

  while (peripheral.connected()) {
    if (counterCharacteristic.valueUpdated()) {
      byte value;
      counterCharacteristic.readValue(value);

      Serial.print("Received counter: ");
      Serial.println(value);

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("RECIEVER");
      display.println("CONNECTED");
      display.print("Count: ");
      display.println(value);
      display.display();
    }
  }
  Serial.println("Peripheral disconnected");
}



