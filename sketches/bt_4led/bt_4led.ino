#include <SoftwareSerial.h>

enum LedState { LED_ON, LED_OFF };
boolean isFadeMode = false;

#define LED_CNT 4
int ledPins[LED_CNT] = {9, 6, 5, 3}; // pwm pins

LedState led_states[LED_CNT];
int led_level[LED_CNT];

#define rxPin 2
#define txPin 4

#define CMD_STEPS 6
char forwardPrefix = 'A';
char backwardPrefix = (char)((int)forwardPrefix + CMD_STEPS);
char leftPrefix = (char)((int)backwardPrefix + CMD_STEPS);
char rightPrefix = (char)((int)leftPrefix + CMD_STEPS);

// set up a new serial port
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

void setup()
{
  for (int i = 0; i < LED_CNT; i++)
  {
    pinMode(ledPins[i], OUTPUT); //set pwm pins to output
    led_states[i] = LED_OFF;
    led_level[i] = 0;
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
      analogWrite(ledPins[i], led_level[i]);
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
    command.concat(c);
  }
  
  command.trim();
  if (command == "")
    return;

  Serial.println("command is -> " + command);

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
  
  char moveCommand = command[0];
  if (moveCommand >= forwardPrefix && moveCommand < backwardPrefix)
  {
    led_level[0] = (moveCommand - forwardPrefix) * (255 / (CMD_STEPS - 1));
  }
  if (moveCommand >= backwardPrefix && moveCommand < leftPrefix)
  {
    led_level[1] = (moveCommand - backwardPrefix) * (255 / (CMD_STEPS - 1));
  }
  if (moveCommand >= leftPrefix && moveCommand < rightPrefix)
  {
    led_level[2] = (moveCommand - leftPrefix) * (255 / (CMD_STEPS - 1));
  }
  if (moveCommand >= rightPrefix && moveCommand < (rightPrefix + CMD_STEPS + 1))
  {
    led_level[3] = (moveCommand - rightPrefix) * (255 / (CMD_STEPS - 1));
  }

  if (moveCommand == 'G' || moveCommand == 'A')
  {
    led_level[0] = led_level[1] = 0;
  }
  if (moveCommand == 'S' || moveCommand == 'M')
  {
    led_level[2] = led_level[3] = 0;
  }

  Serial.print("forward ");
  Serial.println(led_level[0]);
  Serial.print("back ");
  Serial.println(led_level[1]);
  Serial.print("left ");
  Serial.println(led_level[2]);
  Serial.print("right ");
  Serial.println(led_level[3]);

  command = "#" + command + "*";
  char charBuf[5];
  command.toCharArray(charBuf, 5);
  mySerial.write(charBuf);
  mySerial.flush();
 
  updateState();
}
