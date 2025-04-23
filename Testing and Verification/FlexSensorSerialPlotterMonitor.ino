const int flexPins[5] = {A0, A1, A2, A3, A4};
const int minBendLevel0 = 500;
const int minBendLevel1 = 350;
const int minBendLevel2 = 250;
const int minBendLevel3 = 170;
const int minBendLevel4 = 100;
const int minBendLevel5 = 0;

void setup() {
    Serial.begin(9600); // Start serial communication
}

int classifyFingerPosition(int flexValue) {
    if (flexValue > minBendLevel0) return 0;
    else if (flexValue > minBendLevel1) return 1;
    else if (flexValue > minBendLevel2) return 2;
    else if (flexValue > minBendLevel3) return 3;
    else if (flexValue > minBendLevel4) return 4;
    else return 5;
}

void loop() {
    Serial.print("Finger1:"); Serial.print(classifyFingerPosition(analogRead(flexPins[0])));
    Serial.print(" Finger2:"); Serial.print(classifyFingerPosition(analogRead(flexPins[1])));
    Serial.print(" Finger3:"); Serial.print(classifyFingerPosition(analogRead(flexPins[2])));
    Serial.print(" Finger4:"); Serial.print(classifyFingerPosition(analogRead(flexPins[3])));
    Serial.print(" Finger5:"); Serial.print(classifyFingerPosition(analogRead(flexPins[4])));
    Serial.println(); // Newline for Serial Plotter to process data correctly
    delay(15); // Reduce delay for real-time plotting
}
