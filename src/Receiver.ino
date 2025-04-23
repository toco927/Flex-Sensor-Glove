// Imports
#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "PluggableUSBHID.h"
#include "USBKeyboard.h"

// === User Preferences ===
#define USER_DEBUG true
#define USER_OUTPUT_SERIAL true
#define USER_OUTPUT_IMU_XY false

// === Display Config ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// === Bluetooth Low Energy Config ===
BLECharacteristic thumbChar;
BLECharacteristic pointerChar;
BLECharacteristic middleChar;
BLECharacteristic ringChar;
BLECharacteristic pinkyChar;
BLECharacteristic imuChar;

// === MIDI Config ===
USBKeyboard Keyboard; // create Keyboard for MIDI-like device


bool counterSubscribed = false; // DELETE

struct FingerState {
  uint8_t prevLevel = 1;
  bool isBending = false;
  uint8_t peakLevel = 1;
};

FingerState fingerStates[5];
uint8_t imuPrev = 1; // Last seen IMU value


void setup() {
  Serial.begin(9600);
  delay(5); // wait for serial to start

  // Screen Init
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED failed! MUST RESET!");
    while (true);
  }

  // Display settings
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);

  // Display: Booyting
  display.setCursor(0, 0);
  display.println(F("RECIEVER"));
  display.println(F("BOOTING!"));
  display.display();
  delay(2000);

  // BLE Init
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed! MUST RESET!");
    while (true);
  }

  // BLE Searching
  BLE.scanForUuid("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
  if(USER_DEBUG) Serial.println("Scanning for FlexGlove via BLE!");
  Serial.print("Address: ");
  Serial.println(BLE.address());
}

void loop() {
  BLEDevice peripheral = BLE.available();   // Finding a peripheral
  
  if (peripheral) {
    if(USER_DEBUG) Serial.println("Found a peripheral!");
    if (peripheral.localName() == "FlexGlove") { // connected
      if(USER_DEBUG) Serial.println("Found target peripheral!");
      BLE.stopScan(); // Stop looking

      // Display: connected
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("RECIEVER");
      display.println("CONNECTED");
      display.display();

      // This function will run until disconnected
      connectAndRead(peripheral);

      // Disconnected, scan again
      counterSubscribed = false;
      BLE.scanForUuid("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
    } else {
      return;
    }
  } else {
    // Display: Disconnected, scanning again
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("RECIEVER"));
    display.println(F("SCANNING"));
    display.display();
  }
}

void connectAndRead(BLEDevice peripheral) {
  if(USER_DEBUG) Serial.println("Connecting to peripheral!");
  //delay(200); // wait to settle
  if (!peripheral.connect()) {
    delay(500); // wait to let it restart
    Serial.println("Failed to connect!");
    return;
  }

  if (!peripheral.discoverAttributes()) {
    Serial.println("Discovery failed!");
    peripheral.disconnect();
    return;
  }

  // Adding Byte Characteristics (Byte integers are capped at 255)
  thumbChar = peripheral.characteristic("473dab7c-c93a-11e9-a32f-2a2ae2dbcce1"); 
  pointerChar = peripheral.characteristic("473dab7c-c93a-11e9-a32f-2a2ae2dbcce2");
  middleChar = peripheral.characteristic("473dab7c-c93a-11e9-a32f-2a2ae2dbcce3");
  ringChar = peripheral.characteristic("473dab7c-c93a-11e9-a32f-2a2ae2dbcce4");
  pinkyChar = peripheral.characteristic("473dab7c-c93a-11e9-a32f-2a2ae2dbcce5");
  imuChar = peripheral.characteristic("473dab7c-c93a-11e9-a32f-2a2ae2dbcce6");

  // Subscribing to updates of chars via BLE
  if (thumbChar.canSubscribe()) {
    thumbChar.subscribe();
    if(USER_DEBUG) Serial.println("Subscribed to thumb finger");
  } else {
    Serial.println("COULDN'T SUBSCRIBE TO THUMB FINGER! ERROR!");
  }

  if (pointerChar.canSubscribe()) {
    pointerChar.subscribe();
    if(USER_DEBUG) Serial.println("Subscribed to pointer finger");
  } else {
    Serial.println("COULDN'T SUBSCRIBE TO POINTER FINGER! ERROR!");
  }

  if (middleChar.canSubscribe()) {
    middleChar.subscribe();
    if(USER_DEBUG) Serial.println("Subscribed to middle finger");
  } else {
    Serial.println("COULDN'T SUBSCRIBE TO MIDDLE FINGER! ERROR!");
  }

  if (ringChar.canSubscribe()) {
    ringChar.subscribe();
    if(USER_DEBUG) Serial.println("Subscribed to ring finger");
  } else {
    Serial.println("COULDN'T SUBSCRIBE TO RING FINGER! ERROR!");
  }

  if (pinkyChar.canSubscribe()) {
    pinkyChar.subscribe();
    if(USER_DEBUG) Serial.println("Subscribed to pinky finger");
  } else {
    Serial.println("COULDN'T SUBSCRIBE TO PINKY FINGER! ERROR!");
  }

  if (imuChar.canSubscribe()) {
    imuChar.subscribe();
    if(USER_DEBUG) Serial.println("Subscribed to IMU");
  } else {
    Serial.println("COULDN'T SUBSCRIBE TO IMU! ERROR!");
  }
  int sensors[6] = {0}; // Sensor array for storing values

  // Code to run when device is connected
  while (peripheral.connected()) {
    
    bool valuesUpdated = false;

    // Reading updated values
    if (thumbChar.valueUpdated()) {
      valuesUpdated = true;
      byte thumbValue;
      thumbChar.readValue(thumbValue);
      sensors[0] = thumbValue;
    }

    if (pointerChar.valueUpdated()) {
      valuesUpdated = true;
      byte pointerValue;
      pointerChar.readValue(pointerValue);
      sensors[1] = pointerValue;
    }

    if (middleChar.valueUpdated()) {
      valuesUpdated = true;
      byte middleValue;
      middleChar.readValue(middleValue);
      sensors[2] = middleValue;
    }

    if (ringChar.valueUpdated()) {
      valuesUpdated = true;
      byte ringValue;
      ringChar.readValue(ringValue);
      sensors[3] = ringValue;
    }

    if (pinkyChar.valueUpdated()) {
      valuesUpdated = true;
      byte pinkyValue;
      pinkyChar.readValue(pinkyValue);
      sensors[4] = pinkyValue;
    }

    if (imuChar.valueUpdated()) {
      valuesUpdated = true;
      byte imuValue;
      imuChar.readValue(imuValue);
      sensors[5] = imuValue;
    }

    if(valuesUpdated) {

      verifyOutputs(sensors);
      if (USER_OUTPUT_SERIAL) printOutputs(sensors);

      //process fingers
      // Process finger FSM
      for (int i = 0; i < 5; i++) {
        processFinger(i, sensors[i]);
      }

      // Process IMU as a pulse
      processIMU(sensors[5]);
    }
    
  }
  if(USER_DEBUG) Serial.println("Peripheral disconnected");
  peripheral.disconnect(); // ENSURE A CLEAN DISCONNECT
  BLE.scanForUuid("473da924-c93a-11e9-a32f-2a2ae2dbcce4"); // RESTART SCANNING IMMEDIATELY
}

