#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "team_addr.h"

#define DELAY 100
#define CTRL_X_PIN A0
#define CTRL_Y_PIN A1
#define ROLE 1  // User Controls

short controls[2] = {0, 0};
short data[2] = {0, 0};
int prev_success = 0;

RF24 radio(7, 8);

void setup(void)
{
  Serial.begin(9600);
  radio.begin();
  radio.setRetries(15,15); // increase the delay between retries & # of retries
  radio.setPayloadSize(sizeof(controls));
  radio.openWritingPipe(team2_addr[ROLE]);
  radio.openReadingPipe(1,team2_addr[ROLE - 1]); // index error if wrong role
  radio.startListening();
  radio.printDetails();
}

void loop(void)
{
  // only go through receive/send cycle if can first receive (wait for other side's package first)
  if ( radio.available() )
  {
    int success = readData();
    // signal to user 
    if (prev_success != success) {
      digitalWrite(LED_BUILTIN, success);
      prev_success = success;
    }
    radio.stopListening();
    // Serial.print(data[0]); Serial.print(" "); Serial.println(data[1]);
    sendControls();
    radio.startListening(); // reset to start checking for package again
  }
}

int readData(){
  int success = 0;
  while (radio.available())
  {
    radio.read( &data, sizeof(data) );
    success = 1;
  }
  return success;
}

void sendControls(){
  short x_val = analogRead(CTRL_X_PIN);
  short y_val = analogRead(CTRL_Y_PIN);
  short controls[2] = {x_val,y_val};
  bool success = radio.write(&controls, sizeof(controls));
}
