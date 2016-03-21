/*
@ Code for interfacing Alcohol Gas Sensor MQ-3 with Arduino
@ Code by Daniel Spillere Andrade and Daniel Amato Zabotti
@ daniel@danielandrade.net / danielzabotti@gmail.com
@     www.DanielAndrade.net
*/
#include <Arduino.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

unsigned long minCheckDuration = 3000;
bool waitForGoingDown = false;
bool wasButtonPressed = false;
unsigned int minValue = 1024;
unsigned int startTime = 0;
unsigned int loaderFrame = 0;
unsigned int arrowFrame = 0;
unsigned int textScrollIndex = 0;
unsigned long rotateDuration = 250;//Motor rotation duration

const int motorPin = 3;
const int buttonPin = 4;
const int sensorPin = A0;

static const uint8_t PROGMEM
  wait_bmp1[] = { 0x3c,0x1e,0x07,0x03,0x03,0x00,0x00,0x00 },
  wait_bmp2[] = { 0x0c,0x02,0x01,0x03,0x03,0x03,0x06,0x00 },
  wait_bmp3[] = { 0x0c,0x02,0x01,0x01,0x03,0x07,0x0e,0x0c },
  wait_bmp4[] = { 0x00,0x02,0x01,0x01,0x01,0x03,0x3e,0x3c },
  wait_bmp5[] = { 0x00,0x00,0x00,0x00,0x00,0xc1,0x7a,0x3c },
  wait_bmp6[] = { 0x00,0x00,0x00,0xc0,0xc0,0xc0,0x60,0x3c },
  wait_bmp7[] = { 0x00,0x60,0xc0,0xc0,0xc0,0x80,0x40,0x30 },
  wait_bmp8[] = { 0x38,0x78,0xe0,0xc0,0x80,0x80,0x00,0x00 },
  wait_bmp9[] = { 0x3c,0x7e,0xc6,0x80,0x00,0x00,0x00,0x00 },
  wait_bmp10[] = { 0x3c,0x5e,0x87,0x00,0x00,0x00,0x00,0x00 },
  
  arrow_bmp1[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
  arrow_bmp2[] = { 0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
  arrow_bmp3[] = { 0x3c,0x18,0x00,0x00,0x00,0x00,0x00,0x00 },
  arrow_bmp4[] = { 0x7e,0x3c,0x18,0x00,0x00,0x00,0x00,0x00 },
  arrow_bmp5[] = { 0x18,0x7e,0x3c,0x18,0x00,0x00,0x00,0x00 },
  arrow_bmp6[] = { 0x18,0x18,0x7e,0x3c,0x18,0x00,0x00,0x00 },
  arrow_bmp7[] = { 0x18,0x18,0x18,0x7e,0x3c,0x18,0x00,0x00 },
  arrow_bmp8[] = { 0x18,0x18,0x18,0x18,0x7e,0x3c,0x18,0x00 },
  arrow_bmp9[] = { 0x00,0x18,0x18,0x18,0x18,0x7e,0x3c,0x18 },
  arrow_bmp10[] = { 0x5a,0x00,0x18,0x18,0x18,0x18,0x7e,0x3c },
  arrow_bmp11[] = { 0x52,0x5a,0x00,0x18,0x18,0x18,0x18,0x7e },
  arrow_bmp12[] = { 0x52,0x52,0x5a,0x00,0x18,0x18,0x18,0x18 },
  arrow_bmp13[] = { 0x5a,0x52,0x52,0x5a,0x00,0x18,0x18,0x18 },
  arrow_bmp14[] = { 0x00,0x5a,0x52,0x52,0x5a,0x00,0x18,0x18 },
  arrow_bmp15[] = { 0x00,0x00,0x5a,0x52,0x52,0x5a,0x00,0x18 },
  arrow_bmp16[] = { 0x00,0x00,0x5a,0x52,0x52,0x5a,0x00,0x00 };

void setup() {
  matrix.begin(0x70);  // pass in the address
  matrix.clear();
  
  Serial.begin(9600);
  pinMode(motorPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  
  delay(500);
  startTime = millis();
  matrix.setRotation(3);
}

void loop() {
  int sensorReading = analogRead(sensorPin);
  //Serial.println(sensorReading);
  unsigned long diff = millis() - startTime;
  
  if(digitalRead(buttonPin) == 0) {
      analogWrite(motorPin, 200);
      wasButtonPressed = true;
  }else if( wasButtonPressed ) {
      analogWrite(motorPin, 0);
  }
  
  matrix.clear();
  
  int j;
  if(diff < minCheckDuration) {
    //Show "loading" animation
    matrix.setRotation(3);
    loaderFrame = (loaderFrame + 1)%10;
    
    if(loaderFrame == 0) matrix.drawBitmap(0, 0, wait_bmp1, 8, 8, LED_ON);
    if(loaderFrame == 1) matrix.drawBitmap(0, 0, wait_bmp2, 8, 8, LED_ON);
    if(loaderFrame == 2) matrix.drawBitmap(0, 0, wait_bmp3, 8, 8, LED_ON);
    if(loaderFrame == 3) matrix.drawBitmap(0, 0, wait_bmp4, 8, 8, LED_ON);
    if(loaderFrame == 4) matrix.drawBitmap(0, 0, wait_bmp5, 8, 8, LED_ON);
    if(loaderFrame == 5) matrix.drawBitmap(0, 0, wait_bmp6, 8, 8, LED_ON);
    if(loaderFrame == 6) matrix.drawBitmap(0, 0, wait_bmp7, 8, 8, LED_ON);
    if(loaderFrame == 7) matrix.drawBitmap(0, 0, wait_bmp8, 8, 8, LED_ON);
    if(loaderFrame == 8) matrix.drawBitmap(0, 0, wait_bmp9, 8, 8, LED_ON);
    if(loaderFrame == 9) matrix.drawBitmap(0, 0, wait_bmp10, 8, 8, LED_ON);
    
    delay(45);
    
  }else{
    matrix.setRotation(3);
    minValue = min(minValue, sensorReading);
    
    int maxV = 980;
    float value = pow(sensorReading-minValue,2);
    float maxValue = pow(maxV-minValue,2);
    float percent = value/maxValue;
    //int leds = map(round(percent*100), 0,100,0,8*8);
    /*for(int i=0; i<leds; i++) {
      matrix.drawPixel(floor(i/8), 7 - i%8, LED_ON);  
    }*/
    //Spiral(leds);
    
    int perten = round(percent*10);
    if(percent < 0.04) {
      if(textScrollIndex < 180) {
        matrix.setTextSize(1);
        matrix.setTextWrap(false);
        matrix.setTextColor(LED_ON);
        matrix.setCursor(-textScrollIndex,0);
        matrix.print( "                       SOUFFLE  " );
        
      }else{
        
        if(arrowFrame == 0) matrix.drawBitmap(0, 0, arrow_bmp1, 8, 8, LED_ON);
        if(arrowFrame == 1) matrix.drawBitmap(0, 0, arrow_bmp2, 8, 8, LED_ON);
        if(arrowFrame == 2) matrix.drawBitmap(0, 0, arrow_bmp3, 8, 8, LED_ON);
        if(arrowFrame == 3) matrix.drawBitmap(0, 0, arrow_bmp4, 8, 8, LED_ON);
        if(arrowFrame == 4) matrix.drawBitmap(0, 0, arrow_bmp5, 8, 8, LED_ON);
        if(arrowFrame == 5) matrix.drawBitmap(0, 0, arrow_bmp6, 8, 8, LED_ON);
        if(arrowFrame == 6) matrix.drawBitmap(0, 0, arrow_bmp7, 8, 8, LED_ON);
        if(arrowFrame == 7) matrix.drawBitmap(0, 0, arrow_bmp8, 8, 8, LED_ON);
        if(arrowFrame == 8) matrix.drawBitmap(0, 0, arrow_bmp9, 8, 8, LED_ON);
        if(arrowFrame == 9) matrix.drawBitmap(0, 0, arrow_bmp10, 8, 8, LED_ON);
        if(arrowFrame == 10) matrix.drawBitmap(0, 0, arrow_bmp11, 8, 8, LED_ON);
        if(arrowFrame == 11) matrix.drawBitmap(0, 0, arrow_bmp12, 8, 8, LED_ON);
        if(arrowFrame == 12) matrix.drawBitmap(0, 0, arrow_bmp13, 8, 8, LED_ON);
        if(arrowFrame == 13) matrix.drawBitmap(0, 0, arrow_bmp14, 8, 8, LED_ON);
        if(arrowFrame == 14) matrix.drawBitmap(0, 0, arrow_bmp15, 8, 8, LED_ON);
        if(arrowFrame >= 15) matrix.drawBitmap(0, 0, arrow_bmp16, 8, 8, LED_ON);
        
        arrowFrame ++;
        if(arrowFrame == 35) {
          arrowFrame = 0;  
          textScrollIndex = 0;
        }
      }
      textScrollIndex ++;
    }else{
      matrix.setCursor(3,0);
      matrix.print( String(perten) );
      for(int i=0; i<max(1,floor(percent*8)); i++) {
        matrix.drawPixel(0, 7-i, LED_ON);  
        matrix.drawPixel(1, 7-i, LED_ON);  
      }
    }
    
    if(perten < 7) {
      waitForGoingDown = false;
    
    //Throw bills for 0.5 second
    }else if(perten >= 10 && !waitForGoingDown) {
      analogWrite(motorPin, 200);
      
      unsigned int delayStart = millis();
      bool stopped = false;
      
      //Pick a random message
      String message = "";
      int rand = floor(random(0,12));
      if(rand == 0) message = "EPIIIIIC";
      if(rand == 1) message = "BRAVOOOO";
      if(rand == 2) message = "T'ES KEN M'SIEUR";
      if(rand == 3) message = "VA VOMIR";
      if(rand == 4) message = "EPIC WIN";
      if(rand == 5) message = "T'ES MORT";
      if(rand == 6) message = "RENTRE CHEZ TOI";
      if(rand == 7) message = "ARRETE DE BOIRE";
      if(rand == 8) message = "MEC...";
      if(rand == 9) message = "GRAND DIEU...";
      if(rand == 10) message = "ARRETE DE BOIRE";
      if(rand == 11) message = "VIEILLE POCHE";
      message = "       "+message+"  ";
      
      matrix.setTextSize(1);
      matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
      matrix.setTextColor(LED_ON);
      
      int frameDelay = 5;
      int len = (message.length() * 6 * frameDelay);
      for (int x=0; x>=-len; x--) {
        //Scroll message
        matrix.clear();
        matrix.setCursor(round(x/frameDelay),0);
        matrix.print( message );
        matrix.writeDisplay();
        
        //Stop the motor during the animation
        if((millis() - delayStart) > rotateDuration && stopped == false) {
          analogWrite(motorPin, 0);
          stopped = true;
        }
        delay(5);
        
        //Reset "blow here" animation
        arrowFrame = 0;  
        textScrollIndex = 0;
      }
      
      waitForGoingDown = true;//Tells that the gas sensor must go down before starting the motor again
    }
    
    delay(30);
  }
  matrix.writeDisplay();
}

//[EDIT] Not used anymore
//From http://stackoverflow.com/a/1555236
void Spiral( int N ) {
    int X = 8, Y = 8;
    int x,y,dx,dy;
    x = y = dx = 0;
    dy = -1;
    int t = max(X,Y);
    int maxI = t*t;
    for(int i =0; i < N; i++){
        if ((-X/2 <= x) && (x <= X/2) && (-Y/2 <= y) && (y <= Y/2)){
            matrix.drawPixel(x+3, y+3, LED_ON);
        }
        if( (x == y) || ((x < 0) && (x == -y)) || ((x > 0) && (x == 1-y))){
            t = dx;
            dx = -dy;
            dy = t;
        }
        x += dx;
        y += dy;
    }
}
