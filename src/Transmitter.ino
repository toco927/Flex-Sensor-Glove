// Imports
#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <MPU6050_light.h>

// === User Preferences ===
#define USER_DEBUG true
#define USER_OUTPUT_SERIAL true
#define USER_OUTPUT_IMU_XY false

// === Reset Button ===
#define RESET_BUTTON_PIN 7 // Digital pin 7

// === Display Config ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// === Bluetooth Low Energy Config ===
BLEService flexGloveService("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
BLEByteCharacteristic thumbChar("473dab7c-c93a-11e9-a32f-2a2ae2dbcce1", BLERead | BLENotify); // Adding Byte Characteristics (Byte integers are capped at 255)
BLEByteCharacteristic pointerChar("473dab7c-c93a-11e9-a32f-2a2ae2dbcce2", BLERead | BLENotify);
BLEByteCharacteristic middleChar("473dab7c-c93a-11e9-a32f-2a2ae2dbcce3", BLERead | BLENotify);
BLEByteCharacteristic ringChar("473dab7c-c93a-11e9-a32f-2a2ae2dbcce4", BLERead | BLENotify);
BLEByteCharacteristic pinkyChar("473dab7c-c93a-11e9-a32f-2a2ae2dbcce5", BLERead | BLENotify);
BLEByteCharacteristic imuChar("473dab7c-c93a-11e9-a32f-2a2ae2dbcce6", BLERead | BLENotify);


// === IMU Sensor Config ===
MPU6050 mpu(Wire);


// === IMU Twitch Detection Config ===
const float TWITCH_DELTA_X = 90.0;
const float TWITCH_DELTA_Y = 62.0;
const float RETURN_MARGIN_X = 45.0;
const float RETURN_MARGIN_Y = 45.0;
const unsigned long TWITCH_WINDOW_MS = 275;
const unsigned long TWITCH_MIN_TIME_MS = 80;
const float CHAOS_Y_FOR_X = 60.0;
const float CHAOS_X_FOR_Y = 70.0;
const unsigned long IMU_COOLDOWN_DURATION_MS = 50;
unsigned long lastIMUTwitchTime = 0;
float baselineX = 0;
float baselineY = 0;
float baselineZ = 0;

// Twitch tracking
bool twitchXStarted = false;
bool twitchYStarted = false;
unsigned long twitchXStartTime = 0;
unsigned long twitchYStartTime = 0;
bool isRightTwitch = false;
bool isForwardTwitch = false;

// IMU Twitch Output Value
double imu_x = 0; // IMU
double imu_y = 0;
uint8_t imuTwitchValue = 1; // 1 = normal


// === Flex Sensor Config ===
const int flexPins[5] = {A0, A1, A2, A3, A4}; // Thumb to Pinky

// === Calibration Storage ===
int flexBaseline[5];            // Baseline flat-hand value
int bendThresholds[5][4];       // 4 thresholds for 5 levels
// Old Defaults
// const int bendLevel1 = 500;
// const int bendLevel2 = 350;
// const int bendLevel3 = 250;
// const int bendLevel4 = 170;
// const int bendLevel5 = 100;

// === Sensor Values ===
uint8_t thumbFinger = 0;  // Flex Sensors
uint8_t pointerFinger = 0;
uint8_t middleFinger = 0;
uint8_t ringFinger = 0;
uint8_t pinkyFinger = 0;


// === Timer Config ===
const int SAMPLING_TIME = 50; 
unsigned long lastUpdate = 0;

void setup() {
  Serial.begin(9600);
  delay(50);   // Time for serial to begin, don't wait for connection

  // Setting reset pin
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);  // Button pressed = LOW


  // Screen Init
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED failed! MUST RESET!");
    while (true);
  }

  // Display settings
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if(USER_DEBUG) Serial.print("Booting!");

  // Display: Booting
  display.setCursor(0, 0);
  display.println("Flex Glove Peripheral");
  display.println("BOOTING!");
  display.println("");
  display.println("Hold your hand flat.");
  display.println("");
  display.println("Wait for 5 seconds.");
  display.display();

  Serial.println("Hold your hand flat for 5 seconds.");
  delay(2000); // wait one second for display, and for user to read
  

  // IMU Init
  Wire.begin();
  byte mpuStatus = mpu.begin();
  if (mpuStatus != 0) {
    Serial.println("MPU6050 init failed! MUST RESET!");
    while (true);
  }
  
  // === IMU Angle Baseline Calibration ===
  for (int i = 0; i < 100; i++) { // takes 0.5 seconds
    mpu.update();
    baselineX += mpu.getAngleX();
    baselineY += mpu.getAngleY();
    baselineZ += mpu.getAngleZ();
    delay(5);
  }
  baselineX /= 100.0;
  baselineY /= 100.0;
  baselineZ /= 100.0;

  if (USER_DEBUG) {
    Serial.println("Angle Baselines:");
    Serial.print("X: "); Serial.println(baselineX);
    Serial.print("Y: "); Serial.println(baselineY);
    Serial.print("Z: "); Serial.println(baselineZ);
  }

  if(USER_DEBUG) Serial.println("Calculating IMU offsets...");
  delay(1000);
  mpu.calcOffsets(true, true);
  if(USER_DEBUG) Serial.println("IMU Offsets and baselines done.");
  if(USER_DEBUG) Serial.println("Calibrating Flex Sensors...");
  calibrateFlexSensors();
  if(USER_DEBUG) Serial.println("Flex Sensors Calibrated!");


  // BLE Init
  if (!BLE.begin()) {
    Serial.println("BLE init failed! MUST RESET!");

    // Display BLE ERROR
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("Flex Glove Peripheral");
    display.println("BLE ERROR! RESET!");
    display.display();
    while (true);
  }

  // BLE Advertising
  BLE.setLocalName("FlexGlove");
  BLE.setAdvertisedService(flexGloveService);
  // Adding chars to service
  flexGloveService.addCharacteristic(thumbChar);
  flexGloveService.addCharacteristic(pointerChar);
  flexGloveService.addCharacteristic(middleChar);
  flexGloveService.addCharacteristic(ringChar);
  flexGloveService.addCharacteristic(pinkyChar);
  flexGloveService.addCharacteristic(imuChar);
  // Adding service to BLE
  BLE.addService(flexGloveService);

  // Writing intial values
  thumbChar.writeValue(thumbFinger);
  pointerChar.writeValue(pointerFinger);
  middleChar.writeValue(middleFinger);
  ringChar.writeValue(ringFinger);
  pinkyChar.writeValue(pinkyFinger);
  imuChar.writeValue(imuTwitchValue);
  // Advertising BLE

  //BLE.setAdvertisingInterval(320); // do i need this?
  BLE.advertise();

  // Debug info
  if(USER_DEBUG) Serial.print("FlexGlove MAC Address: ");
  if(USER_DEBUG) Serial.println(BLE.address());
  if(USER_DEBUG) Serial.println("Device is running...");

  // Display: Scanning
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Flex Glove Peripheral");
  display.println("SCANNING!");
  display.display();
}

