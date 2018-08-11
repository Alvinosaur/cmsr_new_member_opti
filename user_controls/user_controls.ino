#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CTRL_X_PIN A0
#define CTRL_Y_PIN A1

#define DELAY 100
RF24 radio(7, 8); // CE, CSN
const byte address[2][6] = {"00001","00002"};
unsigned long prev_send_time;
unsigned long prev_rec_time;
short data[2] = {0,0};

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(0, address[0]);
  radio.setPALevel(RF24_PA_HIGH);
  prev_rec_time = millis();
  prev_send_time = prev_rec_time + DELAY/2;
}

void loop() {
  unsigned long cur_time = millis();
  if (cur_time - prev_rec_time > DELAY){
    radio.startListening();
    short* data = readData();
    prev_rec_time = cur_time;
    Serial.print(data[0]); Serial.print(' '); Serial.println(data[1]); 
  }
  else if (cur_time - prev_send_time > DELAY){
    radio.stopListening();
    sendControls();
    prev_send_time = cur_time;
  }
}

void sendControls(){
  short x_val = analogRead(CTRL_X_PIN);
  short y_val = analogRead(CTRL_Y_PIN);
  short controls[] = {x_val,y_val};
  radio.write(&controls, sizeof(controls));
}

short* readData(){
  short data[2];
  radio.read(&data, sizeof(data));
  return data;
}
