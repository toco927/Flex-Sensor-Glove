// === User Preferences ===
#define USER_DEBUG true
#define USER_OUTPUT_SERIAL true
#define USER_OUTPUT_RAW true
#define USER_OUTPUT_IMU_XY false

// === Flex Sensor Config ===
const int flexPins[5] = {A0, A1, A2, A3, A4}; // Thumb to Pinky

// === Sensor Values ===
uint8_t thumbFinger = 0;
uint8_t pointerFinger = 0;
uint8_t middleFinger = 0;
uint8_t ringFinger = 0;
uint8_t pinkyFinger = 0;

// === Calibration Storage ===
int flexBaseline[5];            // Baseline flat-hand value
int bendThresholds[5][4];       // 4 thresholds for 5 levels

// === Finger FSM State ===
struct FingerState {
  uint8_t prevLevel = 1;
  bool isBending = false;
  uint8_t peakLevel = 1;
};

FingerState fingerStates[5];

// === Setup ===
void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("=== FLEX SENSOR CALIBRATION ===");
  delay(1000);
  calibrateFlexSensors();   // Run calibration
  delay(5000);
}

// === Loop ===
void loop() {
  readSensors();
  delay(50);
}

// === Calibration ===
void calibrateFlexSensors() {
  Serial.println("\n[!] Please hold your hand out FLAT and STILL for 2 seconds...");

  const int numSamples = 40;
  int tempSum[5] = {0, 0, 0, 0, 0};
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

    // Set nonlinear thresholds for levels 2–5
    bendThresholds[i][0] = base - 0.40 * base;  // Level 1–2
    bendThresholds[i][1] = base - 0.625 * base; // Level 2–3
    bendThresholds[i][2] = base - 0.725 * base; // Level 3–4
    bendThresholds[i][3] = base - 0.775 * base; // Level 4–5

    Serial.print("Finger "); Serial.print(i);
    Serial.print(" | Baseline: "); Serial.print(base);
    Serial.print(" | Thresholds: ");
    for (int j = 0; j < 4; j++) {
      Serial.print(bendThresholds[i][j]); Serial.print(" ");
    }
    Serial.println();
  }

  Serial.println("\n[✓] Calibration complete! Levels 1–5 configured.\n");
}

// === Sensor Reading + Processing ===
void readSensors() {
  int flexValues[5];
  uint8_t levels[5];

  for (int i = 0; i < 5; i++) {
    flexValues[i] = analogRead(flexPins[i]);
    levels[i] = classifyFingerPosition(flexValues[i], i);
    processFingerLevel(i, levels[i]);
  }

  if (USER_OUTPUT_RAW) {
    Serial.print("T:");  Serial.print(flexValues[0]); Serial.print(" ("); Serial.print(levels[0]); Serial.print(")  | ");
    Serial.print("I:");  Serial.print(flexValues[1]); Serial.print(" ("); Serial.print(levels[1]); Serial.print(")  | ");
    Serial.print("M:");  Serial.print(flexValues[2]); Serial.print(" ("); Serial.print(levels[2]); Serial.print(")  | ");
    Serial.print("R:");  Serial.print(flexValues[3]); Serial.print(" ("); Serial.print(levels[3]); Serial.print(")  | ");
    Serial.print("P:");  Serial.print(flexValues[4]); Serial.print(" ("); Serial.print(levels[4]); Serial.print(")");
    Serial.println();
  }
}

// === Classification Logic ===
int classifyFingerPosition(int flexValue, int fingerIndex) {
  if (flexValue > bendThresholds[fingerIndex][0]) return 1;
  else if (flexValue > bendThresholds[fingerIndex][1]) return 2;
  else if (flexValue > bendThresholds[fingerIndex][2]) return 3;
  else if (flexValue > bendThresholds[fingerIndex][3]) return 4;
  else return 5;
}

// === Per-Finger State Machine ===
void processFingerLevel(int fingerIndex, uint8_t currentLevel) {
  FingerState &state = fingerStates[fingerIndex];

  if (currentLevel == 1) {
    if (state.isBending && state.peakLevel > 1) {
      triggerFingerAction(fingerIndex, state.peakLevel);
    }
    state.isBending = false;
    state.peakLevel = 1;
  } else {
    state.isBending = true;
    if (currentLevel > state.peakLevel) {
      state.peakLevel = currentLevel;
    }
  }

  state.prevLevel = currentLevel;
}

// === Placeholder Action ===
void triggerFingerAction(int fingerIndex, uint8_t level) {
  Serial.print("Finger ");
  Serial.print(fingerIndex);
  Serial.print(" TRIGGERED at peak level ");
  Serial.println(level);
  delay(10); // Mimic keypress duration
}
