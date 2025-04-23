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

String classifyFingerPosition(int flexValue) {
    if (flexValue > minBendLevel0) return "Bend Level 0";
    else if (flexValue > minBendLevel1) return "Bend Level 1";
    else if (flexValue > minBendLevel2) return "Bend Level 2";
    else if (flexValue > minBendLevel3) return "Bend Level 3";
    else if (flexValue > minBendLevel4) return "Bend Level 4";
    else if (flexValue > minBendLevel5) return "Bend Level 5";
    else return "ERROR!!!    ";
}

void loop() {
    for (int i = 0; i < 5; i++) {
        int flexValue = analogRead(flexPins[i]);
        Serial.print("Finger "); Serial.print(i + 1);
        Serial.print(" - Value: "); Serial.print(flexValue);
        Serial.print(" - Classification: "); Serial.println(classifyFingerPosition(flexValue));
    }
    Serial.println("----------------------");
    delay(15); // Delay for readability
}
