#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CONTROLX analogRead(A0);
#define CONTROLY analogRead(A1);
RF24 radio(7, 8); // CE, CSN
const byte address[][6] = {"00001","00002"};

void setup() {
  radio.begin();
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(0, address[1]);
  radio.setPALevel(RF24_PA_HIGH);
  Serial.begin(9600);
}
void loop() {
  radio.stopListening();
  sendControls();
  radio.startListening();
  readData();
  delay(1000);
}

void sendControls(){
  uint8_t x_val = CONTROLX;
  uint8_t y_val = CONTROLY;
  uint8_t text[] = {x_val,y_val};
  radio.write(&text, sizeof(text));
}

void readData(){
  uint8_t data[2];
  radio.read(&data, sizeof(data));
  for (uint8_t i=0; i< sizeof(data); i++)
  {
    Serial.print(data[i]); Serial.print(',');
  }
  Serial.println();
}
