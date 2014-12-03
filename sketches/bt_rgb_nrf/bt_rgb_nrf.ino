#define rxPin 2
#define txPin 4

#include <SoftwareSerial.h>
#include <SPI.h>
//#include <nRF24L01.h>
#include <RF24.h>

SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);
int state1 = LOW;
int state2 = LOW;
int state3 = LOW;

// CE,CSN пины
RF24 radio(9,10);

void setup()  
{
  Serial.begin(38400);

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(8, OUTPUT);

  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(0x4c);
  radio.openWritingPipe(0xF0F0F0F0E1LL);
  radio.enableDynamicPayloads();
  radio.powerUp();

  mySerial.begin(9600);
}

void loop() // run over and over
{
  String command = "";
  char outBuffer[32]= "";
  while (mySerial.available() > 0)
  {
    char c = mySerial.read();
    Serial.println(c);
    command.concat(c);
  }
  
  command.trim();
  if (command == "")
    return;

  Serial.println("command is -> " + command);

  if (command == "r")
  {
    if (state3 == LOW)
      state3 = HIGH;
    else if (state3 == HIGH)
      state3 = LOW;
      
    digitalWrite(8, state3);
  }

  if (command == "g")
  {
    if (state2 == LOW)
      state2 = HIGH;
    else if (state2 == HIGH)
      state2 = LOW;
      
    digitalWrite(12, state2);
  }

  if (command == "b")
  {
    if (state1 == LOW)
      state1 = HIGH;
    else if (state1 == HIGH)
      state1 = LOW;
      
    digitalWrite(13, state1);
  }

  String out = "dev1:p1:" + command;
  out.toCharArray(outBuffer, 32);
  radio.write(outBuffer, 32);
}

