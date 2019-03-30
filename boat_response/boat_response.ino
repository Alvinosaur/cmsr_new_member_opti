#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define DELAY 100
#define ROLE 0 // boat
#define TEAM 0

#define MID_X 501
#define MID_Y 506
#define THRESH 30

#define TEMP A0 // analog pins to read sensor data
#define LIGHT A1
#define MOTOR_1_DIR_1 9
#define MOTOR_1_DIR_2 10
#define MOTOR_2_DIR_1 3
#define MOTOR_2_DIR_2 4
#define MOTOR_1_SP_PIN 6 // right propeller speed
#define MOTOR_2_SP_PIN 5 // left propeller speed

byte** address;  // array of two byte ptrs
int* final_controls;
int controls[2] = {0, 0}; // NOTE: hold with wires leading backwards
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
  address = (byte**)malloc(sizeof(byte*)*2);
  final_controls = (int*)malloc(sizeof(int)*2);
  // print warning if address == NULL
  switch(TEAM) {
    case 0:
     address[0] = (byte*)"00001";
     address[1] = (byte*)"00002";
     break;

    case 1:
     address[0] = (byte*)"00003";
     address[1] = (byte*)"00004";
     break;

    default:
     address[0] = (byte*)"00001";
     address[1] = (byte*)"00002";
  }
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

void calcMotorSpeed(int* raw_controls) {
  // calculates speed of the left and right motors
  // takes in raw joystick values
  // returns array of {left_motor_speed, right_motor_speed}
  // or returns NULL if malloc error
  int x = raw_controls[0];
  int y = raw_controls[1];
  int speedLEFT, speedRIGHT;
  if (final_controls == NULL) return NULL;  // error allocating memory
  
  if (y < THRESH) {
    // cranking motors at full power
    if (abs(x - MID_X) < THRESH) {
       speedLEFT = 255;
       speedRIGHT = 255;
    }

    // Turn boat left, so turn right motor to max
    else if (x < MID_X) {
      speedRIGHT = 255;
      speedLEFT = map(x, 0, MID_X, 0, 255);
    }
    else if (x > MID_X) {
      speedLEFT = 255;
      // 1023 corresponds to hard right
      // so lowervalues mean more power to left motor
      speedRIGHT = map(x, MID_X, 1023, 255, 0);
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

void readControls() {
  while ( radio.available() ) {
    radio.read( &controls, sizeof(controls) );
  }
  calcMotorSpeed(controls);  // modifies global int array, final_controls
  if (final_controls == NULL) {
    Serial.println("Malloc error!");
    return;
  }
  Serial.print("Right "); Serial.print(final_controls[1]);
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


