#include <Arduino.h>

// S-Box for SubBytes step (example values, replace with actual S-box)
const uint8_t sbox[256] = {
    // Fill in with S-box values (example, this is not the actual S-Box)
    0x00, 0x01, 0x02, // ... complete the S-Box
};

// Function to perform the AddRoundKey step
void addRoundKey(uint8_t* state, const uint8_t* roundKey) {
    for (int i = 0; i < 16; i++) {
        state[i] ^= roundKey[i];
    }
}

// Function to SubBytes
void subBytes(uint8_t* state) {
    for (int i = 0; i < 16; i++) {
        state[i] = sbox[state[i]];
    }
}

// Function to ShiftRows
void shiftRows(uint8_t* state) {
    // Implement ShiftRows logic (simple example)
    uint8_t temp[16];
    for (int i = 0; i < 4; i++) {
        temp[i] = state[i];
        temp[i + 4] = state[(i + 5) % 4 + 4];
        temp[i + 8] = state[(i + 10) % 4 + 8];
        temp[i + 12] = state[(i + 15) % 4 + 12];
    }
    for (int i = 0; i < 16; i++) {
        state[i] = temp[i];
    }
}

// AES Encrypt function (simplified)
void aesEncrypt(const uint8_t* input, const uint8_t* key, uint8_t* output) {
    uint8_t state[16];

    // Copy input to state
    memcpy(state, input, 16);

    // Initial AddRoundKey
    addRoundKey(state, key);

    // Main round (simplified example)
    subBytes(state);
    shiftRows(state);
    // Note: MixColumns and further rounds would be implemented here

    // Final AddRoundKey
    addRoundKey(state, key);  // Normally you'd use a different round key

    // Copy state to output
    memcpy(output, state, 16);
}

void setup() {
    Serial.begin(9600);
    delay(1000); // Allow time for serial to initialize

    // Check if Serial is connected
    if (!Serial) {
        Serial.println("Serial not connected.");
        return;
    }

    Serial.println("Setup complete.");

    const char* message = "Hello, World!!!"; // 16 bytes
    uint8_t key[16] = { 0x00 }; // Use a simple key for testing
    uint8_t encrypted[16];

    // Print original message
    Serial.print("Original Message: ");
    for (int i = 0; i < 16; i++) {
        Serial.print(message[i]);
    }
    Serial.println();

    // Encrypt message
    Serial.println("Encrypting message...");
    aesEncrypt((uint8_t*)message, key, encrypted);

    // Print encrypted message
    Serial.print("Encrypted Message: ");
    for (int i = 0; i < 16; i++) {
        Serial.print(encrypted[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void loop() {
    // Nothing to do here
}
