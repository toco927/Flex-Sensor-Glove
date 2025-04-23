#include <ArduinoBLE.h>
#include "PluggableUSBHID.h"
#include "USBKeyboard.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// === Display Config ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int USER_DEBUG = 0; // Set to 1 to see serial info

USBKeyboard Keyboard; // create Keyboard for MIDI-like device


BLEDevice peripheral;
BLECharacteristic numberCharacteristic;
BLECharacteristic thumbChar;
BLECharacteristic pointerChar;
BLECharacteristic middleChar;
BLECharacteristic ringChar;
BLECharacteristic pinkyChar;
BLECharacteristic imuChar;

static bool thumbDown = false;

// Track last flex level for each finger
static int lastThumbLevel = 0;
static int lastPointerLevel = 0;
static int lastMiddleLevel = 0;
static int lastRingLevel = 0;

// Debounce flags
static bool playPauseTriggered = false;
static unsigned long lastPlayPauseTime = 0;
const unsigned long DEBOUNCE_TIME = 500; // 500ms debounce


void setup() {
  Serial.begin(9600);
  //while (!Serial);


  // Initialize Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed. MUST RESET!");
    while (true);
  }
  display.clearDisplay();
  display.display();

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed! MUST RESET!");
    while (1);
  }

  if(USER_DEBUG) Serial.println("RECEIVER scanning for SENDER...");
  BLE.scanForAddress("8d:6b:89:75:30:c4"); // SENDER's MAC
}

void verifyOutputs(int outputValues[]) {
  if(outputValues[0] < 1 || outputValues[0] > 6) {
    // ERROR
    if(USER_DEBUG) Serial.println("Error on Thumb Finger Input");
    outputValues[0] = 0; // 0 is an error / unverified output flag
  }

  if(outputValues[1] < 1 || outputValues[1] > 6) {
    // ERROR
    if(USER_DEBUG) Serial.println("Error on Pointer Finger Input");
    outputValues[1] = 0; // 0 is an error / unverified output flag
  }

  if(outputValues[2] < 1 || outputValues[2] > 6) {
    // ERROR
    if(USER_DEBUG) Serial.println("Error on Middle Finger Input");
    outputValues[2] = 0; // 0 is an error / unverified output flag
  }

  if(outputValues[3] < 1 || outputValues[3] > 6) {
    // ERROR
    if(USER_DEBUG) Serial.println("Error on Ring Finger Input");
    outputValues[3] = 0; // 0 is an error / unverified output flag
  }

  if(outputValues[4] < 1 || outputValues[4] > 6) {
    // ERROR
    if(USER_DEBUG) Serial.println("Error on Pinky Finger Input");
    outputValues[4] = 0; // 0 is an error / unverified output flag
  }

  if(outputValues[5] < 1 || outputValues[5] > 10) {
    // ERROR
    if(USER_DEBUG) Serial.println("Error on IMU Input");
    outputValues[5] = 0; // 0 is an error / unverified output flag
  }
}

void printOutputs(int outputValues[]) {
  Serial.print("Thumb: ");
  Serial.print(outputValues[0]);
  Serial.print("\t Pointer: ");
  Serial.print(outputValues[1]);
  Serial.print("\t Middle: ");
  Serial.print(outputValues[2]);
  Serial.print("\t Ring: ");
  Serial.print(outputValues[3]);
  Serial.print("\t Pinky: ");
  Serial.print(outputValues[4]);
  Serial.print("\t IMU: ");
  Serial.println(outputValues[5]);
  return;
}

void processThumb(int currentThumbLevel) {
  if (currentThumbLevel != lastThumbLevel) {
    if (currentThumbLevel >= 2 && currentThumbLevel <= 4) {
      int presses = currentThumbLevel - 1; // 1 press for Level 2, 2 presses for Level 3, etc.
      for (int i = 0; i < presses; i++) {
        Keyboard.media_control(KEY_VOLUME_UP);
        delay(40);
      }
      Serial.print("Thumb Volume Up x"); Serial.println(presses);
    }
    else if (currentThumbLevel == 5) {
      unsigned long now = millis();
      if (!playPauseTriggered || (now - lastPlayPauseTime > DEBOUNCE_TIME)) {
        Keyboard.media_control(KEY_PLAY_PAUSE);
        Serial.println("Thumb Play/Pause triggered");
        playPauseTriggered = true;
        lastPlayPauseTime = now;
      }
    }
    else if (currentThumbLevel == 1) {
      playPauseTriggered = false; // reset trigger when thumb goes straight
    }
  }
  lastThumbLevel = currentThumbLevel;
}

void processPointer(int currentPointerLevel) {
  if (currentPointerLevel != lastPointerLevel) {
    if (currentPointerLevel >= 2 && currentPointerLevel <= 4) {
      int scrollAmount = (currentPointerLevel - 1) * 3; // Scroll faster at higher levels
      for (int i = 0; i < scrollAmount; i++) {
        Keyboard.key_code(DOWN_ARROW); // Scroll down todo: fix later
        delay(10);
      }
      Serial.print("Pointer Scroll Down x"); Serial.println(scrollAmount);
    }
    else if (currentPointerLevel == 5) {
      Keyboard.media_control(KEY_NEXT_TRACK);
      Serial.println("Pointer Next Track");
    }
  }
  lastPointerLevel = currentPointerLevel;
}

void processMiddle(int currentMiddleLevel) {
  if (currentMiddleLevel != lastMiddleLevel) {
    if (currentMiddleLevel >= 2 && currentMiddleLevel <= 4) {
      int scrollAmount = (currentMiddleLevel - 1) * 3; // Scroll faster at higher levels
      for (int i = 0; i < scrollAmount; i++) {
        Keyboard.key_code(UP_ARROW); // Scroll up todo: fix later
        delay(10);
      }
      Serial.print("Middle Scroll Up x"); Serial.println(scrollAmount);
    }
    else if (currentMiddleLevel == 5) {
      Keyboard.media_control(KEY_PREVIOUS_TRACK);
      Serial.println("Middle Previous Track");
    }
  }
  lastMiddleLevel = currentMiddleLevel;
}

