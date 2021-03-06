//#define rxPin 2
//#define txPin 4

//#include <SoftwareSerial.h>

//SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);
int state1 = LOW;
int state2 = LOW;
int state3 = LOW;

void setup()  
{
  Serial.begin(9600);

  //pinMode(rxPin, INPUT);
  //pinMode(txPin, OUTPUT);
  
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(8, OUTPUT);

  //mySerial.begin(9600);
}

void loop() // run over and over
{
  String command = "";
  while (Serial.available() > 0)
  {
    char c = Serial.read();
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
}

