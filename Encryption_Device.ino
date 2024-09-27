#include <Wire.h>

const uint8_t deviceAddr = 0x50; // I2C address of the SRAM
const uint8_t buttonPin = 2;      // Pin for the button
const uint16_t writeAddrStart = 0x0000; // Starting address to write in SRAM
const char key = 0x25; // Simple XOR key for encryption/decryption

const int micPin = A0; // Microphone pin

void setup() {
  Serial.begin(9600); // Initialize serial communication
  Wire.begin();       // Initialize I2C
  pinMode(buttonPin, INPUT_PULLUP); // Set button pin as input with pull-up

  Serial.println("Bootloader ready. Waiting for button press...");
}

void loop() {
  static unsigned long pressStartTime = 0;
  static bool isPressing = false;

  if (digitalRead(buttonPin) == LOW) { // Button pressed
    if (!isPressing) {
      pressStartTime = millis(); // Start timing the press
      isPressing = true;
    } else {
      // Check if the button has been held for 3 seconds
      if (millis() - pressStartTime >= 3000) {
        // Start sampling microphone data and store it
        for (int i = 0; i < 8; i++) { // Sample 8 times (1 byte)
          int micValue = analogRead(micPin); // Read microphone data
          uint16_t randomAddr = random(0x0000, 0xFFFF); // Random memory address
          writeByteToSRAM(deviceAddr, randomAddr, micValue & 0xFF); // Store only lower 8 bits
          Serial.print("Mic Value: ");
          Serial.print(micValue);
          Serial.print(" written to SRAM at address 0x");
          Serial.println(randomAddr, HEX);
          delayNanoseconds(150); // Delay for approximately 150 nanoseconds
        }
      }
    }
  } else { // Button released
    if (isPressing) {
      // If it was pressed for less than 3 seconds, read data
      if (millis() - pressStartTime < 3000) {
        char readBuffer[12];
        // You might want to change the reading mechanism here as you would want to read from the addresses you wrote to
        readStringFromSRAM(deviceAddr, writeAddrStart, readBuffer, sizeof(readBuffer));
        Serial.print("Encrypted data read from SRAM: ");
        Serial.println(readBuffer);
        char decryptedBuffer[12];
        decryptString(readBuffer, decryptedBuffer);
        Serial.print("Decrypted data: ");
        Serial.println(decryptedBuffer);
      }
      isPressing = false; // Reset press state
    }
  }
}

void delayNanoseconds(uint32_t ns) {
    // Assuming 16 MHz clock, each loop iteration takes about 62.5 ns
    // Adjust the loop count based on your clock speed
    uint32_t count = ns / 62.5; // Calculate how many cycles to run

    for (volatile uint32_t i = 0; i < count; i++) {
        asm volatile(""); // Empty statement to keep the loop from being optimized away
    }
}

void encryptString(const char* str, char* encrypted) {
  while (*str) {
    *encrypted++ = *str++ ^ key; // XOR with the key
  }
  *encrypted = '\0'; // Null-terminate the encrypted string
}

void decryptString(const char* encrypted, char* decrypted) {
  while (*encrypted) {
    *decrypted++ = *encrypted++ ^ key; // XOR with the key to decrypt
  }
  *decrypted = '\0'; // Null-terminate the decrypted string
}

void writeByteToSRAM(uint8_t deviceAddr, uint16_t memAddr, uint8_t data) {
  Wire.beginTransmission(deviceAddr);
  Wire.write((memAddr >> 8) & 0xFF);
  Wire.write(memAddr & 0xFF);
  Wire.write(data); // Write the data byte
  Wire.endTransmission();
  delay(20);
}

void readStringFromSRAM(uint8_t deviceAddr, uint16_t memAddr, char* buffer, size_t bufferSize) {
  Wire.beginTransmission(deviceAddr);
  Wire.write((memAddr >> 8) & 0xFF);
  Wire.write(memAddr & 0xFF);
  Wire.endTransmission();

  Wire.requestFrom(deviceAddr, bufferSize); // Request the buffer size
  size_t index = 0;
  while (Wire.available() && index < bufferSize - 1) { // Ensure space for null terminator
    buffer[index++] = Wire.read();
  }
  buffer[index] = '\0'; // Null-terminate the string
}