void loop() {
  //BLE.poll();
  BLEDevice central = BLE.central();  // find a central device
  //Serial.println("Discovering central device!");
  delay(500);

  // === Manual Reset Button Check ===
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    Serial.println("RESET BUTTON PRESSED - RESTARTING");
    delay(100);  // Debounce
    softwareReset();
  }


  if (central) {  // Connected to a device
    if(USER_DEBUG) Serial.println("Connected to central");

    // Display: connected
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Flex Glove: Sender"));
    display.println(F("CONNECTED"));
    display.display();

    while (central.connected()) {
      // Code to happen while device is connected
      BLE.poll();

      if (millis() - lastUpdate > SAMPLING_TIME) {   // consistent sampling
        lastUpdate = millis();

        readSensors(); // Reads, updates, and writes values via BLE. Outputs on Serial if needed
        displayValues(); // Displays sensor classified values on screen
      }
    }

    // After while loop, device disconnected from peripheral
    if(USER_DEBUG) Serial.print("Disconnected from central: ");
    if(USER_DEBUG) Serial.println(central.address());
    central.disconnect(); // force disconnect TODO: DELETE?

    // Display: disconnected
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Flex Glove Peripheral");
    display.println(F("DISCONNECTED"));
    display.display();

    // delay(500); // TODO: add this?
    BLE.advertise();  // ensure re-advertising starts again
    // delay(1000); // TODO: add this?

  }
}

