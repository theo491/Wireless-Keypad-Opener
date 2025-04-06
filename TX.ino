// PROJECT  :Wireless Gate Opener
// PURPOSE  :To make a wireless access control system that integrates programming, prototyping and design.
// COURSE   :ICS3U-E
// AUTHOR   :T. Park
// DATE     :2025 06 04
// MCU      :328P (Standalone)
// STATUS   :Working
// REFRENCE :http://darcy.rsgc.on.ca/ACES/TEI3M/2425/ISPs.html#logs

#include <SPI.h>                                                                                     // SPI library for communication
#include <RF24.h>                                                                                    // RF24 library for radio

#define RF_CE_PIN         9                                                                          // RF24 CE pin
#define RF_CSN_PIN        8                                                                          // RF24 CSN pin

#define RED_LED_PIN       4                                                                          // Red LED output pin
#define GREEN_LED_PIN     5                                                                          // Green LED output pin

#define INTERRUPT_PIN     3                                                                          // Input pin for interrupt
#define ANALOG_KEY_PIN    A5                                                                         // Analog pin for keypad

#define PASSWORD_LENGTH   4                                                                          // Length of the password
const char CORRECT_PASSWORD[PASSWORD_LENGTH] = {'0', '6', '1', '7'};                                 // Correct password

#define NUM_KEYS          12                                                                         // Total number of keys
const uint16_t THRESHOLDS[NUM_KEYS] = {285, 310, 330, 360, 400, 425, 469, 540, 625, 720, 845, 1024}; // ADC thresholds
const char KEYS[NUM_KEYS] = { '#', '9', '6', '3', '0', '8', '5', '2', '*', '7', '4', '1' };          // Key values

RF24 radio(RF_CE_PIN, RF_CSN_PIN);                                                                   // Create radio object
const byte ADDRESS[6] = "12345";                                                                     // Pipe address for RF

char inputPassword[PASSWORD_LENGTH] = {'\0', '\0', '\0', '\0'};                                      // User input buffer
int inputIndex = 0;                                                                                  // Current index for input
volatile bool interruptTriggered = false;                                                            // Flag for interrupt

void setup() {
  Serial.begin(9600);                                                                                // Start serial communication
  while (!Serial);                                                                                   // Wait for serial monitor

  if (!radio.begin()) {                                                                              // Try initializing RF24
    Serial.println("RF24 module not responding!");                                                   // Error message
  } else {
    Serial.println("RF24 module responding!");                                                       // Success message
  }

  radio.openWritingPipe(ADDRESS);                                                                    // Set up RF writing pipe
  radio.setPALevel(RF24_PA_MIN);                                                                     // Set low power level
  radio.setDataRate(RF24_1MBPS);                                                                     // Set RF data rate
  radio.stopListening();                                                                             // Set to transmit mode

  pinMode(RED_LED_PIN, OUTPUT);                                                                      // Set red LED as output
  pinMode(GREEN_LED_PIN, OUTPUT);                                                                    // Set green LED as output
  pinMode(INTERRUPT_PIN, INPUT);                                                                     // Set interrupt pin as input

  digitalWrite(RED_LED_PIN, LOW);                                                                    // Turn off red LED
  digitalWrite(GREEN_LED_PIN, LOW);                                                                  // Turn off green LED

  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), handleInterrupt, RISING);                    // Attach interrupt
}

void handleInterrupt() {
  interruptTriggered = true;                                                                         // Set interrupt flag
}

char getKey() {
  uint16_t value = 0;                                                                                // Initialize analog value

  if (interruptTriggered) {                                                                          // If interrupt occurred
    value = analogRead(ANALOG_KEY_PIN);                                                              // Read keypad input
    interruptTriggered = false;                                                                      // Reset the flag
  }

  char ch = '?';                                                                                     // Default unknown key

  for (int i = 0; i < NUM_KEYS; i++) {                                                               // Iterate through all keys
    if (i == 0 && value < THRESHOLDS[i]) {
      ch = KEYS[i];                                                                                  // Match first threshold
      break;
    } else if (i == NUM_KEYS - 1 || value >= THRESHOLDS[i]) {
      ch = KEYS[i];                                                                                  // Match last or above threshold
    } else if (value >= THRESHOLDS[i - 1] && value < THRESHOLDS[i]) {
      ch = KEYS[i];                                                                                  // Match in between thresholds
      break;
    }
  }
  return ch;                                                                                         // Return detected key
}

void checkPassword() {
  bool correct = true;                                                                               // Assume password correct

  for (int i = 0; i < PASSWORD_LENGTH; i++) {
    if (inputPassword[i] != CORRECT_PASSWORD[i]) {
      correct = false;                                                                               // Mismatch found
      break;
    }
  }

  digitalWrite(GREEN_LED_PIN, correct ? HIGH : LOW);                                                 // Green for correct
  digitalWrite(RED_LED_PIN, correct ? LOW : HIGH);                                                   // Red for incorrect

  if (radio.write(&correct, sizeof(correct))) {
    Serial.println("MSG SENT");                                                                      // Radio sent successfully
  } else {
    Serial.println("MSG NOT SENT");                                                                  // Radio send failed
  }

  inputIndex = 0;                                                                                    // Reset input index
  memset(inputPassword, '\0', PASSWORD_LENGTH);                                                      // Clear input buffer
}

void loop() {
  char ch = getKey();                                                                                 // Get key press

  if (ch >= '0' && ch <= '9') {                                                                       // If valid number key
    Serial.println(ch);                                                                               // Print key to serial
                                                                       
    if (inputIndex < PASSWORD_LENGTH) {
      inputPassword[inputIndex] = ch;                                                                 // Store key
      inputIndex++;                                                                                   // Increment input index
    }

    if (inputIndex == PASSWORD_LENGTH) {
      checkPassword();                                                                                // Check entered password
    }
  }
}
