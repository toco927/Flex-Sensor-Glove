#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// === Display Config ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// BLE
BLEService counterService("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
BLEByteCharacteristic counterCharacteristic("473dab7c-c93a-11e9-a32f-2a2ae2dbcce4", BLERead | BLENotify);

uint8_t counter = 0;
unsigned long lastUpdate = 0;

void setup() {
  Serial.begin(9600);
  delay(10);

  if (!BLE.begin()) {
    Serial.println("BLE init failed!");
    while (1);
  }

  // Init screen
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED failed! MUST RESET!");
    while (true);
  }

  BLE.setLocalName("counterPeripheral");
  BLE.setAdvertisedService(counterService);
  counterService.addCharacteristic(counterCharacteristic);
  BLE.addService(counterService);

  counterCharacteristic.writeValue(counter);

  BLE.advertise();
  Serial.print("Sender MAC Address: ");
  Serial.println(BLE.address());
  Serial.println("Peripheral is running...");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Flex Glove: Sender");
  display.println("Waiting...");
  display.display();
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.println("Connected to central");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Flex Glove: Sender"));
    display.println(F("CONNECTED"));
    display.display();

    while (central.connected()) {
      if (millis() - lastUpdate > 1000) {
        lastUpdate = millis();
        counter++;
        counterCharacteristic.writeValue(counter);
        Serial.print("Sent counter: ");
        Serial.println(counter);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(F("Flex Glove: Sender"));
        display.println(F("CONNECTED"));
        display.print(F("COUNTER: "));
        display.print(counter);
        display.display();
      }
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Flex Glove: Sender"));
    display.println(F("DISCONNECTED"));
    display.display();
  }
} 