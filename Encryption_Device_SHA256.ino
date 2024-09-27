#include <Wire.h>
#include <Arduino.h>
#include <SHA256.h> // Include the SHA256 library

// Constants
const uint8_t deviceAddr = 0x50; // I2C address of the SRAM
const uint8_t button1Pin = 2;     // Pin for the first button (start sampling)
const uint8_t button2Pin = 3;     // Pin for the second button (generate keys)
const int micPin = A0;            // Microphone pin
const int numSamples = 8;         // Number of samples to take
uint16_t writeAddresses[numSamples]; // Store written addresses
uint8_t micValues[numSamples];     // Store mic values

// RSA Variables
uint32_t p, q, n, phi, e = 65537, d; // RSA components
unsigned long lastWriteTime = 0; // Last write time for mic data
const unsigned long writeInterval = 6; // Interval in microseconds (150 samples per second)

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

    // Button 2 for generating keys
    if (digitalRead(button2Pin) == LOW) { // Button 2 pressed
        if (!isPressingButton2) {
            isPressingButton2 = true;
            Serial.println("Button 2 pressed. Generating keys using microphone data...");
            generateKeys();
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
    uint16_t randomAddr = random(0x0000, 0xFFFF); // Random memory address

    // Store the mic value and address
    micValues[index] = micValue; // Store the mic value
    writeAddresses[index] = randomAddr; // Store the random address

    // Attempt to write to SRAM and check for errors
    if (writeByteToSRAM(deviceAddr, randomAddr, micValue & 0xFF)) {
        Serial.print("Mic Value: ");
        Serial.print(micValue);
        Serial.print(" written to SRAM at address 0x");
        Serial.println(randomAddr, HEX);
    } else {
        Serial.println("Error writing to SRAM.");
    }

    index = (index + 1) % numSamples; // Loop index
}

void generateKeys() {
    // Randomly access the addresses to sample microphone values
    for (int i = 0; i < numSamples; i++) {
        int randomIndex = random(0, numSamples);
        uint16_t addr = writeAddresses[randomIndex];
        micValues[i] = readByteFromSRAM(deviceAddr, addr); // Read back the value from RAM
    }

    // Generate new prime numbers for RSA
    p = generatePrime();
    q = generatePrime();
    n = p * q; // n = p * q
    phi = (p - 1) * (q - 1); // φ(n) = (p-1)(q-1)
    d = modInverse(e, phi); // Calculate modular inverse

    Serial.print("Public Key (e, n): ");
    Serial.print(e);
    Serial.print(", ");
    Serial.println(n);
    
    Serial.print("Private Key (d): ");
    Serial.println(d);
}

uint32_t generatePrime() {
    // Simple function to generate a random prime number
    return random(50, 100); // Random number for demonstration, should be a prime
}

uint32_t modInverse(uint32_t a, uint32_t m) {
    uint32_t m0 = m, t, q;
    uint32_t x0 = 0, x1 = 1;

    if (m == 1) return 0;

    while (a > 1) {
        q = a / m;
        t = m;

        m = a % m;
        a = t;
        t = x0;

        x0 = x1 - q * x0;
        x1 = t;
    }

    if (x1 < 0) x1 += m0;

    return x1;
}

bool writeByteToSRAM(uint8_t deviceAddr, uint16_t memAddr, uint8_t data) {
    Wire.beginTransmission(deviceAddr);
    Wire.write((memAddr >> 8) & 0xFF);
    Wire.write(memAddr & 0xFF);
    Wire.write(data); // Write the data byte

    // Check for errors in transmission
    if (Wire.endTransmission() != 0) {
        return false; // Indicate a write error
    }
    return true; // Write successful
}

uint8_t readByteFromSRAM(uint8_t deviceAddr, uint16_t memAddr) {
    Wire.beginTransmission(deviceAddr);
    Wire.write((memAddr >> 8) & 0xFF);
    Wire.write(memAddr & 0xFF);
    
    // Check for errors in transmission
    if (Wire.endTransmission() != 0) {
        Serial.println("Error reading from SRAM.");
        return 0; // Return 0 on error
    }
    
    Wire.requestFrom(deviceAddr, (uint8_t)1);
    return Wire.read();
}
