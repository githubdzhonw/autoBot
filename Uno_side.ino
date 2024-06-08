// Wire Peripheral Receiver
// by Nicholas Zambetti [http://www.zambetti.com](http://www.zambetti.com)

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI Peripheral device
// Refer to the "Wire Master Writer" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

#include <Servo.h>
Servo servo;
// Ultrasonic Module pins
const int trigPin = 13; // 10 microsecond high pulse causes chirp , wait 50 us const int echoPin = 12; // Width of high pulse indicates distance
// Servo motor that aims ultrasonic sensor .
const int echoPin=12; //Width of high pulse indicates distance
const int servoPin = 11; // PWM output for hobby servo

// Motor control pins : L298N H bridge
const int enAPin = 6; // Left motor PWM speed control
const int in1Pin = 7; // Left motor Direction 1
const int in2Pin = 5; // Left motor Direction 2
const int in3Pin = 4; // Right motor Direction 1
const int in4Pin = 2; // Right motor Direction 2
const int enBPin = 3; // Right motor PWM speed control
enum Motor { LEFT, RIGHT };

// Set motor speed: 255 full ahead, −255 full reverse , 0 stop
void go( enum Motor m, int speed)
{
digitalWrite (m == LEFT ? in1Pin : in3Pin , speed > 0 ? HIGH : LOW );
digitalWrite (m == LEFT ? in2Pin : in4Pin , speed <= 0 ? HIGH : LOW );
analogWrite(m==LEFT ? enAPin:enBPin, speed <0? -speed : speed);
}
// Read distance from the ultrasonic sensor , return distance in mm //
// Speed of sound in dry air , 20C is 343 m/s
// pulseIn returns time in microseconds (10ˆ−6)
// 2d=p10ˆ−6s343m/s=p0.00343m=p0.343mm/us
unsigned int readDistance() {
digitalWrite ( trigPin , HIGH );
delayMicroseconds (10);
digitalWrite ( trigPin , LOW );
unsigned long period = pulseIn ( echoPin, HIGH ); return period * 343 / 2000;
}

#define NUM_ANGLES 7
unsigned char sensorAngle[NUM_ANGLES] = { 60, 70, 80, 90, 100, 110, 120 };
unsigned int distance [NUM_ANGLES];
// Scan the area ahead by sweeping the ultrasonic sensor left and right
// and recording the distance observed . This takes a reading , then
// sends the servo to the next angle . Call repeatedly once every 50 ms or so .
void readNextDistance() {
static unsigned char angleIndex = 0;
static signed char step = 1;
distance [angleIndex] = readDistance ();
angleIndex += step ;
if (angleIndex==NUM_ANGLES-1) step=-1;
else if (angleIndex==0) step=1;
servo.write( sensorAngle[angleIndex] );
}

// Initial configuration
//
// Configure the input and output pins
// Center the servo
// Turn off the motors
// Test the motors
// Scan the surroundings once //
void setup() {
pinMode(trigPin, OUTPUT);
pinMode(echoPin, INPUT);
digitalWrite ( trigPin , LOW);
pinMode(enAPin, OUTPUT);
pinMode(in1Pin, OUTPUT);
pinMode(in2Pin, OUTPUT);
pinMode(in3Pin, OUTPUT);
pinMode(in4Pin, OUTPUT);
pinMode(enBPin, OUTPUT);
servo . attach ( servoPin );
servo . write (90);
go(LEFT, 0);
go(RIGHT, 0);

//setup comm
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
  pinMode(LED_BUILTIN, OUTPUT);

}

// Mainloop
void loop () {

readNextDistance ();
// See if something is too close at any angle
unsigned char tooClose = 0;
for (unsigned char i = 0 ; i < NUM_ANGLES ; i++)
if ( distance [i] < 100)    // was 300
tooClose = 1;
if ( tooClose ) {
// Something's nearby: back up left
go(LEFT, -50);
go(RIGHT,-170);
} else {

// Nothing in our way: go forward
//go(LEFT, 175);
//go(RIGHT, 175);
}

// Check the next direction in 50 ms
delay (50);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  while(1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer


  if (x == 0)  {
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
    go(LEFT, 0);
    go(RIGHT, 0);
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
    if (x == 1) { // turn right
      go(LEFT, 255);
      go(RIGHT, -10);
      //delay(5000);
      //x = 2;
    }
    if (x == 4) { // turn left
      go(LEFT, -10);
      go(RIGHT, 255);
      //delay(5000);
      //x = 2;
    }
    if (x == 2) { // go forward
      go(LEFT, 175);
      go(RIGHT, 175);
    }
    if (x == 3) { // go backward
      go(LEFT, -175);
      go(RIGHT, -175);
      //delay(5000);
      //x = 2;      
    }
  }

  delay (1000);

}