void processRing(int currentRingLevel) {
  if (currentRingLevel != lastRingLevel) {
    if (currentRingLevel >= 2 && currentRingLevel <= 4) {
      int presses = currentRingLevel - 1; // 1 press for Level 2, 2 for Level 3, etc
      for (int i = 0; i < presses; i++) {
        Keyboard.media_control(KEY_VOLUME_DOWN);
        delay(40);
      }
      Serial.print("Ring Volume Down x"); Serial.println(presses);
    }
    else if (currentRingLevel == 5) {
      Keyboard.media_control(KEY_MUTE);
      Serial.println("Ring Toggle Mute");
    }
  }
  lastRingLevel = currentRingLevel;
}


void loop() {
  BLE.poll();
  int outputValues[6] = {0}; 
  if (!peripheral || !peripheral.connected()) {
    peripheral = BLE.available();

    if (peripheral && peripheral.address() == "8d:6b:89:75:30:c4") {
      BLE.stopScan();

      Serial.println("Connecting to SENDER...");

      if (!peripheral.connect()) {
        Serial.println("Failed to connect!");
        BLE.scanForAddress("8d:6b:89:75:30:c4");
        return;
      }

      Serial.println("Connected. Discovering attributes...");
      if (!peripheral.discoverAttributes()) {
        Serial.println("Attribute discovery failed!");
        peripheral.disconnect();
        return;
      }

      numberCharacteristic = peripheral.characteristic("12345678-1234-5678-1234-56789abcdef1");
      thumbChar = peripheral.characteristic("12345678-1234-5678-1234-56789abcdea1");
      pointerChar = peripheral.characteristic("12345678-1234-5678-1234-56789abcdea2");
      middleChar = peripheral.characteristic("12345678-1234-5678-1234-56789abcdea3");
      ringChar = peripheral.characteristic("12345678-1234-5678-1234-56789abcdea4");
      pinkyChar = peripheral.characteristic("12345678-1234-5678-1234-56789abcdea5");
      //imuXChar = peripheral.characteristic("12345678-1234-5678-1234-56789abcdea6");
      //imuYChar = peripheral.characteristic("12345678-1234-5678-1234-56789abcdea7");
      imuChar = peripheral.characteristic("12345678-1234-5678-1234-56789abcdea8");

      if (!thumbChar || !pointerChar || !middleChar || !ringChar || !pinkyChar || !imuChar) {
        Serial.println("A Characteristic was not found.");
        peripheral.disconnect();
        return;
      }

      if (thumbChar.canSubscribe()) {
        thumbChar.subscribe();
        if(USER_DEBUG) Serial.println("Subscribed to thumb notifications.");
      }

      if (pointerChar.canSubscribe()) {
        pointerChar.subscribe();
        if(USER_DEBUG) Serial.println("Subscribed to pointer finger notifications.");
      }

      if (middleChar.canSubscribe()) {
        middleChar.subscribe();
        if(USER_DEBUG) Serial.println("Subscribed to middle finger notifications.");
      }

      if (ringChar.canSubscribe()) {
        ringChar.subscribe();
        if(USER_DEBUG) Serial.println("Subscribed to ring finger notifications.");
      }

      if (pinkyChar.canSubscribe()) {
        pinkyChar.subscribe();
        if(USER_DEBUG) Serial.println("Subscribed to pinky finger notifications.");
      }

      if (imuChar.canSubscribe()) {
        imuChar.subscribe();
        if(USER_DEBUG) Serial.println("Subscribed to IMU notifications.");
      }
    }
  }

  if (thumbChar && thumbChar.valueUpdated()) {
    byte thumbValue;
    thumbChar.readValue(thumbValue);
    outputValues[0] = thumbValue;
  }

  if (pointerChar && pointerChar.valueUpdated()) {
    byte pointerValue;
    pointerChar.readValue(pointerValue);
    outputValues[1] = pointerValue;
  }

  if (middleChar && middleChar.valueUpdated()) {
    byte middleValue;
    middleChar.readValue(middleValue);
    outputValues[2] = middleValue;
  }

  if (ringChar && ringChar.valueUpdated()) {
    byte ringValue;
    ringChar.readValue(ringValue);
    outputValues[3] = ringValue;
  }

  if (pinkyChar && pinkyChar.valueUpdated()) {
    byte pinkyValue;
    pinkyChar.readValue(pinkyValue);
    outputValues[4] = pinkyValue;
  }

  if (imuChar && imuChar.valueUpdated()) {
    byte imuValue;
    imuChar.readValue(imuValue);
    outputValues[5] = imuValue;
  }
  
  verifyOutputs(outputValues);
  if(USER_DEBUG) printOutputs(outputValues);

  // OUTPUT CIRCUIT IMPLEMENTATION
  // PUT MIDI CODE HERE
  processThumb(outputValues[0]);  
  processPointer(outputValues[1]);
  processMiddle(outputValues[2]);
  processRing(outputValues[3]);
  // Pinky left out for now, you can assign later if needed



  /*
  if (outputValues[0] == 2 && !thumbDown) {
    thumbDown = true;
    Keyboard.media_control(KEY_VOLUME_UP);
    Serial.println("THUMB FLEX - MIDI SENT - VOLUME UP");
  } else if (outputValues[0] != 2) {
    thumbDown = false;
  }
  */

  delay(10); // Light CPU usage
}