void calibrateFlexSensors() {
  const int numSamples = 40;
  int tempSum[5] = {0, 0, 0, 0, 0};

  Serial.println("\n[!] Please hold your hand out FLAT and STILL for 2 seconds...");
  delay(1000); // wait for hand to stabilize

  for (int s = 0; s < numSamples; s++) {
    for (int i = 0; i < 5; i++) {
      tempSum[i] += analogRead(flexPins[i]);
    }
    delay(50);
  }

  for (int i = 0; i < 5; i++) {
    flexBaseline[i] = tempSum[i] / numSamples;
    int base = flexBaseline[i];

    bendThresholds[i][0] = base - 0.40 * base;  // Level 1–2
    bendThresholds[i][1] = base - 0.625 * base; // Level 2–3
    bendThresholds[i][2] = base - 0.725 * base; // Level 3–4
    bendThresholds[i][3] = base - 0.775 * base; // Level 4–5

    if(USER_DEBUG) Serial.print("Finger "); Serial.print(i);
    if(USER_DEBUG) Serial.print(" | Baseline: "); Serial.print(base);
    if(USER_DEBUG) Serial.print(" | Thresholds: ");
    for (int j = 0; j < 4; j++) {
      if(USER_DEBUG) Serial.print(bendThresholds[i][j]); Serial.print(" ");
    }
    if(USER_DEBUG) Serial.println();
  }

  if(USER_DEBUG) Serial.println("\n[✓] Flex Calibration complete!\n");
}



void readSensors() {
  mpu.update();   // calls this to get new readings

  int flexValues[5];
  for (int i = 0; i < 5; i++) {   // reading voltage dividers
    flexValues[i] = analogRead(flexPins[i]);
  }

  // Classifying finger position
  thumbFinger = classifyFingerPosition(flexValues[0], 0);
  pointerFinger = classifyFingerPosition(flexValues[1], 1);
  middleFinger = classifyFingerPosition(flexValues[2], 2);
  ringFinger = classifyFingerPosition(flexValues[3], 3);
  pinkyFinger = classifyFingerPosition(flexValues[4], 4);

  // Retrieving X and Y angles from IMU
  imu_x = mpu.getAngleX();
  imu_y = mpu.getAngleY();

  // Classifying values
  updateIMUTwitchStatus();  // classify twitch

  // Writing values into chars via BLE
  thumbChar.writeValue(thumbFinger);
  pointerChar.writeValue(pointerFinger);
  middleChar.writeValue(middleFinger);
  ringChar.writeValue(ringFinger);
  pinkyChar.writeValue(pinkyFinger);
  imuChar.writeValue(imuTwitchValue);

  // Outputting on serial if desired
  if (USER_OUTPUT_SERIAL) {
    Serial.print("T:"); Serial.print(thumbFinger);
    Serial.print(" I:"); Serial.print(pointerFinger);
    Serial.print(" M:"); Serial.print(middleFinger);
    Serial.print(" R:"); Serial.print(ringFinger);
    Serial.print(" P:"); Serial.print(pinkyFinger);
    Serial.print(" IMU:"); Serial.print(imuTwitchValue);

    if (USER_OUTPUT_IMU_XY) {
      Serial.print("  |  imuX: "); Serial.print(imu_x, 2);
      Serial.print(" imuY: "); Serial.print(imu_y, 2);
    }
    Serial.println();
  }
}

