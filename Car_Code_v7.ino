/* 
 * ENR 281 Enginnering Design Lab
 * Red Team Car Code
 * Receiver V7.5.2 from 4/12/2021
 */

#include <PWMServo.h> //Include PWMServo library. This library was used because it avoids a conflict with SoftwareSerial
#include<SoftwareSerial.h> //Include SoftwareSerial library to comunicate with HC-12 wireless radio
#include <Adafruit_NeoPixel.h> //Include Neopixel library to control addresable LED strip used in our headlights.

#define PIN 6 //Pin to control LED lights
#define NUMPIXELS 7 //Number of LED lights
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //Setup for LED lights

SoftwareSerial radioSerial(3,4); //Set Software Serial pins
PWMServo rightMotor; //Add right motor
PWMServo leftMotor; //Add left motor

//Serial communications variables
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars]; 
boolean newData = false;
char incommingData[numChars] = {0};

//Variables for joystick inputs
int throttle = 90;
int steering = 90;
long int xpos;
long int ypos;

//Motor speed control pins and variables
int lspeed = 0;
int rspeed = 0;
int turn;
int speed;

//Setup
void setup() {
  Serial.begin(9600); //Open serial for debugging
  rightMotor.attach(9); //Attach right motor
  leftMotor.attach(10); //Attach left motor
  rightMotor.write(90); //Set motor to stopped
  leftMotor.write(90); //Set motor to stopped
  delay(5000); //Wait for motor controller to initialize
  
      while (!Serial){}; //Wait for serial to open
      Serial.println("POWER ON"); //Print power on message
      radioSerial.begin(9600); //Open connection to wireless radio
      pixels.begin(); //Initialize LED strip 

//Set up the lights with a nice little animation
for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    pixels.show();
    delay(100);
  }
for(int i=1; i<6; i++) { 
    pixels.setPixelColor(i, pixels.Color(255, 255, 255));
    pixels.show();
    delay(100);
  }
}

//Main loop
void loop() {
//Receive data from radio
recvWithStartEndMarkers(); 
    if (newData == true) {
        strcpy(tempChars, receivedChars);
        parseData();
        newData = false;
    }
    
//Map incomming data
throttle = map(xpos,0,1023,20,160);
steering = map(ypos,0,1023,0,180);

//Get distance from center with abs()
turn = abs(90-steering);
speed = abs(90-throttle);

//steering limiter (Currently Disabled)
//if (turn > speed){
//  turn = speed;
//}


//Motor speed control
//Mixing throttle and steering to get motor speeds

//Forward
if (throttle > 91 ){
  //Right
  if (steering > 91){
    lspeed=throttle-turn;
    rspeed=throttle+turn;
    //Left
  }else if (steering < 89){
    lspeed=throttle+turn;
    rspeed=throttle-turn;
  }else{
    //No turn
    lspeed=throttle;
    rspeed=throttle;
  }
  //Backwards
}else if (throttle < 89){
  //Right
  if (steering > 92){
    lspeed=throttle+turn;
    rspeed=throttle-turn;
    //Left
  }else if (steering < 88){
    lspeed=throttle-turn;
    rspeed=throttle+turn;
  }else{
    //No turn
    lspeed=throttle;
    rspeed=throttle;
  }
}else{
  //Not moving forward, pivot in place
  if (steering < 90){
    lspeed=90+turn;
    rspeed=90-turn;
  }else{
    lspeed=90-turn;
    rspeed=90+turn;
  }
}


//Send speed values to motor controllers
rightMotor.write(lspeed);
leftMotor.write(rspeed);

//Debugging output (Disabled unless needed)
//Serial.print("L:");
//Serial.print(lspeed);
//Serial.print("R:");
//Serial.println(rspeed);
//Serial.println(steering);

/*
Serial.print("T:");
Serial.println(throttle);
Serial.print("S:");
Serial.println(steegring);
*/
}


//Handle incomming data
void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
    while (radioSerial.available() > 0 && newData == false) {
        rc = radioSerial.read();
        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0';
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }
        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

//Parse data function
void parseData() {      // split the data into parts
    char * strtokIndx; // this is used by strtok() as an index
    strtokIndx = strtok(tempChars,",");      // get the first part - the string
    strcpy(incommingData, strtokIndx); // copy it to incommingData
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    xpos = atoi(strtokIndx);     // convert this part to an integer
    strtokIndx = strtok(NULL, ",");
    ypos = atoi(strtokIndx);     // convert this part to a float
}


//Print data to Serial
void showParsedData() {
    Serial.print("A:");
    Serial.println(throttle);
    Serial.print("B:");
    Serial.println(steering);
}
