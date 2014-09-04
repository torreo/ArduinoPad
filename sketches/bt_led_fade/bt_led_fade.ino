#include <SoftwareSerial.h>

enum LedState { LED_ON, LED_OFF };
boolean isFadeMode = false;

#define LED_CNT 6
int ledPins[LED_CNT] = {11, 10, 9, 6, 5, 3}; // pwm pins
int ledBrightnessesWave[LED_CNT] = {0, 50, 100, 150, 205, 255}; // represents initial brightness for wave mode
int ledStepsWave[LED_CNT] = {-5, 5, 5, 5, 5, 5}; // represents initial change for wave mode
int ledBrightnesses[LED_CNT]; // represents brightness for pin
int ledSteps[LED_CNT]; // represents change, each pin gets its own change so it wont interfere with any other pin

int ledSpeed = 50;
int maxLedBrightness = 255;
int ledBrightnessesConst = 255; // represents brightness for pins in no wave or fade mode

LedState led_state;

#define rxPin 2
#define txPin 4

#define SPEED_PREFIX 'G'
#define SPEED_PREFIX_MAX 'P'
#define BRIGHTNESS_PREFIX 'Q'
#define BRIGHTNESS_PREFIX_MAX 'Z'

#define MIN_SPEED_DELAY 5
#define MAX_SPEED_DELAY 100
#define MIN_BRIGHTNESS 5
#define MAX_BRIGHTNESS 255

// set up a new serial port
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

void setup()
{
  for (int i = 0; i < LED_CNT; i++)
  {
    pinMode(ledPins[i], OUTPUT); //set pwm pins to output

    //copy initial values
    ledBrightnesses[i] = ledBrightnessesWave[i];
    ledSteps[i] = ledStepsWave[i];
  }
  
  led_state = LED_ON;
  
  Serial.begin(9600);

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  mySerial.begin(115200);
}

int getBrightness(int b)
{
  return b;
}

int valueToDelay(int value)
{
  return MIN_SPEED_DELAY + (MAX_SPEED_DELAY - MIN_SPEED_DELAY) * value / (SPEED_PREFIX_MAX - SPEED_PREFIX);
}

int valueToBrightness(int value)
{
  return MIN_BRIGHTNESS + (MAX_BRIGHTNESS - MIN_BRIGHTNESS) * value / (BRIGHTNESS_PREFIX_MAX - BRIGHTNESS_PREFIX);
}

void recalculateBrightness(int brValue)
{
  ledBrightnessesConst = valueToBrightness(brValue);
}

void ledFade()
{
  if (led_state == LED_OFF)
  {
    for (int i = 0; i < LED_CNT; i++)
    {
      analogWrite(ledPins[i], ledBrightnessesConst); // update all pins
    }
    
    return;
  }

  String s = "###";
  for (int i = 0; i < LED_CNT; i++)
  {
    analogWrite(ledPins[i], getBrightness(ledBrightnesses[i]));

    int newBr = ledBrightnesses[i] + ledSteps[i];
    if (newBr <= 0 || newBr >= maxLedBrightness)
    {
      ledSteps[i] =- ledSteps[i]; //change direction if exceeds max/min value
    }
    else
    {
      ledBrightnesses[i] = newBr;
    }

    s.concat(ledBrightnesses[i]);
    if (i < LED_CNT - 1) //skip separator for last entity
      s.concat("-");
  }
  s.concat("***");
  
  char charBuf[1000];
  s.toCharArray(charBuf, 1000);
  mySerial.write(charBuf);
  mySerial.flush();
  
  delay(ledSpeed);
}

void loop()
{
  if (mySerial.available())
  {
    char command = mySerial.read();
    Serial.println("command is -> " + command);
    
    boolean handled = false;
    boolean changeFadeMode = false;
    
    switch (command)
    {
      case 'a':
        led_state = LED_OFF;
        handled = true;
        Serial.println("led off");
        break;
      case 'b':
        led_state = LED_ON;
        ledSpeed = 50;
        handled = true;
        //do these lines to set wave mode
        changeFadeMode = true;
        isFadeMode = true;
        Serial.println("speed 50");
        break;
      case 'c':
        led_state = LED_ON;
        ledSpeed = 30;
        handled = true;
        //do these lines to set wave mode
        changeFadeMode = true;
        isFadeMode = true;
        Serial.println("speed 30");
        break;
      case 'd':
        led_state = LED_ON;
        ledSpeed = 10;
        handled = true;
        //do these lines to set wave mode
        changeFadeMode = true;
        isFadeMode = true;
        Serial.println("speed 10");
        break;
      case 'e':
        led_state = LED_ON;
        changeFadeMode = true;
        handled = true;
        Serial.println("change fade mode");
        break;
      case 'f':
        led_state = LED_ON;
        handled = true;
        Serial.println("led on");
        break;
      default:
        break;
    }
    
    if (changeFadeMode)
    {
      if (isFadeMode)
      {
        Serial.println("Set fade mode off");
        for (int i = 0; i < LED_CNT; i++)
        {
          //copy initial values
          ledBrightnesses[i] = ledBrightnessesWave[i];
          ledSteps[i] = ledStepsWave[i];
        }
      }
      else
      {
        Serial.println("Set fade mode on");
        for (int i = 0; i < LED_CNT; i++)
        {
          ledBrightnesses[i] = 0;
          ledSteps[i] = 5;
        }
      }
      isFadeMode = !isFadeMode;
    }
    else
    {
      //do nothing
    }
    
    if (!handled)
    {
      boolean isSpeedCommand = command >= SPEED_PREFIX && command <= SPEED_PREFIX_MAX;
      boolean isBrCommand = command >= BRIGHTNESS_PREFIX && command <= BRIGHTNESS_PREFIX_MAX;
      
      if (isSpeedCommand)
      {
        led_state = LED_ON;
        int speedValue = command - SPEED_PREFIX; //from 0 to 9
        ledSpeed = valueToDelay(speedValue);
      }
      if (isBrCommand)
      {
        led_state = LED_OFF;
        int brValue = command - BRIGHTNESS_PREFIX; //from 0 to 9
        recalculateBrightness(brValue);
      }
    }
  }
  
  ledFade();
}
