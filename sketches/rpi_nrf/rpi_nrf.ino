#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9,10);

//we only need a write pipe, but am planning to use it later
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
int counter = 0;
char SendPayload[31];

void setup(void)
{
  Serial.begin(57600); //Debug 

  //nRF24 configurations
  radio.begin();
  radio.setChannel(0x4c);
  radio.setAutoAck(1);
  radio.setRetries(15,15);
  radio.setDataRate(RF24_250KBPS);
  radio.setPayloadSize(32);
  radio.openReadingPipe(1,pipes[0]);
  radio.openWritingPipe(pipes[1]);
  radio.startListening();
  radio.printDetails(); //for Debugging
}

void loop() {
  
  float temperature = counter;

  dtostrf(temperature, 2, 2, SendPayload);
  
  //add a tag
  strcat(SendPayload, "X");   // add first string

  //send a heartbeat
  radio.stopListening();
  bool ok = radio.write(&SendPayload, strlen(SendPayload));
  radio.startListening(); 

  Serial.println(SendPayload);  
  
  counter++;

  // slow down a bit
  delay(1000);  
}
