#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define DELAY 100
#define TEMP A0
#define LIGHT A1
#define ROLE 0 // Boat

const byte address[2][6] = {"00001","00002"};
short controls[2] = {0, 0};
short data[2] = {0, 0};
unsigned long prev_time = 0;

RF24 radio(9, 10);

void setup(void)
{
  Serial.begin(9600);
  radio.begin();
  radio.setRetries(15,15); // optionally, increase the delay between retries & # of retries
  radio.setPayloadSize(sizeof(data));
  radio.openWritingPipe(address[ROLE]);
  radio.openReadingPipe(1,address[ROLE + 1]); // index error if wrong role
  radio.startListening();
  radio.printDetails();
}

void loop(void)
{
  if ( (millis() - prev_time) > DELAY){
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
  bool success = radio.write(&data, sizeof(data)); 
}

void readControls(){
  while ( radio.available() )
  {
    radio.read( &controls, sizeof(controls) );
  }
  Serial.print(controls[0]); Serial.print(" "); Serial.println(controls[1]); 
}

bool isTimedOut(int waitThresh){
  // Waits for a response for waitThresh ms
  // returns whether the radios finally connected
  bool timedout = false;
  unsigned long start_waiting = millis();
  while (! radio.available())
  {
    if (millis() - start_waiting > waitThresh )
    {
      timedout = true;
      break;
    }
  }
  return timedout;
}
  
