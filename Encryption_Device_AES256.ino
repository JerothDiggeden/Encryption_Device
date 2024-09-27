#include <Wire.h>
#include <Arduino.h>
#include <AES.h>       // Include the AES library

// Constants
const uint8_t deviceAddr = 0x50; // I2C address of the SRAM
const uint8_t button1Pin = 2;     // Pin for the first button (start sampling)
const uint8_t button2Pin = 3;     // Pin for the second button (generate keys)
const int micPin = A0;            // Microphone pin
const int numSamples = 8;         // Number of samples to take
uint16_t writeAddresses[numSamples]; // Store written addresses
uint8_t micValues[numSamples];     // Store mic values

// AES Variables
AES aes;
uint8_t aesKey[32]; // 256-bit key for AES
uint8_t encryptedHash[16]; // AES block size (16 bytes)

// Variables for microphone sampling
unsigned long lastWriteTime = 0; // Last write time for mic data
const unsigned long writeInterval = 150; // Interval in microseconds

void setup() {
    Serial.begin(9600);
    Wire.begin();
    pinMode(button1Pin, INPUT_PULLUP); // Button 1 for sampling
    pinMode(button2Pin, INPUT_PULLUP); // Button 2 for key generation
    Serial.println("Bootloader ready. Waiting for button press...");
}

void loop() {
    static bool isPressingButton1 = false;
    static bool isPressingButton2 = false;

    // Button 1 for sampling microphone data
    if (digitalRead(button1Pin) == LOW) { // Button 1 pressed
        if (!isPressingButton1) {
            isPressingButton1 = true;
            Serial.println("Button 1 pressed. Starting microphone sampling...");
        }

        // Continuously read mic data and write to SRAM
        unsigned long currentTime = micros();
        if (currentTime - lastWriteTime >= writeInterval) {
            sampleMicrophoneData();
            lastWriteTime = currentTime; // Update last write time
        }
    } else { // Button 1 released
        if (isPressingButton1) {
            isPressingButton1 = false; // Reset press state
            Serial.println("Button 1 released. Stopping microphone sampling.");
        }
    }

    // Button 2 for generating AES key
    if (digitalRead(button2Pin) == LOW) { // Button 2 pressed
        if (!isPressingButton2) {
            isPressingButton2 = true;
            Serial.println("Button 2 pressed. Generating AES key using microphone data...");
            generateAESKey();
        }
    } else { // Button 2 released
        if (isPressingButton2) {
            isPressingButton2 = false; // Reset press state
        }
    }
}

void sampleMicrophoneData() {
    int micValue = analogRead(micPin); // Read microphone data
    static int index = 0; // Keep track of the index
    micValues[index] = micValue; // Store the mic value
    uint16_t randomAddr = random(0x0000, 0xFFFF); // Random memory address
    writeByteToSRAM(deviceAddr, randomAddr, micValue & 0xFF); // Store only lower 8 bits
    Serial.print("Mic Value: ");
    Serial.print(micValue);
    Serial.print(" written to SRAM at address 0x");
    Serial.println(randomAddr, HEX);

    index = (index + 1) % numSamples; // Loop index
}

void generateAESKey() {
    // Use microphone values to generate an AES key
    for (int i = 0; i < 32; i++) {
        aesKey[i] = micValues[i % numSamples]; // Fill the key with mic values
    }

    Serial.print("Generated AES-256 Key: ");
    for (int i = 0; i < 32; i++) {
        Serial.print(aesKey[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void writeByteToSRAM(uint8_t deviceAddr, uint16_t memAddr, uint8_t data) {
    Wire.beginTransmission(deviceAddr);
    Wire.write((memAddr >> 8) & 0xFF);
    Wire.write(memAddr & 0xFF);
    Wire.write(data); // Write the data byte
    Wire.endTransmission();
}
