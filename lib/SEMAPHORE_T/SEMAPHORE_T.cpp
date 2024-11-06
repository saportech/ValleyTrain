#include "SEMAPHORE_T.h"

// Define the pulse duration in milliseconds
#define PULSE_DURATION 1000

void Semaphore::init() {
    // Set MUX select pins and output pin as output
    pinMode(MUX_OUTPUT_PIN, OUTPUT);
    pinMode(SEL0, OUTPUT);
    pinMode(SEL1, OUTPUT);
    pinMode(SEL2, OUTPUT);
    pinMode(SEL3, OUTPUT);

    // Initialize all semaphores to LOW (inactive) state
    digitalWrite(MUX_OUTPUT_PIN, LOW);
}

// Function to set all semaphores to RED non-blockingly
bool Semaphore::initToRed() {
    static uint8_t currentSemaphore = 1;  // Start from 1 to match station numbering
    static bool allDone = false;

    if (allDone) {
        allDone = false;       // Reset for the next call
        currentSemaphore = 1;  // Reset to start with the first semaphore
        Serial.println("All semaphores set to RED");
        return true;           // Indicate all semaphores have been set to RED
    }

    // Call setSemaphore for the current semaphore and check if the pulse is complete
    if (setSemaphore(currentSemaphore, RED)) {
        currentSemaphore++;        // Move to the next semaphore
        if (currentSemaphore > 6) {
            allDone = true;        // Set flag when all semaphores are processed
        }
    }

    return false; // Return false until all semaphores are set
}

// Function to pulse the semaphore channel and return true when pulse is complete
bool Semaphore::setSemaphore(uint8_t id, SemaphoreState state) {
    static unsigned long pulseStartTime = 0;
    static bool isPulsing = false;

    if (id < 1 || id > 6) return false; // Ensure ID is within range 1-6 for semaphores

    // Adjust `id` to match zero-based channel indexing
    uint8_t zeroBasedId = id - 1;

    // Calculate channels for RED and GREEN states of this semaphore
    uint8_t redChannel = zeroBasedId;           // Channels 0-5 are for RED
    uint8_t greenChannel = zeroBasedId + 6;     // Channels 6-11 are for GREEN

    // Start the pulse if it hasn't started
    if (!isPulsing) {
        // Select the appropriate channel based on the desired color
        if (state == RED) {
            selectMuxChannel(redChannel);    // Select RED channel for this semaphore
            Serial.print("Semaphore ");
            Serial.print(id);
            Serial.println(" set to RED");
        } else {
            selectMuxChannel(greenChannel);  // Select GREEN channel for this semaphore
            Serial.print("Semaphore ");
            Serial.print(id);
            Serial.println(" set to GREEN");
        }

        digitalWrite(MUX_OUTPUT_PIN, HIGH);  // Set the color to HIGH to start the pulse
        pulseStartTime = millis();           // Record the pulse start time
        isPulsing = true;                    // Set pulsing state
        return false;                        // Pulse has started but is not complete
    }

    // Check if the pulse duration has passed
    if (isPulsing && (millis() - pulseStartTime >= PULSE_DURATION)) {
        digitalWrite(MUX_OUTPUT_PIN, LOW);   // Turn off the selected color
        isPulsing = false;                   // Reset pulsing state
        return true;                         // Pulse is complete
    }

    return false; // Pulse is still in progress
}


void Semaphore::selectMuxChannel(uint8_t channel) {
    digitalWrite(SEL0, channel & 0x01);
    digitalWrite(SEL1, (channel >> 1) & 0x01);
    digitalWrite(SEL2, (channel >> 2) & 0x01);
    digitalWrite(SEL3, (channel >> 3) & 0x01);
}
