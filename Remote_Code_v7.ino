/*
 *ENR 281 Engineering Design Lab
 *Red Team Car Code 
 *Transmitter V7.5.2 from 04/12/2021
 */
 
#include <SoftwareSerial.h> //Include software serial library to comunicate with HC-12 radio
SoftwareSerial radioSerial(3,4); //Set pins for software serial connection

//Variables for joystick
int xaxis = A3; //X axis pin
int yaxis = A2; //Y axis pin
int xpos = 512; //Starting position for x axis
int ypos = 512; //Starting position for y axis

//LED Pins
int blueLED = 7;
int yellowLED = A1;
int redLED = 6;
int green1LED = 5;
int green2LED = A0;

//Button Pins
int upButton = 2;
int downButton = A4;
int leftButton = 3;
int rightButton = A5;


//Input smoothing variables
const int numReadings = 20;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int average = 0;

//Array
int message[4]; //Array that holds data to send

// Setup code
void setup() {

  //Fill array
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  //Set LEDs pinmode
  pinMode(blueLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(green1LED, OUTPUT);
  pinMode(green2LED, OUTPUT);

  //Set buttons pinmode
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);

  Serial.begin(9600); //Open serial connection for debugging
  radioSerial.begin(9600); //Open serial connection to wireless radio

  //Fancy light show
  digitalWrite(blueLED, HIGH);
  delay(500);
  digitalWrite(redLED, HIGH);
  delay(500);
  digitalWrite(yellowLED, HIGH);
  delay(500);
  digitalWrite(green1LED, HIGH);
  delay(500);
  digitalWrite(green2LED, HIGH);
}


//Main loop
void loop() {
//Throttle control
//If the boost button is not pressed, re-map steering to make the input less sensitive and store the values.
  if (digitalRead(upButton) < 1) {
    //Boost on
    digitalWrite(redLED, HIGH); //Turn on boost indicator
    message[0] = map(analogRead(xaxis), 0, 1023, 0, 1023); //Store x axis
  } else {
    //Boost off
    digitalWrite(redLED,LOW); //Turn off boost indicator
    //Re-map Forward
    if (analogRead(xaxis) > 511) {
      message[0] = map(analogRead(xaxis), 511, 960, 512, 562); //Store mapped x axis
    //Re-map Backward
    } else {
      message[0] = map(analogRead(xaxis), 44, 511, 462, 512); //Store mapped x axis
    }
  }


//Steering 
//Re-map steering to make the input less sensitive and store the values
    if (analogRead(yaxis) > 578) {
      message[1] = map(analogRead(yaxis), 578, 1023, 512, 542); //Store y axis
    } else {
      message[1] = map(analogRead(yaxis), 0, 578, 482, 512); //Store y axis
    }
  message[2] = digitalRead(leftButton); //Store button status
  message[3] = digitalRead(rightButton); //Store another button
  
  //Input Smoothing to help filter ramdom input jumps/spikes
  inputSmoothing();

  //Debugging output (Commented out unless needed)
  //if (digitalRead(downButton) < 1) Serial.print("DOWN");
  //if (digitalRead(upButton) < 1) Serial.print("UP");
  //if (digitalRead(leftButton) < 1) Serial.print("LEFT");
  //if (digitalRead(rightButton) < 1) Serial.print("RIGHT");
  //Serial.println(message[0]);
  //Serial.println(message[1]);
  //Serial.println(analogRead(xaxis));
  //Serial.println(analogRead(yaxis));

  //Transmit data to car
  radioSerial.print("<Hello," + String(message[0]) + "," + String(message[1]) + ">");

  //Delay for smoothing
  delay(1);

}


//Input smoothing functoin. 
void inputSmoothing(){
  //Store last several readings and average them to smooth out random input spikes
  total = total - readings[readIndex];
  readings[readIndex] = message[0];
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  message[0] = total / numReadings;
}