// === Verifying the Sensor Values ===
void verifyOutputs(int sensorValues[]) {
  // [0-4] in the array are flex sensors
  // [5] is the IMU

  if(sensorValues[0] < 1 || sensorValues[0] > 6) {
    if(USER_DEBUG) Serial.println("Error on Thumb Finger Input");
    sensorValues[0] = 0; // 0 is an error / unverified output flag
  }

  if(sensorValues[1] < 1 || sensorValues[1] > 6) {
    if(USER_DEBUG) Serial.println("Error on Pointer Finger Input");
    sensorValues[1] = 0; // 0 is an error / unverified output flag
  }

  if(sensorValues[2] < 1 || sensorValues[2] > 6) {
    if(USER_DEBUG) Serial.println("Error on Middle Finger Input");
    sensorValues[2] = 0; // 0 is an error / unverified output flag
  }

  if(sensorValues[3] < 1 || sensorValues[3] > 6) {
    if(USER_DEBUG) Serial.println("Error on Ring Finger Input");
    sensorValues[3] = 0; // 0 is an error / unverified output flag
  }

  if(sensorValues[4] < 1 || sensorValues[4] > 6) {
    if(USER_DEBUG) Serial.println("Error on Pinky Finger Input");
    sensorValues[4] = 0; // 0 is an error / unverified output flag
  }

  if(sensorValues[5] < 1 || sensorValues[5] > 5) {
    if(USER_DEBUG) Serial.println("Error on IMU Input");
    sensorValues[5] = 0; // 0 is an error / unverified output flag
  }
}

void processFinger(uint8_t index, uint8_t level) {
  FingerState &state = fingerStates[index];

  if (level == 1) {
    if (state.isBending && state.peakLevel > 1) {
      triggerFingerAction(index, state.peakLevel);
    }
    state.isBending = false;
    state.peakLevel = 1;
  } else {
    state.isBending = true;
    if (level > state.peakLevel) {
      state.peakLevel = level;
    }
  }

  state.prevLevel = level;
}

void processIMU(uint8_t value) {
  if (value != 1 && imuPrev == 1) {
    triggerIMUAction(value);
  }
  imuPrev = value;
}

void triggerFingerAction(uint8_t index, uint8_t level) {
  String fingerNames[5] = {"Thumb", "Pointer", "Middle", "Ring", "Pinky"};
  String label = fingerNames[index] + ": " + String(level);
  Serial.println(label);
  updateDisplay(label);
  
  delay(10); // mimic keypress timing
  // You could add Keyboard functionality here too if you want.
}

void triggerIMUAction(uint8_t value) {
  String label = "IMU: ";
  switch (value) {
    case 2: // Right twitch
      Keyboard.media_control(KEY_VOLUME_UP);
      label += "Volume Up";
      break;
    case 3: // Left twitch
      Keyboard.media_control(KEY_VOLUME_DOWN);
      label += "Volume Down";
      break;
    case 4: // Forward twitch
      Keyboard.media_control(KEY_MUTE);
      label += "Mute Toggle";
      break;
    case 5: // Backward twitch
      Keyboard.media_control(KEY_NEXT_TRACK);
      label += "Next Song";
      break;
    default:
      return; // Ignore invalid values
  }

  Serial.println(label);
  updateDisplay(label);
}

void updateDisplay(const String &line2) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.println("RECIEVER");
  display.println("CONNECTED");
  display.println(line2);
  display.display();
}




void printOutputs(int sensors[]) {
  Serial.print("Thumb: ");
  Serial.print(sensors[0]);
  Serial.print("\t Pointer: ");
  Serial.print(sensors[1]);
  Serial.print("\t Middle: ");
  Serial.print(sensors[2]);
  Serial.print("\t Ring: ");
  Serial.print(sensors[3]);
  Serial.print("\t Pinky: ");
  Serial.print(sensors[4]);
  Serial.print("\t IMU: ");
  Serial.println(sensors[5]);
  return;
}
