#include <SoftwareSerial.h>

enum LedState { LED_ON, LED_OFF };
boolean isFadeMode = false;

#define LED_CNT 6
int ledPins[LED_CNT] = {11, 10, 9, 6, 5, 3}; // pwm pins

LedState led_states[LED_CNT];

#define rxPin 2
#define txPin 4

// set up a new serial port
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

void setup()
{
  for (int i = 0; i < LED_CNT; i++)
  {
    pinMode(ledPins[i], OUTPUT); //set pwm pins to output
    led_states[i] = LED_OFF;
  }
  
  Serial.begin(38400);

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  mySerial.begin(115200);
}

void updateState()
{
  for (int i = 0; i < LED_CNT; i++)
  {
    if (led_states[i] == LED_OFF)
    {
      analogWrite(ledPins[i], 0);
    }
    else
    {
      analogWrite(ledPins[i], 255);
    }
  }
}

void loop()
{
  String command = "";
  while (mySerial.available() > 0)
  {
    char c = mySerial.read();
    Serial.println(c);
    delay(10);
    command.concat(c);
  }
  
  command.trim();
  if (command == "")
    return;

  Serial.println("command is -> " + command);
  Serial.println(command.length());

  if (command == "ab")
  {
    led_states[0] = led_states[0] == LED_OFF ? LED_ON : LED_OFF;
    led_states[1] = led_states[1] == LED_OFF ? LED_ON : LED_OFF;
    Serial.println("led 0,1 change");
  }

  if (command == "a")
  {
    led_states[0] = led_states[0] == LED_OFF ? LED_ON : LED_OFF;
    Serial.println("led 0 change");
  }
  if (command == "b")
  {
    led_states[1] = led_states[1] == LED_OFF ? LED_ON : LED_OFF;
    Serial.println("led 1 change");
  }
  if (command == "c")
  {
    led_states[2] = led_states[2] == LED_OFF ? LED_ON : LED_OFF;
    Serial.println("led 2 change");
  }
  if (command == "d")
  {
    led_states[3] = led_states[3] == LED_OFF ? LED_ON : LED_OFF;
    Serial.println("led 3 change");
  }

  char charBuf[1000];
  command.toCharArray(charBuf, 1000);
  mySerial.write(charBuf);
  mySerial.flush();
 
  updateState();
}
