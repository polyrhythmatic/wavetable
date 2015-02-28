#include <PID_v1.h>
#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <Encoder.h>
#include <EEPROM.h>


Adafruit_MPR121 cap = Adafruit_MPR121();

float offset = 0;
int PWMPins[] = {
  44,13,3,12,4,11,5,10,6,9,7,8};
int analogPins[] = {
  11, 0, 10, 1, 9, 2, 8, 3, 7, 4, 6, 5};
int enablePins[] = {
  53, 31, 51, 33, 49, 35, 47, 37, 45, 39, 43, 41};

int rotaryEncoderA1 = 2;
int rotaryEncoderA2 = 18;
int rotaryEncoderA3 = 19;

int rotaryEncoderB1 = 46;
int rotaryEncoderB2 = 50;
int rotaryEncoderB3 = A15;

int rotaryLatch1 = 48;
int rotaryLatch2 = 52;
int rotaryLatch3 = A14;

int clockPin = 16;
int dataPin = 17;

int shiftLatch = 15;

int buttonList[] = {
  22, 28, 26, 24, 30, 36, 34, 32};

int redLED = 38;
int greenLED = 40;
int blueLED = 42;

Encoder encoder1(rotaryEncoderB1, rotaryEncoderA1);
Encoder encoder2(rotaryEncoderB2, rotaryEncoderA2);
Encoder encoder3(rotaryEncoderB3, rotaryEncoderA3);

boolean faderTouched[12];
boolean inTransition = false;

byte LEDdataArray[] = {
  0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F, 0xFF};
//dataArray[8] = 0xFF; //11111111
//dataArray[7] = 0x7F; //01111111
//dataArray[6] = 0xBF; //10111111
//dataArray[5] = 0xDF; //11011111
//dataArray[4] = 0xEF; //11101111
//dataArray[3] = 0xF7; //11110111
//dataArray[2] = 0xFB; //11111011
//dataArray[1] = 0xFD; //11111101
//dataArray[0] = 0xFE; //11111110

String sendValues;
String buttonPressedVal;
String potPosVal;
String waveformEncoderVal;
String resolutionEncoderVal;
String transitionSpeedVal;
String setButtonVal;

char incomingByte;

int sliderVals[12];
float sliderTarget[12];

int target[] = {
  750,933,1020,991,852,635,390,173,34,4,90,273};

int transitionTarget[15];

float encoder1Target;
float encoder2Target;


//int userSet[15][7];

int userSet[7][15] = {
  {
    649,725,825,812,850,950,5,323,907,525,442,1020,92,44,36    }
  ,{
    791,976,550,2,102,323,576,731,902,525,442,1019,32,96    }
  ,{
    3,2,550,2,1016,808,1021,1020,1021,248,3,1020,64,57    }
  ,{
    280,171,1021,1021,2,401,13,179,1019,996,466,1020,28,28    }
  ,{
    280,171,1021,1021,2,401,13,178,1019,996,466,1020,40,60    }
  ,{
    1021,934,825,859,26,205,106,139,1021,769,412,1020,80,83    }
  ,{
    1021,1021,1012,874,679,50,106,138,1021,768,412,1020,80,76    }
};


