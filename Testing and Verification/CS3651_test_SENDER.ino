#include <ArduinoBLE.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// === Display Config ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

const bool PRINT_XY = true;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// === Sensor & BLE Config ===
MPU6050 mpu(Wire);
const int flexPins[5] = {A0, A1, A2, A3, A4};
const int bendLevel1 = 500;
const int bendLevel2 = 350;
const int bendLevel3 = 250;
const int bendLevel4 = 170;
const int bendLevel5 = 100;

// Sampling Timer
unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL_MS = 50;

// BLE Service and Characteristics
BLEService flexGloveService("12345678-1234-5678-1234-56789abcdef0");

BLEByteCharacteristic thumbChar("12345678-1234-5678-1234-56789abcdea1", BLERead | BLENotify);
BLEByteCharacteristic pointerChar("12345678-1234-5678-1234-56789abcdea2", BLERead | BLENotify);
BLEByteCharacteristic middleChar("12345678-1234-5678-1234-56789abcdea3", BLERead | BLENotify);
BLEByteCharacteristic ringChar("12345678-1234-5678-1234-56789abcdea4", BLERead | BLENotify);
BLEByteCharacteristic pinkyChar("12345678-1234-5678-1234-56789abcdea5", BLERead | BLENotify);
BLEByteCharacteristic imuChar("12345678-1234-5678-1234-56789abcdea8", BLERead | BLENotify);

// === Sensor Classification ===
int classifyFingerPosition(int flexValue) {
  if (flexValue > bendLevel1) return 1;
  else if (flexValue > bendLevel2) return 2;
  else if (flexValue > bendLevel3) return 3;
  else if (flexValue > bendLevel4) return 4;
  else if (flexValue > bendLevel5) return 5;
  else return 6;
}

int classifyIMU(double angleX, double angleY) {
  // FLAT
  if (abs(angleX) < 10 && abs(angleY) < 10) {
    return 1;
  }
  // FORWARD TILT
  else if (angleX < -30 && abs(angleY) < 40) {
    return 2;
  }
  // BACKWARD TILT
  else if (angleX > 30 && abs(angleY) < 40) {
    return 3;
  }
  // LEFT SIDE TILT
  else if (abs(angleX) < 20 && angleY > 70) {
    return 4;
  }
  // RIGHT SIDE TILT
  else if (abs(angleX) < 20 && angleY > 20 && angleY <= 50) {
    return 5;
  }
  // UPSIDE DOWN
  else if (abs(angleX) < 20 && angleY > 50 && angleY <= 70) {
    return 6;
  }
  // OTHER
  else {
    return 9;
  }
}


// === Screen Drawing ===
void updateDisplay(int thumb, int pointer, int middle, int ring, int pinky, int imu, bool bleConnected) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  
  display.println("Status: Running");
  display.print("BLE: ");
  display.println(bleConnected ? "Connected" : "Disconnected");
  
  display.print("Thumb: "); display.println(thumb);
  display.print("Pointer: "); display.println(pointer);
  display.print("Middle: "); display.println(middle);
  display.print("Ring: "); display.println(ring);
  display.print("Pinky: "); display.println(pinky);
  display.print("IMU: "); display.println(imu);

  display.display();
}

// === Reading Sensors & Sending BLE ===
void readSensorsAndUpdate(bool writeBLE, bool showSerial, bool bleConnected, bool printIMUAngles) {
  if (millis() - lastReadTime < READ_INTERVAL_MS) return;
  lastReadTime = millis();

  mpu.update();
  
  int flexValues[5];
  for (int i = 0; i < 5; i++) {
    flexValues[i] = analogRead(flexPins[i]);
  }

  int thumb = classifyFingerPosition(flexValues[0]);
  int pointer = classifyFingerPosition(flexValues[1]);
  int middle = classifyFingerPosition(flexValues[2]);
  int ring = classifyFingerPosition(flexValues[3]);
  int pinky = classifyFingerPosition(flexValues[4]);

  double imuX = mpu.getAngleX();
  double imuY = mpu.getAngleY();
  int imu = classifyIMU(imuX, imuY);

  if (writeBLE) {
    thumbChar.writeValue(thumb);
    pointerChar.writeValue(pointer);
    middleChar.writeValue(middle);
    ringChar.writeValue(ring);
    pinkyChar.writeValue(pinky);
    imuChar.writeValue(imu);
  }

  if (showSerial) {
    Serial.print("T:"); Serial.print(thumb);
    Serial.print(" I:"); Serial.print(pointer);
    Serial.print(" M:"); Serial.print(middle);
    Serial.print(" R:"); Serial.print(ring);
    Serial.print(" P:"); Serial.print(pinky);
    Serial.print(" IMU:"); Serial.print(imu);

    if (printIMUAngles) {
      Serial.print("  |  imuX: "); Serial.print(imuX, 2);
      Serial.print(" imuY: "); Serial.print(imuY, 2);
    }

    Serial.println();
  }

  updateDisplay(thumb, pointer, middle, ring, pinky, imu, bleConnected);
}


// === Arduino Setup ===
void setup() {
  Serial.begin(9600);
  //while (!Serial);
  Serial.println("SETUP STARTED");

  // Initialize Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    while (true);
  }
  display.clearDisplay();
  display.display();

  // Initialize IMU
  Wire.begin();
  byte mpuStatus = mpu.begin();
  if (mpuStatus != 0) {
    Serial.println("MPU6050 init failed! Reset device.");
    while (true);
  }
  Serial.println("Calculating MPU offsets...");
  delay(1000);
  mpu.calcOffsets(true, true);
  Serial.println("Offsets done.");

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed! Reset device.");
    while (true);
  }
  BLE.setLocalName("FlexGlove");
  BLE.setAdvertisedService(flexGloveService);

  flexGloveService.addCharacteristic(thumbChar);
  flexGloveService.addCharacteristic(pointerChar);
  flexGloveService.addCharacteristic(middleChar);
  flexGloveService.addCharacteristic(ringChar);
  flexGloveService.addCharacteristic(pinkyChar);
  flexGloveService.addCharacteristic(imuChar);

  BLE.addService(flexGloveService);

  // Default characteristic values
  thumbChar.writeValue(0);
  pointerChar.writeValue(0);
  middleChar.writeValue(0);
  ringChar.writeValue(0);
  pinkyChar.writeValue(0);
  imuChar.writeValue(0);

  BLE.advertise();
  Serial.println("Waiting for BLE Central to connect...");
}

// === Arduino Main Loop ===
void loop() {
  BLEDevice central = BLE.central();
  BLE.poll(); // Good practice
  
  if (central) {
    Serial.print("Connected to: ");
    Serial.println(central.address());

    while (central.connected()) {
      readSensorsAndUpdate(true, true, true,PRINT_XY);
    }

    Serial.print("Disconnected from: ");
    Serial.println(central.address());
    BLE.advertise();
    Serial.println("Re-advertising...");
  } else {
    readSensorsAndUpdate(false, true, false,PRINT_XY);
  }
}

