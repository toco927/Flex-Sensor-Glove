#include <Wire.h>
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

// Thresholds
const float TWITCH_DELTA_X = 110.0;
const float TWITCH_DELTA_Y = 65.0;

const float RETURN_MARGIN_X = 40.0;
const float RETURN_MARGIN_Y = 40.0;

const unsigned long TWITCH_WINDOW_MS = 220;
const unsigned long TWITCH_MIN_TIME_MS = 90;

// Chaos thresholds
const float CHAOS_Y_FOR_X = 30.0;
const float CHAOS_X_FOR_Y = 45.0;

// Baselines
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

void setup() {
  Serial.begin(9600);
  Wire.begin();
  byte status = mpu.begin();
  if (status != 0) {
    Serial.print("MPU init failed with code: ");
    Serial.println(status);
    while (1);
  }

  Serial.println("MPU6050 ready");
  delay(1000);
  mpu.calcGyroOffsets();

  for (int i = 0; i < 100; i++) {
    mpu.update();
    baselineX += mpu.getAngleX();
    baselineY += mpu.getAngleY();
    baselineZ += mpu.getAngleZ();
    delay(5);
  }
  baselineX /= 100.0;
  baselineY /= 100.0;
  baselineZ /= 100.0;

  Serial.println("Angle Baselines:");
  Serial.print("X: "); Serial.println(baselineX);
  Serial.print("Y: "); Serial.println(baselineY);
  Serial.print("Z: "); Serial.println(baselineZ);
}

void loop() {
  mpu.update();
  float angleX = mpu.getAngleX();
  float angleY = mpu.getAngleY();
  unsigned long now = millis();

  float deltaX = angleX - baselineX;
  float deltaY = angleY - baselineY;

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
        Serial.print(isRightTwitch ? "Right" : "Left");
        Serial.println(" Wrist Twist Detected");
        Serial.print("  Duration (ms): ");
        Serial.println(duration);
        Serial.print("  Peak Angle ΔX: ");
        Serial.println(peakDeltaX, 2);
        delay(20);
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
        Serial.print(isForwardTwitch ? "Forward" : "Backward");
        Serial.println(" Wrist Twitch Detected");
        Serial.print("  Duration (ms): ");
        Serial.println(duration);
        Serial.print("  Peak Angle ΔY: ");
        Serial.println(peakDeltaY, 2);
        delay(20);
      }
      twitchYStarted = false;
    }

    if (now - twitchYStartTime > TWITCH_WINDOW_MS) twitchYStarted = false;
  }
}
