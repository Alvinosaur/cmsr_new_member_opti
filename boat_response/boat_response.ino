#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "team_addr.h"

#define DELAY 100
#define ROLE 0 // boat
#define MID_X 510
#define MID_Y 529
#define THRESH 30
#define MAX_SPEED 1020

#define TEMP A0 // analog pins to read sensor data
#define LIGHT A1
#define MOTOR_1_DIR_1 9
#define MOTOR_1_DIR_2 10 
#define MOTOR_2_DIR_1 3
#define MOTOR_2_DIR_2 4
#define MOTOR_1_SP_PIN 6 // right propeller speed
#define MOTOR_2_SP_PIN 5 // left propeller speed

int controls[2] = {0, 0}; // {Y, X}, NOTE: hold with wires leading downwards
short data[2] = {0, 0};
unsigned long prev_time = 0;
int* final_controls;

RF24 radio(7, 8); // CE, CSN pins

void setup(void) {
  Serial.begin(9600);
  final_controls = (int*)malloc(sizeof(int)*2);
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
  radio.openWritingPipe(team2_addr[ROLE]);
  radio.openReadingPipe(1,team2_addr[ROLE + 1]); // index error if wrong role
  radio.startListening();
}

void calcMotorSpeed(int* raw_controls) {
  // calculates speed of the left and right motors
  // takes in raw joystick values
  // modifies global int array, final_controls
  int x = raw_controls[0];
  int y = raw_controls[1];
  int speedLEFT, speedRIGHT;
  if (final_controls == NULL) return NULL;  // error allocating memory
  
  if (y < THRESH) {
    // cranking motors at full power
    if (abs(x - MID_X) < THRESH) {
       speedLEFT = MAX_SPEED;
       speedRIGHT = MAX_SPEED;
    }

    // Turn boat left, so turn right motor to max
    else if (x < MID_X) {
      speedRIGHT = MAX_SPEED;
      speedLEFT = map(x, 0, MID_X, 0, MAX_SPEED);
    }
    else if (x > MID_X) {
      speedLEFT = MAX_SPEED;
      // 1023 corresponds to hard right
      // so lowervalues mean more power to left motor
      speedRIGHT = map(x, MID_X, 1023, MAX_SPEED, 0);
    }
  }

  // user trying to go backwards, just stop boat
  else if (y - MID_Y > THRESH){
    speedLEFT = 0;
    speedRIGHT = 0;
  }

  final_controls[0] = speedLEFT;
  final_controls[1] = speedRIGHT;
}

void loop(void) {
  /*
  Idea is that boat should wait for user response since sensor data is
  not a priority.
  */
  if ( (millis() - prev_time) > DELAY) {
    Serial.print(controls[0]); Serial.print(" "); Serial.println(controls[1]);
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
  
  calcMotorSpeed(controls);
  Serial.print("X "); Serial.print(controls[0]);
  Serial.print(", Y "); Serial.print(controls[1]);
  Serial.print(", Right "); Serial.print(final_controls[1]);
  Serial.print(", Left "); Serial.println(final_controls[0]);
  analogWrite(MOTOR_1_SP_PIN, final_controls[1]);
  analogWrite(MOTOR_2_SP_PIN, final_controls[0]);
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

  
