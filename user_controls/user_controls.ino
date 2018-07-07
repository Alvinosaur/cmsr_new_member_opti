/*
* Arduino Wireless Communication Tutorial
*     Example 1 - Transmitter Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
boolean buttonState = 0;
void setup() {
  uint_8 potValues[2];
  radio.begin();
  radio.openWritingPipe(addresses[1]); // 00001
  radio.openReadingPipe(1, addresses[0]); // 00002
  radio.setPALevel(RF24_PA_HIGH); //RF24_PA_HIGH, ..MAX, 
  Serial.begin(9600);
}
void loop() {
  radio.startListening();
  if (radio.available())
  {
    radio.read(&data, sizeof(data));
    Serial.println(data);
  }
  radio.stopListening();
  }
  potValues[0] = analogRead(A0)
  potValues[1] = analogRead(A1);
  radio.write(&potValues, sizeof(potValues));
  delay(500);
}

