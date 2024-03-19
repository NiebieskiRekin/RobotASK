#include <Arduino.h>
#include <Servo.h>

Servo myservo;  // create servo object to control a servo

// int potpin = 0;  // analog pin used to connect the potentiometer
// int val;    // variable to read the value from the analog pin

// void setup() {
//   // myservo.attach(9);  // attaches the servo on pin 9 to the servo object

//   pinMode(A0,INPUT);
// }

// void loop() {
//   Serial.println(analogRead(A0));
//   delay(100);
//   // val = analogRead(potpin);            // reads the value of the potentiometer (value between 0 and 1023)
//   // val = map(val, 0, 1023, 0, 180);     // scale it to use it with the servo (value between 0 and 180)
//   // myservo.write(val);                  // sets the servo position according to the scaled value
//   // delay(15);                           // waits for the servo to get there

//   // myservo.write(0);
//   // delay(100);
//   // myservo.write(45);
//   // delay(100);

// }


int sensorPin = A0;   // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  Serial.begin(9600);
  myservo.attach(9);
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);
  if (sensorValue < 30) {
    digitalWrite(ledPin, HIGH);
    Serial.println(sensorValue);
    myservo.write(0);
  } else {
    digitalWrite(ledPin, LOW);
    myservo.write(45);
  }
  delay(10);
}
