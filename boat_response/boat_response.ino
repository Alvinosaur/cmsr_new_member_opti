#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define MOTOR_1_DIR_1 7  // ran out of digital pins
#define MOTOR_1_DIR_2 8 
#define MOTOR_2_DIR_1 3
#define MOTOR_2_DIR_2 4
#define MOTOR_1_SP_PIN 6
#define MOTOR_2_SP_PIN 5

#define TEMP A0
#define LIGHT A1

#define DELAY 100
RF24 radio(9, 10); // CE, CSN
const byte address[][6] = {"00001","00002"};
unsigned long prev_send_time;
unsigned long prev_rec_time;
short inputs[2] = {0,0};

void setup() {
  Serial.begin(9600);
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
  analogWrite(MOTOR_1_SP_PIN, 255);
  analogWrite(MOTOR_2_SP_PIN, 255);
  radio.begin();
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(0, address[1]);
  radio.setPALevel(RF24_PA_HIGH);
  prev_send_time = millis();
  prev_rec_time = prev_send_time + DELAY/2;
}
void loop() {
  unsigned long cur_time = millis();
  if (cur_time - prev_send_time > DELAY){
    radio.stopListening();
    sendData();
    prev_send_time = cur_time;
  }
  else if (cur_time - prev_rec_time > DELAY){
    radio.startListening();
    short* inputs = readControls();
    controlMotors(inputs);
    prev_rec_time = cur_time;
    
  }
  Serial.print(inputs[0]); Serial.print(' '); Serial.println(inputs[1]); 
}

void sendData(){
  short temp = analogRead(TEMP);
  short light = analogRead(LIGHT);
  short data[2] = {temp,light};
  radio.write(&data, sizeof(data));
}

short* readControls(){
  short controls[2];
  radio.read(&controls, sizeof(controls));
  return controls;
}

void controlMotors(short* input){
  // x and y inputs in range (0, 1023)
  // we want to map that to range (0, 255)
  // if x > 127, turn right, meaning right motor decreases: (255 - x)
  // left motor will increase (x)
  // simply multiply the x values by y/1023
  // x = input[];
}