// === Flex Sensor Classification ===
int classifyFingerPosition(int flexValue, int fingerIndex) {
  if (flexValue > bendThresholds[fingerIndex][0]) return 1; // normal
  else if (flexValue > bendThresholds[fingerIndex][1]) return 2;
  else if (flexValue > bendThresholds[fingerIndex][2]) return 3;
  else if (flexValue > bendThresholds[fingerIndex][3]) return 4;
  else return 5;  // full flex
}

void updateIMUTwitchStatus() {
  mpu.update();

  float angleX = mpu.getAngleX();
  float angleY = mpu.getAngleY();
  unsigned long now = millis();

  float deltaX = angleX - baselineX;
  float deltaY = angleY - baselineY;

  bool twitchDetected = false; // New flag to track if we saw one this cycle

  // === X-axis (left/right twist) detection ===
  static float peakDeltaX = 0;
  if (!twitchXStarted && abs(deltaX) >= TWITCH_DELTA_X && abs(deltaY) <= CHAOS_Y_FOR_X) {
    twitchXStarted = true;
    twitchXStartTime = now;
    isRightTwitch = (deltaX > 0);
    peakDeltaX = deltaX;
  }

  if (twitchXStarted) {
    if (abs(deltaX) > abs(peakDeltaX)) peakDeltaX = deltaX;

    if (abs(deltaX) <= RETURN_MARGIN_X) {
      unsigned long duration = now - twitchXStartTime;
      if (duration >= TWITCH_MIN_TIME_MS && duration <= TWITCH_WINDOW_MS) {
        imuTwitchValue = isRightTwitch ? 2 : 3;
        lastIMUTwitchTime = now;
        twitchDetected = true;
      }
      twitchXStarted = false;
    }
    if (now - twitchXStartTime > TWITCH_WINDOW_MS) twitchXStarted = false;
  }

  // === Y-axis (forward/backward twitch) detection ===
  static float peakDeltaY = 0;
  if (!twitchYStarted && abs(deltaY) >= TWITCH_DELTA_Y && abs(deltaX) <= CHAOS_X_FOR_Y) {
    twitchYStarted = true;
    twitchYStartTime = now;
    isForwardTwitch = (deltaY > 0);
    peakDeltaY = deltaY;
  }

  if (twitchYStarted) {
    if (abs(deltaY) > abs(peakDeltaY)) peakDeltaY = deltaY;

    if (abs(deltaY) <= RETURN_MARGIN_Y) {
      unsigned long duration = now - twitchYStartTime;
      if (duration >= TWITCH_MIN_TIME_MS && duration <= TWITCH_WINDOW_MS) {
        imuTwitchValue = isForwardTwitch ? 4 : 5;
        lastIMUTwitchTime = now;
        twitchDetected = true;
      }
      twitchYStarted = false;
    }
    if (now - twitchYStartTime > TWITCH_WINDOW_MS) twitchYStarted = false;
  }

  // === Cooldown Reset ===
  if (!twitchDetected && (millis() - lastIMUTwitchTime) > IMU_COOLDOWN_DURATION_MS) {
    imuTwitchValue = 1; // Reset to normal
  }
}



// === Displaying Sensor Values on Screen ===
void displayValues() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Flex Glove Peripheral");
  display.println("CONNECTED");

  //display.println("OUTPUT CLASSIFIERS:");
  display.print("Thumb: ");
  display.println(thumbFinger);
  display.print("Pointer: ");
  display.println(pointerFinger);
  display.print("Middle: ");
  display.println(middleFinger);
  display.print("Ring: ");
  display.println(ringFinger);
  display.print("Pinky: ");
  display.println(pinkyFinger);
  display.print("IMU: ");
  display.println(imuTwitchValue);
  display.display();
}

void softwareReset() {
  NVIC_SystemReset();  // Full MCU reset (same as pressing reset button)
}
