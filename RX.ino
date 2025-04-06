// PROJECT  :Wireless Gate Opener
// PURPOSE  :To make a wireless access control system that integrates programming, prototyping and design for the receiver.
// COURSE   :ICS3U-E
// AUTHOR   :T. Park
// DATE     :2025 06 04
// MCU      :328P (Standalone)
// STATUS   :Working
// REFRENCE :http://darcy.rsgc.on.ca/ACES/TEI3M/2425/ISPs.html#logs

#include <SPI.h>                                           // Include SPI communication library
#include <RF24.h>                                          // Include RF24 radio module library
#include <Servo.h>                                         // Include Servo motor control library

Servo myservo;                                             // Create a servo object
int servoPin = 6;                                          // Servo connected to pin 6

#define GREEN 3                                            // Green LED connected to pin 3
#define RED   2                                            // Red LED connected to pin 2

#define GATE_OPEN_ANGLE    180                             // Angle to open the gate
#define GATE_CLOSED_ANGLE  0                               // Angle to close the gate
#define GATE_HOLD_TIME     3000                            // Time to hold the gate open/closed

RF24 radio(9, 8);                                          // Create RF24 object using CE=9, CSN=8
const byte address[6] = "12345";                           // Unique RF address

int correct = -1;                                          // Holds received result (-1 means no data yet)

void setup() {
  Serial.begin(9600);                                      // Start serial communication
  while (!Serial);                                         // Wait for Serial to be ready

    if (!radio.begin()) {                                  // Initialize the RF24 module
    Serial.println("RF24 module not responding!");
  } else {
    Serial.println("RF24 module responding!");
    myservo.attach(servoPin);                              // Attach servo to control pin
  }

  radio.openReadingPipe(1, address);                       // Open RF24 reading pipe
  radio.setPALevel(RF24_PA_MIN);                           // Set low power level
  radio.setDataRate(RF24_1MBPS);                           // Set data rate
  radio.startListening();                                  // Start listening for messages

  pinMode(GREEN, OUTPUT);                                  // Set green LED as output
  pinMode(RED, OUTPUT);                                    // Set red LED as output

  myservo.write(0);                                        // Set servo to initial closed position
}

void loop() {
  if (radio.available()) {                                 // If data available to read
    radio.read(&correct, sizeof(correct));                 // Read data into 'correct'
    Serial.print("Received: ");                
    Serial.println(correct);                               // Print received value

    if (correct != 0 && correct != 1) {                    // Ignore invalid values
      Serial.println("Invalid data received, ignoring.");
      correct = -1;                                        // Reset to invalid
    }
  }

  if (correct == 1) {                                      // If password was correct
    digitalWrite(GREEN, HIGH);                             // Turn on green LED
    digitalWrite(RED, LOW);                                // Ensure red LED is off

    myservo.write(GATE_OPEN_ANGLE);                        // Open gate
    delay(GATE_HOLD_TIME);                                 // Hold open
    myservo.write(GATE_CLOSED_ANGLE);                      // Close gate
    delay(GATE_HOLD_TIME);                                 // Hold closed

    digitalWrite(GREEN, LOW);                              // Turn off green LED
    correct = -1;                                          // Reset state
  } 
  else if (correct == 0) {                                 // If password was incorrect
    digitalWrite(GREEN, LOW);                              // Make sure green LED is off
    digitalWrite(RED, HIGH);                               // Turn on red LED (error indicator)

    delay(3000);                                           // Show red LED for 3 seconds
    digitalWrite(RED, LOW);                                // Turn off red LED

    correct = -1;                                          // Reset state
  }
}
