#include <SPI.h>
#include <RF24.h>
#include <Servo.h>

Servo myservo;
int servoPin = 6;

#define GREEN 3
#define RED 2

RF24 radio(9, 8);  // CE, CSN
const byte address[6] = "12345";

int correct = -1;  // Initialize with an invalid value

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!radio.begin()) {
    Serial.println("RF24 module not responding!");
  } else {
    Serial.println("RF24 module responding!");
    myservo.attach(servoPin);  // Attach servo if RF24 works
  }

  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_1MBPS);
  radio.startListening();

  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);

  myservo.write(0);  // Ensure gate starts in a closed position
}

void loop() {
  if (radio.available()) {  
    radio.read(&correct, sizeof(correct));  // Read the integer value
    Serial.print("Received: ");
    Serial.println(correct);

    if (correct != 0 && correct != 1) {  // Ignore invalid values
      Serial.println("Invalid data received, ignoring.");
      correct = -1;
    }
  }

  if (correct == 1) {  // Correct password received
    digitalWrite(GREEN, HIGH);  // Turn on green LED
    digitalWrite(RED, LOW);     // Ensure red LED is off

    myservo.write(180);  // Move servo to 180 degrees
    delay(3000);         // Wait 3 seconds
    myservo.write(0);    // Move servo back to 0 degrees
    delay(3000);             // Wait for closing

    digitalWrite(GREEN, LOW);   // Turn off green LED
    correct = -1;               // Reset for next input
  } 
  
  else if (correct == 0) {  // Incorrect password received
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, HIGH);  // Turn on red LED

    delay(3000);  // Keep red LED on for 3 seconds
    digitalWrite(RED, LOW);   // Turn off red LED

    correct = -1;  // Reset for next input
  }
}


