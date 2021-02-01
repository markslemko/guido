/*
   Parking Assistant Using Range Sensor
   Created by Mark Slemko - October 5, 2013

   Parts taken from: http://arduino.cc/en/Tutorial/AnalogInput

   refined 2015-10-17
     - adjusted to accomodate shorter distance
*/

#include <math.h>
#include <SoftwareSerial.h>
SoftwareSerial Serial7Segment(8, 9); //RX pin, TX pin

int pingPin = 2;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED

int colorR = 7;
int colorGnd = 6;
int colorG = 4;
int colorB = 5;


int settleTime = 20000; // when same value is reached for this time, stop checking as often
int recheckTime = 0;
int timer = 0;
double settleDistance = 0.0;
double variance = 2.0;

#define STOP_DISTANCE     70.0
#define STOP_BRACKET      6.0
#define MAX_RECHECK_TIME  2000
#define NORMAL_RECHECK_TIME 100

#define SENSOR_ACTIVE     250.0
#define SENSOR_TOO_CLOSE  20.0

#define LED_ACTIVE_PING_INDICATOR_TIME 10

void setColor( int r, int g, int b) {
  digitalWrite(colorGnd, LOW);
  digitalWrite(colorR, r);
  digitalWrite(colorG, g);
  digitalWrite(colorB, b);
}

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);  
  
  pinMode(colorR, OUTPUT);
  pinMode(colorGnd, OUTPUT);
  pinMode(colorG, OUTPUT);
  pinMode(colorB, OUTPUT);
  
  setColor(LOW, LOW, LOW);
  
  Serial.begin(9600);
  Serial7Segment.begin(9600); //Talk to the Serial7Segment at 9600 bps
  Serial7Segment.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 343 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  double cm = (double)microseconds / 58.32;  // 29.16 * 2 us
  return (long)cm;
}

long sonicPing() {
  long duration, cm;
  
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(20);
  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH) - 350;
  Serial.print(duration);
  Serial.print("us ");
  cm = microsecondsToCentimeters(duration);

  return cm;
}

void loop() {
  // read the value from the sensor:
  double distance = sonicPing();
  Serial.print(distance);
  Serial.print("cm ");
  
  // display the distance
  char tempString[10]; //Used for sprintf
  sprintf(tempString, "%4d", (int)distance); //Convert deciSecond into a string that is right adjusted
  Serial7Segment.print(tempString);

  // give visual indication that a ping occurs
  digitalWrite(ledPin, HIGH); // turn the ledPin on  
  delay(LED_ACTIVE_PING_INDICATOR_TIME); // stop the program for a few milliseconds to show a ping just occurred
  digitalWrite(ledPin, LOW); // turn the ledPin off:        
  Serial.print(" wait ");

  // when objects are further away, make the between pings a bit longer
  timer += (int)distance + NORMAL_RECHECK_TIME;

  // check if settle time reached
  if (checkSettleTimeReached(distance)) {
    // turn off stop LED
    setColor( LOW, LOW, LOW);
    
    recheckTime = timer * 2;
    if (recheckTime < settleTime) {
      if (recheckTime > MAX_RECHECK_TIME) {
        recheckTime = MAX_RECHECK_TIME;
      }
    } 
    
    if (recheckTime < 10) {
      recheckTime = NORMAL_RECHECK_TIME;
    }
    
    Serial.print(" delay recheck ");
    Serial.print(timer);
    delay(recheckTime); // wait for next check
    
    // turn off RGBLED
    setColor( LOW, LOW, LOW);
  } else {
    recheckTime = NORMAL_RECHECK_TIME;

    setColor( LOW, LOW, LOW);
    if (shouldShowStopLED(distance)) {
      if (distance < (STOP_DISTANCE - STOP_BRACKET)) {
        // too close - blue on
        setColor( LOW, LOW, HIGH);
      } else if (distance > (STOP_DISTANCE + STOP_BRACKET)) {
        // too far - green on
        setColor( LOW, HIGH, LOW);
      } else  {
        // perfect - red on
        setColor( HIGH, LOW, LOW);
      }
    }
    delay(NORMAL_RECHECK_TIME);
  }
  Serial.println("");
}

boolean shouldShowStopLED(double distance) {
   //abs(distance - STOP_DISTANCE) < (variance * (1.0 + distance / 50.0)) + (STOP_BRACKET * 2)) {
  if ( abs(distance) < SENSOR_ACTIVE && abs(distance) > SENSOR_TOO_CLOSE) {
    return true;
  }
  return false;
}

boolean checkSettleTimeReached(double distance) {
  boolean delta = abs(settleDistance - distance) > (variance * (1.0 +  distance / 100.0));
  if (delta) {
    timer = 0;
  }
  if (timer < settleTime) {
    settleDistance = distance;
    return false;
  } else {
    return true;
  }
}
