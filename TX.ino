#include <SPI.h>
#include <RF24.h>

#define DURATION  200
#define DEBUG     1

RF24 radio(9, 8);  // CE, CSN
const byte address[6] = "12345";

uint16_t thresholds[] = {285, 310, 330, 360, 400, 425, 469, 540, 625, 720, 845, 1024};
char keys[] = { '#', '9', '6', '3', '0', '8', '5', '2', '*', '7', '4', '1' };

const char correctPassword[4] = {'0', '6', '1', '7'};  // Set your desired password
char inputPassword[4] = {'\0', '\0', '\0', '\0'};
int inputIndex = 0;

volatile bool interruptTriggered = false;  // Flag to indicate interrupt

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!radio.begin()) {
    Serial.println("RF24 module not responding!");
  } else {
    Serial.println("RF24 module responding!");
  }
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_1MBPS);
  radio.stopListening();

  // LEDs for feedback
  pinMode(4, OUTPUT);  // Red LED (incorrect password)
  pinMode(5, OUTPUT);  // Green LED (correct password)
  pinMode(3, INPUT);   // Interrupt pin (D6)

  // Ensure all LEDs are off initially
  digitalWrite(4, LOW);  // Red LED off
  digitalWrite(5, LOW);  // Green LED off

  // Attach interrupt to D6 (pin 6)
  attachInterrupt(digitalPinToInterrupt(3), handleInterrupt, RISING);
}

void handleInterrupt() {
  interruptTriggered = true;  // Set the flag to indicate the interrupt occurred
}

char getKey() {
  uint16_t value = 0;

  // Wait for interrupt to trigger analog read instead of using delay
  if (interruptTriggered) {
    value = analogRead(A5);  // Analog read on pin A5
    interruptTriggered = false;  // Reset the interrupt flag
  }

  char ch = '?';
  int numKeys = sizeof(keys) / sizeof(keys[0]);

  // Check the thresholds and map value to corresponding key
  for (int i = 0; i < numKeys; i++) {
    if (i == 0 && value < thresholds[i]) {
      ch = keys[i];
      break;
    } 
    else if (i == numKeys - 1 || value >= thresholds[i]) {
      ch = keys[i];
    } 
    else if (value >= thresholds[i - 1] && value < thresholds[i]) {
      ch = keys[i];
      break;
    }
  }
  return ch;
}

void checkPassword() {
  bool correct = true;
  for (int i = 0; i < 4; i++) {
    if (inputPassword[i] != correctPassword[i]) {
      correct = false;
      break;
    }
  }

  if (correct) {
    digitalWrite(5, HIGH);  // Green LED ON (if correct)
    digitalWrite(4, LOW);   // Red LED OFF
  } else {
    digitalWrite(5, LOW);   // Green LED OFF
    digitalWrite(4, HIGH);  // Red LED ON (if incorrect)
  }

  if (radio.write(&correct, sizeof(correct))) {
    Serial.println("MSG SENT");
  } else {
    Serial.println("MSG NOT SENT");
  }

  // Reset input and indicator
  inputIndex = 0;
  memset(inputPassword, '\0', 4);
}

void loop() {
  char ch = getKey();
  
  // Only register numeric inputs ('0' to '9')
  if (ch >= '0' && ch <= '9') {
    Serial.println(ch);

    if (inputIndex < 4) {
      inputPassword[inputIndex] = ch;
      inputIndex++;
    }

    if (inputIndex == 4) {
      checkPassword();
    }
  }
}
