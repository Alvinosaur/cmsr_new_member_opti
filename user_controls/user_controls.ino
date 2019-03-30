#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define DELAY 100
#define CTRL_X_PIN A0
#define CTRL_Y_PIN A1
#define ROLE 1  // User Controls
#define TEAM 0

byte** address;  // array of two byte ptrs
short controls[2] = {0, 0};
short data[2] = {0, 0};

RF24 radio(7, 8);

void setup(void)
{
  Serial.begin(9600);
  radio.begin();
  radio.setRetries(15,15); // increase the delay between retries & # of retries
  radio.setPayloadSize(sizeof(controls));
  radio.openWritingPipe(address[ROLE]);
  radio.openReadingPipe(1,address[ROLE - 1]); // index error if wrong role
  radio.startListening();
  radio.printDetails();
  address = (byte**)malloc(sizeof(byte*)*2);
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

void loop(void)
{
  // only go through receive/send cycle if can first receive (wait for other side's package first)
  if ( radio.available() )
  {
    readData();
    radio.stopListening();
    sendControls();
    radio.startListening(); // reset to start checking for package again
  }
}

void readData(){
  while ( radio.available() )
  {
    radio.read( &data, sizeof(data) );
  }
}

void sendControls(){
  short x_val = analogRead(CTRL_X_PIN);
  short y_val = analogRead(CTRL_Y_PIN);
  short controls[2] = {x_val,y_val};
  bool success = radio.write(&controls, sizeof(controls));
}
