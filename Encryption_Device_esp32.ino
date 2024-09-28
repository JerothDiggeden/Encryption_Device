#include "heltec_unofficial.h"

// Global variables for RSA
int e = 65537; // Common public exponent for RSA
unsigned long long p, q, n, phi, d;

// Placeholder definitions for mic values
const int numSamples = 10; // Number of samples for mic values
uint32_t micValues[numSamples]; // Array to store mic values

// Function to check if a number is probably prime
bool isProbablePrime(uint64_t num, int k) {
    if (num <= 1) return false;
    if (num <= 3) return true;
    if (num % 2 == 0) return false;

    for (int i = 0; i < k; i++) {
        uint64_t a = random(2, num - 1);
        if (a % num == 0) continue; // Skip if a is divisible by num
        if (num % a == 0) return false; 
    }
    return true;
}

// Function to generate a prime number
uint32_t generatePrime() {
    uint32_t seed = 0;

    for (int i = 0; i < numSamples; i++) {
        micValues[i] = random(1, 100); // Replace with actual mic data
        seed ^= micValues[i]; // Combine mic values into a seed
    }

    randomSeed(seed); // Seed the random number generator
    uint64_t num;

    do {
        num = random((1ULL << 15), (1ULL << 16) - 16); // Adjust range for 16-bit primes
    } while (!isProbablePrime(num, 3)); // Ensure it’s prime
    return num;
}

// Extended GCD to compute modular inverse
long extendedGCD(long a, long b, long &x, long &y) {
    if (a == 0) {
        x = 0; y = 1;
        return b;
    }
    long x1, y1;
    long gcd = extendedGCD(b % a, a, x1, y1);
    x = y1 - (b / a) * x1;
    y = x1;
    return gcd;
}

// Function to compute modular inverse
long modInverse(long a, long m) {
    long x, y;
    long g = extendedGCD(a, m, x, y);
    if (g != 1) {
        return -1; // No inverse exists
    } else {
        return (x % m + m) % m; // Ensure positive value
    }
}

void setup() {
    Serial.begin(115200);
    heltec_setup(); // Initialize Heltec library
    heltec_display_power(true); // Ensure display is powered on
    display.init(); // Initialize the display

    // Clear the display and show initial message
    display.clear();
    display.drawString(0, 0, "Generating RSA 2048 key pair...");
    display.display();

    // Generate two distinct large prime numbers
    p = generatePrime();
    q = generatePrime();

    // Ensure p and q are different
    while (p == q) {
        q = generatePrime();
    }

    // Calculate n and phi
    n = p * q;
    phi = (p - 1) * (q - 1);

    // Calculate d (modular inverse of e mod phi)
    d = modInverse(e, phi);

    // Print the keys to Serial Monitor
    Serial.print("Public Key (e, n): ");
    Serial.print(e);
    Serial.print(", ");
    Serial.println(n);
    
    Serial.print("Private Key (d): ");
    Serial.println(d);

    // Display the keys on the Heltec display
    display.clear();
    display.drawString(0, 0, "Public Key:");
    display.drawString(0, 10, "e: " + String(e));
    display.drawString(0, 20, "n: " + String(n));
    display.drawString(0, 30, "Private Key:");
    display.drawString(0, 40, "d: " + String(d));
    display.display();
}

void loop() {
    // Your loop code (if needed)
}
