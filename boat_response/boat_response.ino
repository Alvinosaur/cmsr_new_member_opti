#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define DELAY 100
#define ROLE 0 // boat

#define TEMP A0 // analog pins to read sensor data
#define LIGHT A1
#define MOTOR_1_DIR_1 9
#define MOTOR_1_DIR_2 10 
#define MOTOR_2_DIR_1 3
#define MOTOR_2_DIR_2 4
#define MOTOR_1_SP_PIN 6 // right propeller speed
#define MOTOR_2_SP_PIN 5 // left propeller speed

const byte address[2][6] = {"00001","00002"};
short controls[2] = {0, 0}; // NOTE: hold with wires leading backwards
short data[2] = {0, 0};
unsigned long prev_time = 0;

RF24 radio(7, 8); // CE, CSN pins

void setup(void) {
  Serial.begin(9600);
  initMotor();
  initRadio();
}

void initMotor() {
  pinMode(MOTOR_1_DIR_1, OUTPUT);
  pinMode(MOTOR_1_DIR_2, OUTPUT);
  pinMode(MOTOR_2_DIR_1, OUTPUT);
  pinMode(MOTOR_2_DIR_2, OUTPUT);
  pinMode(MOTOR_1_SP_PIN, OUTPUT);
  pinMode(MOTOR_2_SP_PIN, OUTPUT);
  digitalWrite(MOTOR_1_DIR_1, HIGH);
  digitalWrite(MOTOR_1_DIR_2, LOW);
  digitalWrite(MOTOR_2_DIR_1, HIGH);
  digitalWrite(MOTOR_2_DIR_2, LOW);
  analogWrite(MOTOR_1_SP_PIN, 0); //initially have motors not running
  analogWrite(MOTOR_2_SP_PIN, 0);
}

void initRadio() {
  radio.begin();
  radio.setRetries(15, 15); // delay between retries & # of retries
  radio.setPayloadSize(sizeof(data));
  // Use role as index to check that role is correct
  radio.openWritingPipe(address[ROLE]);
  radio.openReadingPipe(1,address[ROLE + 1]); // index error if wrong role
  radio.startListening();
}

void loop(void) {
  /*
  Idea is that boat should wait for user response since sensor data is
  not a priority.
  */
  if ( (millis() - prev_time) > DELAY) {
    radio.stopListening();
    sendData();
    radio.startListening();
    if ( ! isTimedOut(200) ) readControls(); 
    prev_time = millis();
  }
}

void sendData() {
  short temp_val = analogRead(TEMP);
  short light_val = analogRead(LIGHT);
  short data[2] = {temp_val, light_val};
  radio.write(&data, sizeof(data)); 
}

void readControls() {
  while ( radio.available() ) {
    radio.read( &controls, sizeof(controls) );
  }
  // Serial.print(controls[0]); Serial.print(" "); Serial.println(controls[1]); 
    // x and y inputs in range (0, 1023)
  // we want to map that to range (0, 255)
  // if x > 127, turn right, meaning right motor decreases: (255 - x)
  // left motor will increase (x)
  // simply multiply the x values by y/1023
  short motorInput = map(controls[1], 0, 1023, 0, 255);
  float speed = ((float)controls[0] / (float)1023) * 2;
  Serial.print("Right "); Serial.println((255 - motorInput) * speed);
  Serial.print("Left "); Serial.println(motorInput * speed);
  analogWrite(MOTOR_1_SP_PIN, (255 - motorInput) * speed);
  analogWrite(MOTOR_2_SP_PIN, motorInput * speed);
}

bool isTimedOut(int waitThresh) {
  // Waits for a response for waitThresh ms
  // returns whether the radios finally connected
  bool timedout = false;
  unsigned long start_waiting = millis();
  while (! radio.available()) {
    if (millis() - start_waiting > waitThresh ) {
      timedout = true;
      break;
    }
  }
  return timedout;
}

  
