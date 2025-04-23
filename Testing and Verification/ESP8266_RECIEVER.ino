#include <ESP8266WiFi.h>
#include <espnow.h>

// Struct to store received data
typedef struct struct_message {
    char thumb[20];
    char index[20];
    char middle[20];
    char ring[20];
    char pinky[20];
} struct_message;

// Create a message object
struct_message receivedData;

// Callback function to handle received data
void onReceiveData(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    memcpy(&receivedData, incomingData, sizeof(receivedData)); // Copy received data

    Serial.println("Received Finger Positions:");
    Serial.print("Thumb: "); Serial.println(receivedData.thumb);
    Serial.print("Index: "); Serial.println(receivedData.index);
    Serial.print("Middle: "); Serial.println(receivedData.middle);
    Serial.print("Ring: "); Serial.println(receivedData.ring);
    Serial.print("Pinky: "); Serial.println(receivedData.pinky);
    Serial.println("-----------------------------------");
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA); // Set as Station mode for ESP-NOW

    if (esp_now_init() != 0) {
        Serial.println("ESP-NOW Init Failed");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE); // Set as receiver
    esp_now_register_recv_cb(onReceiveData); // Register receive callback
}

void loop() {
    // Nothing needed in loop, data is received in the callback
}