void setup() {
  cap.begin(0x5A);// start the capsense communication

  for(int i = 0; i < 12; i++){//set pinmodes for motor outputs
    pinMode(enablePins[i], OUTPUT);
    pinMode(PWMPins[i], OUTPUT);
    pinMode(analogPins[i], OUTPUT);
    sliderTarget[i] = 512; //initialize silder target array
  }

  for(int i = 0; i < 8; i++){ //setting pinmodes for buttons
    pinMode(buttonList[i], INPUT);
    digitalWrite(buttonList[i], LOW);
  }

  pinMode(redLED, OUTPUT);//button RGB LED pin mode
  pinMode(blueLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  digitalWrite(redLED, HIGH);

  pinMode(shiftLatch, OUTPUT);
  digitalWrite(shiftLatch, HIGH);

  digitalWrite(shiftLatch, LOW); //writing the shift register low 
  digitalWrite(clockPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, 0xFF); //clearing out values
  digitalWrite(shiftLatch, HIGH);

  pinMode(rotaryLatch1, OUTPUT); //rotary latch output
  pinMode(rotaryLatch2, OUTPUT);
  pinMode(rotaryLatch3, OUTPUT);
  digitalWrite(rotaryLatch1, LOW);
  digitalWrite(rotaryLatch2, LOW);
  digitalWrite(rotaryLatch3, LOW);

  pinMode(dataPin, OUTPUT); //setting data and clock pin modes and state
  pinMode(clockPin, OUTPUT);
  digitalWrite(dataPin, LOW);
  digitalWrite(clockPin, LOW);

  //turn all LEDs on the circular bar-graph OFF
  //digitalWrite(latchPin, LOW);  // first send latch low
  shiftOut16(0x0000);
  digitalWrite(rotaryLatch1, HIGH);  // send latch high to indicate data is done sending
  digitalWrite(rotaryLatch2, HIGH);
  digitalWrite(rotaryLatch3, HIGH);

  sendValues = String();
  buttonPressedVal = String();
  setButtonVal = String();
  potPosVal = String();
  waveformEncoderVal = String();
  resolutionEncoderVal = String();
  transitionSpeedVal = String();

  for(int i = 0; i < 12; i++){
    transitionTarget[i] = 512;
  }
  encoder3.write(24);

  Serial.begin(115200);

  for(int i = 1; i <100; i++){//Startup sequence
    startupSeq();
  }
}

void loop() {
  //  
  //  for(int i = 0; i < 7; i++){
  //    for(int j = 0; j <15; j++){
  //    Serial.print(userSet[j][i]);
  //    Serial.print(',');
  //    }
  //    Serial.println();
  //  }


  updateCapSense();

  for(int i = 0; i < 12; i++){
    motorControl(PWMPins[i], analogPins[i], enablePins[i], sliderTarget[i], faderTouched[i], i);
  } 

  waveformEncoder();
  bitDepthEncoder();
  transitionEncoder();
  buttonPress();
  transitionSpeed();
  // startupSeq();
  if (Serial.available() > 0){
    // Serial.write(A);

    incomingByte = Serial.read();
    if (incomingByte == '1'){
      serialOut();
    } 
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//SERIAL OUT ROUTINE
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
void serialOut(){
  sendValues = "";
  potPosVal = "";
  for(int i = 0; i < 12; i++){
    potPosVal += String(sliderVals[i])+",";
  }

  sendValues = setButtonVal + "," + buttonPressedVal+ "," + potPosVal + waveformEncoderVal + ","
    + resolutionEncoderVal + "," + transitionSpeedVal;

  Serial.print(sendValues);
  Serial.print("\n");
}

int maxSpeed = 250;
int minSpeed = 120;
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//TRANSITION SPEED
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

void transitionSpeed(){
  
  boolean sliderOnTarget = false;
  float transitionFraction = encoder3.read() / 420.0;
  //Serial.println(transitionFraction);

  for(int i = 0; i < 12; i++){
    sliderTarget[i] += (transitionTarget[i]-sliderTarget[i])*transitionFraction;
//    Serial.print(i);
//    Serial.print(" slider = ");
//    Serial.println(sliderTarget[i] - transitionTarget[i]);
  }

  int encoder1Difference =  transitionTarget[12] - encoder1.read();
  int encoder2Difference =  transitionTarget[13] - encoder2.read();
  
  float encoder1Increment = (encoder1Difference*transitionFraction);
  float encoder2Increment = (encoder2Difference*transitionFraction);

  encoder1Target += encoder1Increment;
  encoder2Target += encoder2Increment;

  if(inTransition){
    encoder1.write(encoder1Target);
    encoder2.write(encoder2Target);
    // encoder3.write(sliderTarget[14] + transitionFraction *transitionTarget[14]);
  }
  if(abs(encoder1Difference) == 0 && abs(encoder2Difference) == 0){
    inTransition = false;
  }

//  Serial.print("   encoder1diff:   ");
//  Serial.print(encoder1Difference);
//  Serial.print("   encoder2diff:   ");
//  Serial.println(encoder2Difference);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//MOTOR CONTROL
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

void motorControl(int PWMPin, int analogPin, int enablePin, int target, int faderTouched, int iCount){
  // read the input on analog pin 0:
  if(faderTouched == true){
    int potValue = analogRead(analogPin);
    sliderVals[iCount] = potValue;
    transitionTarget[iCount] = potValue;
    sliderTarget[iCount] = potValue;
    digitalWrite(PWMPin, LOW);
    return;
  }
  int potValue = analogRead(analogPin);
  sliderVals[iCount] = potValue;
  int proximity = abs(potValue - target);
  int error = 12;

  if (proximity < error){
    digitalWrite(PWMPin, LOW);
    return;
  }
  int potSpeed = constrain(map(proximity, 0, 1024, minSpeed, maxSpeed), minSpeed, maxSpeed);

  if (target > potValue && proximity > error) {
    analogWrite(PWMPin, potSpeed);
    digitalWrite(enablePin, HIGH);   // set leg 1 of the H-bridge high
  }
  // if the switch is low, motor will turn in the other direction:
  if (target < potValue && proximity > error) {
    analogWrite(PWMPin, potSpeed);
    digitalWrite(enablePin, LOW);  // set leg 1 of the H-bridge low

  }

}
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//CAPSENSE FUNCTION
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t currtouched = 0;
uint16_t lasttouched = 0;

void updateCapSense(){
  currtouched = cap.touched();

  int x = 0;
  for (uint8_t i=0; i<12; i++) { //iterate through bit mask from 1 to 100000000000 bitshifting by 1
    if (currtouched & _BV(i)){
      faderTouched[x] = true;
    }
    else{ // resolves to false
      faderTouched[x] = false;
    }
    x++;
  }
  lasttouched = currtouched;
}

void shiftOut16(uint16_t data){
  byte datamsb;
  byte datalsb;

  // Isolate the MSB and LSB
  datamsb = (data&0xFF00)>>8;  // mask out the MSB and shift it right 8 bits
  datalsb = data & 0xFF;  // Mask out the LSB



  // First shift out the MSB, MSB first.
  shiftOut(dataPin, clockPin, MSBFIRST, datamsb);
  // Then shift out the LSB
  shiftOut(dataPin, clockPin, MSBFIRST, datalsb);
}
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//WAVEFORM ENCODER AND LED RING. uses ledfollow code from example.
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

long newEncoder1 = 0;

void waveformEncoder(){

  boolean encoderIncreasing;
  boolean encoderDecreasing;

  if(newEncoder1 < encoder1.read()){
    encoderIncreasing = true;
  }
  else{
    encoderIncreasing = false;
  }
  if(newEncoder1 > encoder1.read()){
    encoderDecreasing = true;
  }
  else{
    encoderDecreasing = false;
  }

  newEncoder1 = encoder1.read();

  if (newEncoder1 < 0x00 && encoderDecreasing == true){
    encoder1.write(0x60 + newEncoder1);
  }
  if(newEncoder1 >= 0x60 && encoderIncreasing == true){
    encoder1.write((0x00 + newEncoder1 - 0x60));
  }
  if(encoder1.read() == 0x60){
    newEncoder1 = 0x00;
  }
  unsigned int ledShift = 0;
  // each bit of ledOutput represents a single LED on the ring
  // this should be a value between 0 and 0xFFFF (16 bits for 16 LEDs)
  unsigned int ledOutput = 0;

  // Only do this if encoderPosition = 0, if it is 0, we don't
  // want any LEDs lit up
  //if (newEncoder != 0)
  // {
  ledShift = map(newEncoder1, 0, 96, 0, 16);
  // Now just use ledShift to calculate ledOutput.
  // ledOutput will only have 1 bit set
  ledOutput = 1 << ledShift;
  // }
  digitalWrite(rotaryLatch1, LOW);  // first send latch low
  shiftOut16(ledOutput);  // send the ledOutput value to shiftOut16
  digitalWrite(rotaryLatch1, HIGH);  // send latch high to indicate data is done sending 

  waveformEncoderVal = String(newEncoder1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//BIT DEPTH ROTARY KNOB
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
long newEncoder2 = 0;

void bitDepthEncoder(){

  if (encoder2.read() <=0){
    encoder2.write(0);
  }
  if(encoder2.read() >= 0x60){
    encoder2.write(0x60);
  }
  newEncoder2 = encoder2.read();

  // ledShift stores the bit position of the upper-most LED
  // this value should be between 0 and 15 (shifting a 16-bit vaule)
  unsigned int ledShift = 0;
  // each bit of ledOutput represents a single LED on the ring
  // this should be a value between 0 and 0xFFFF (16 bits for 16 LEDs)
  unsigned int ledOutput = 0;

  // Only do this if encoderPosition = 0, if it is 0, we don't
  // want any LEDs lit up
  if (newEncoder2 != 0){
    ledShift =map(newEncoder2, 0, 96, 0, 16);
    //Serial.println(newEncoder);

    // This for loop sets each bet that is less signfigant than
    // ledShift. This is what sets ledBarFiller apart from
    // ledBarFollower()
    for (int i=ledShift; i>=0; i--)
      ledOutput |= 1<<i;
  }

  // Now we just need to write to the shift registers. We have to
  // control latch manually, but shiftOut16 will take care of
  // everything else.
  digitalWrite(rotaryLatch2, LOW);  // first send latch low
  shiftOut16(ledOutput);  // send the ledOutput value to shiftOut16
  digitalWrite(rotaryLatch2, HIGH);  // send latch high to indicate data is done sending 

  resolutionEncoderVal = String(newEncoder2);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//TRANSITION ROTARY KNOB
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
long newEncoder3;

void transitionEncoder(){

  if (encoder3.read() <=0){
    encoder3.write(0);
  }
  if(encoder3.read() >= 0x60){
    encoder3.write(0x60);
  }
  newEncoder3 = encoder3.read();

  // ledShift stores the bit position of the upper-most LED
  // this value should be between 0 and 15 (shifting a 16-bit vaule)
  unsigned int ledShift = 0;
  // each bit of ledOutput represents a single LED on the ring
  // this should be a value between 0 and 0xFFFF (16 bits for 16 LEDs)
  unsigned int ledOutput = 0;

  // Only do this if encoderPosition = 0, if it is 0, we don't
  // want any LEDs lit up
  if (newEncoder3 != 0){
    ledShift =map(newEncoder3, 0, 96, 0, 16);
    //Serial.println(newEncoder);

    // This for loop sets each bet that is less signfigant than
    // ledShift. This is what sets ledBarFiller apart from
    // ledBarFollower()
    for (int i=ledShift; i>=0; i--)
      ledOutput |= 1<<i;
  }

  // Now we just need to write to the shift registers. We have to
  // control latch manually, but shiftOut16 will take care of
  // everything else.
  digitalWrite(rotaryLatch3, LOW);  // first send latch low
  shiftOut16(ledOutput);  // send the ledOutput value to shiftOut16
  digitalWrite(rotaryLatch3, HIGH);  // send latch high to indicate data is done sending 

  transitionSpeedVal = String(newEncoder3);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//BUTTON PRESS ROUTINE
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

void buttonPress(){
  byte ledTotal = 0xFF;
  boolean buttonIsPressed = false;

  if(digitalRead(buttonList[0]) == 1){
    setButtonVal = String(1);
    ledTotal = LEDdataArray[0] & ledTotal;

    for(int i = 1; i < 8; i++){
      if (digitalRead(buttonList[i]) == 1){
        for(int j = 0; j < 12; j++){
          userSet[i-1][j] = sliderVals[j];
        }
        userSet[i-1][12] = encoder1.read();
        userSet[i-1][13] = encoder2.read();
        userSet[i-1][14] = encoder3.read();

      }

    }
  } 
  else{
    setButtonVal = String(0);
  }

  for(int i = 1; i < 8; i++){
    int buttonState = digitalRead(buttonList[i]);
    if (buttonState == 1){

      ledTotal = LEDdataArray[i] & ledTotal;
      buttonIsPressed = true;
      buttonPressedVal = String(i);
    }
    if (buttonState == 0 && buttonIsPressed == false){
      buttonPressedVal = String(0);
    }
  }
  digitalWrite(clockPin, LOW);
  digitalWrite(shiftLatch, LOW);
  //move 'em out
  shiftOut(dataPin, clockPin, MSBFIRST, ledTotal);
  //return the latch pin high to signal chip that it 
  //no longer needs to listen for information
  digitalWrite(shiftLatch, HIGH);


  if(digitalRead(buttonList[0]) == 0){
    for(int i = 1; i < 8; i++){
      if (digitalRead(buttonList[i]) == 1){
        for(int j = 0; j < 15; j++){
          inTransition = true;

          transitionTarget[j] = userSet[i-1][j];
        }
      }
    }
  } 
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//STARTUP SEQUENCE
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

long unsigned int lastTime = 0;

int targetWave[] = {
  750,933,1020,991,852,635,390,173,34,4,90,273};

void startupSeq(){
  updateCapSense();

  for(int i = 0; i < 12; i++){
    motorControl(PWMPins[i], analogPins[i], enablePins[i], targetWave[i], faderTouched[i], i);

  }  
  if(millis() - lastTime >= 150){
    int tempTargetWave = targetWave[11];
    for(int i = 10; i >=0; i--){
      targetWave[i+1] = targetWave[i];
    }
    targetWave[0] = tempTargetWave;
    lastTime = millis();
  }
}






























































