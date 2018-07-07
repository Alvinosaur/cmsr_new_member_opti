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
void setup() {
  uint_8 tempVal = 0;
  uint_8 lightVal = 0;
  radio.begin();
  radio.openWritingPipe(addresses[1]); // 00001
  radio.openReadingPipe(1, addresses[0]); // 00002
  radio.setPALevel(RF24_PA_HIGH); //RF24_PA_HIGH, ..MAX, 
}
void loop() {
  radio.startListening();
  if (radio.available())
  {
    radio.read(&controls, sizeof(controls));
    Serial.println(controls);
  }
  radio.stopListening();
  }
  tempVal = analogRead(A0);
  lightVal = analogRead(A1);
  uint_8 sensorVals[2] = {tempVal, lightVal};
  radio.write(&sensorVals, sizeof(sensorVals));
  delay(500);
}

