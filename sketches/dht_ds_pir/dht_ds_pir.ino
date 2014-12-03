#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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

void setupPIR()
{
    pinMode(pirPin, INPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(calibratingLedPin, OUTPUT);
    pinMode(motionLedPin, OUTPUT);
    pinMode(noMotionLedPin, OUTPUT);
    digitalWrite(pirPin, LOW);

    //give the sensor some time to calibrate
    Serial.print("calibrating sensor ");

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

void setup()
{
    Serial.begin(9600); 
    setupPIR();
    setupTemperature();
}

void motionDetected(boolean isDetected)
{
    digitalWrite(motionLedPin, isDetected ? HIGH : LOW);
    digitalWrite(noMotionLedPin, isDetected ? LOW : HIGH);
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

void loop()
{
    detectMotion();
    if (motionTimer > 1000)
    {
        readTemperature();
        motionTimer = 0;
    }
}
