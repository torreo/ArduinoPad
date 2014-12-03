#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

//---------------------------------------

struct
{
  float humidity;
  float temperature;
  float dsTemperature;
} tempData;

//---------------------------------------

//Temperature/humidity variables
#define DHTPIN 3       // what pin we're connected to
#define ONE_WIRE_BUS 2 // Data wire is plugged into port 2 on the Arduino

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Uncomment whatever type you're using
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22 (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

int temperatureCounter = 0;
#define TEMPERATURE_PERIOD 3000

//---------------------------------------

//PIR variables
int pirPin = 7;        //the digital pin connected to the PIR sensor's output
int ledPin = 13;
int calibratingLedPin = 4;
int calibratingLedState = LOW;
int motionLedPin = 6;
int noMotionLedPin = 8;

//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 30;
//the time when the sensor outputs a low impulse
long unsigned int lowIn;

//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 5000;
boolean lockLow = true;
boolean takeLowTime;

unsigned long int motionTimer = 0;

//---------------------------------------

//NRF variables
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
char sendPayload[31];

RF24 radio(9, 10);

//---------------------------------------

//LDR pins

const int ldrInPin = A0;
int ldrValue = 0;

//---------------------------------------

void setupLDR()
{
}

void setupPIR()
{
    pinMode(pirPin, INPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(calibratingLedPin, OUTPUT);
    pinMode(motionLedPin, OUTPUT);
    pinMode(noMotionLedPin, OUTPUT);
    digitalWrite(pirPin, LOW);

    //give the sensor some time to calibrate
    Serial.print("Calibrating PIR sensor ");

    for(int i = 0; i < calibrationTime; i++)
    {
        Serial.print(".");
        calibratingLedState = calibratingLedState == LOW ? HIGH : LOW;
        digitalWrite(calibratingLedPin, calibratingLedState);
        delay(1000);
    }

    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    digitalWrite(calibratingLedPin, LOW);
    motionDetected(false);
    delay(50);
}

void setupTemperature()
{
    dht.begin();
    sensors.begin();
}

void setupRadio()
{
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

void setup()
{
    Serial.begin(9600);
    setupLDR();
    setupPIR();
    setupTemperature();
    setupRadio();
}

void motionDetected(boolean isDetected)
{
    digitalWrite(motionLedPin, isDetected ? HIGH : LOW);
    digitalWrite(noMotionLedPin, isDetected ? LOW : HIGH);
}

void readLightness()
{
    ldrValue = analogRead(ldrInPin);
    ldrValue = map(ldrValue, 0, 1023, 0, 255);
    Serial.print("l = ");
    Serial.println(ldrValue);
}

void readTemperature()
{
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    //DS sensor
    sensors.requestTemperatures(); // Send the command to get temperatures
    float dsTemperature = sensors.getTempCByIndex(0);
    
    tempData.humidity = humidity;
    tempData.temperature = temperature;
    tempData.dsTemperature = dsTemperature;
    
    Serial.print("h = ");
    Serial.println(tempData.humidity);
    Serial.print("t = ");
    Serial.println(tempData.temperature);
    Serial.print("ds = ");
    Serial.println(tempData.dsTemperature);
}

void detectMotion()
{
    if (digitalRead(pirPin) == HIGH)
    {
        digitalWrite(ledPin, HIGH);   //the led visualizes the sensors output pin state
        if (lockLow)
        {
            //makes sure we wait for a transition to LOW before any further output is made:
            lockLow = false;
            Serial.println("---");
            Serial.print("motion detected at ");
            Serial.print(millis()/1000);
            Serial.println(" sec");
            motionDetected(true);
            delay(50);
        }
        takeLowTime = true;
    }

    if (digitalRead(pirPin) == LOW)
    {
        digitalWrite(ledPin, LOW);  //the led visualizes the sensors output pin state

        if (takeLowTime)
        {
            lowIn = millis();          //save the time of the transition from high to LOW
            takeLowTime = false;       //make sure this is only done at the start of a LOW phase
        }

        //if the sensor is low for more than the given pause, 
        //we assume that no more motion is going to happen
        if (!lockLow && millis() - lowIn > pause)
        {
            //makes sure this block of code is only executed again after 
            //a new motion sequence has been detected
            lockLow = true;
            Serial.print("motion ended at ");      //output
            Serial.print((millis() - pause)/1000);
            Serial.println(" sec");
            motionDetected(false);
            delay(50);
        }
    }
    
    motionTimer = millis();
}

void sendData()
{
    memset(sendPayload, 0, sizeof(sendPayload));
    char humidityString[10];
    char temperatureString[10];
    char dsTemperatureString[10];
    char lightnessString[10];
    dtostrf(tempData.humidity, 2, 2, humidityString);
    dtostrf(tempData.temperature, 2, 2, temperatureString);
    dtostrf(tempData.dsTemperature, 2, 2, dsTemperatureString);
    dtostrf(ldrValue, 3, 0, lightnessString);
    
    strcat(sendPayload, humidityString);
    strcat(sendPayload, "|");
    strcat(sendPayload, temperatureString);
    strcat(sendPayload, "|");
    strcat(sendPayload, dsTemperatureString);
    strcat(sendPayload, "|");
    strcat(sendPayload, lightnessString);
    strcat(sendPayload, "X");

    //send a heartbeat
    radio.stopListening();
    bool ok = radio.write(&sendPayload, strlen(sendPayload));
    radio.startListening();
    
    Serial.print("Data to send: ");
    Serial.println(sendPayload);
}

void loop()
{
    detectMotion();
    if (motionTimer > TEMPERATURE_PERIOD)
    {
        readTemperature();
        readLightness();
        sendData();
        motionTimer = 0;
    }
}
