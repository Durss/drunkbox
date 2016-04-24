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


//==============================================
//============CUSTOMIZABLE VARIABLES============
//==============================================
  const unsigned long rotateDuration = 500;//Motor rotation duration when drunk enough
  const int motorPin = 3;//Pin that controls the motor
  const int buttonPin = 4;//Pin where the button to manually send bills is plugged
  const int sensorPin = A0;//Pin where the gas sensor is plugged
  
  //The gas sensor should normally heat ~15min before being used.
  //But sometimes we want to power off/on the box without having to wait
  //15min again as the sensor is already hot.
  //If you want to force these 15min of heating, just set the following
  //value to 900000 (milliseconds).
  const unsigned long minCheckDuration = 3000;//900000;
  //Message displayed to tell the user to blow.
  //The full message says "Blow here =>" but the "here =>" part is
  //actually an animation.i made the frames with this tool of mine :
  //https://dl.dropboxusercontent.com/u/20758492/arduino/thermalPrinterCharsGen/index.html
  //But frames are basically binary data that say which pixel set on.
  //I set white spaces before the "Blow"to simulate a delay between
  //two animations.
  const String msg_blowHere = "                       SOUFFLE  ";
//=====================================================
//============END OF CUSTOMIZABLE VARIABLES============
//=====================================================



//=========================================================
//============SOME VARS YOU DON'T WANT TO TOUCH============
//=========================================================
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();
bool waitForGoingDown = false;
bool wasButtonPressed = false;
unsigned int minValue = 1024;
unsigned int startTime = 0;
unsigned int loaderFrame = 0;
unsigned int arrowFrame = 0;
unsigned int textScrollIndex = 0;



//===============================================
//============GRAPHICS FOR LED MATRIX============
//===============================================
static const uint8_t PROGMEM
  //Loading frames
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
  
  //"here =>" frames of the "Blow here =>" message
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
  arrow_bmp16[] = { 0x00,0x00,0x5a,0x52,0x52,0x5a,0x00,0x00 },
  
  //Win animation (sort of rotating rays)
  win_bmp1[] = { 0x76,0xb5,0xd3,0x07,0xe0,0xcb,0xad,0x6e },
  win_bmp2[] = { 0xdd,0x6b,0xa6,0xc1,0x83,0x65,0xd6,0xbb },
  win_bmp3[] = { 0xbb,0xd6,0x65,0x83,0xc1,0xa6,0x6b,0xdd },
  
  //0 to 9 animation
  counter_bmp1[] = { 0x18,0x24,0x24,0x24,0x24,0x24,0x24,0x18 },
  counter_bmp2[] = { 0x08,0x38,0x08,0x08,0x08,0x08,0x08,0x3c },
  counter_bmp3[] = { 0x18,0x24,0x24,0x04,0x18,0x20,0xff,0xc3 },
  counter_bmp4[] = { 0x3c,0x04,0x0c,0x18,0x04,0xfb,0xdb,0xe7 },
  counter_bmp5[] = { 0x20,0x20,0x28,0x28,0x3c,0xf7,0xf7,0xff },
  counter_bmp6[] = { 0x3c,0x20,0x20,0x3c,0xfb,0xfb,0xdb,0xe7 },
  counter_bmp7[] = { 0x1c,0x20,0x20,0x38,0xc3,0xdb,0xdb,0xe7 },
  counter_bmp8[] = { 0x3c,0x04,0x04,0xf7,0xf7,0xef,0xdf,0xdf },
  counter_bmp9[] = { 0x18,0x24,0xdb,0xdb,0xe7,0xdb,0xdb,0xe7 },
  counter_bmp10[] = { 0x3c,0xc3,0xdb,0xdb,0xc3,0xfb,0xfb,0xc7 };

void setup() {
  matrix.begin(0x70);
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
  
  //Is button pressed ?
  if(digitalRead(buttonPin) == 0) {
    //Throw bills until it's released
    analogWrite(motorPin, 200);
    wasButtonPressed = true;
    return;
  }else if( wasButtonPressed ) {
    analogWrite(motorPin, 0);
  }
  
  matrix.clear();
  
  int j;
  //Show "loading" animation
  if(diff < minCheckDuration) {
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
    
    //Sensor to 0, just display the "Blow here =>" animation
    if(percent < 0.04) {
      if(textScrollIndex < 180) {
        //"Blow" message
        matrix.setTextSize(1);
        matrix.setTextWrap(false);
        matrix.setTextColor(LED_ON);
        matrix.setCursor(-textScrollIndex,0);
        matrix.print( msg_blowHere );
        
      }else{
        //"here =>" message
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
    
    //User is blowing, display progression
    }else{
      //*
      if(perten == 0) matrix.drawBitmap(0, 0, counter_bmp1, 8, 8, LED_ON);
      if(perten == 1) matrix.drawBitmap(0, 0, counter_bmp2, 8, 8, LED_ON);
      if(perten == 2) matrix.drawBitmap(0, 0, counter_bmp3, 8, 8, LED_ON);
      if(perten == 3) matrix.drawBitmap(0, 0, counter_bmp4, 8, 8, LED_ON);
      if(perten == 4) matrix.drawBitmap(0, 0, counter_bmp5, 8, 8, LED_ON);
      if(perten == 5) matrix.drawBitmap(0, 0, counter_bmp6, 8, 8, LED_ON);
      if(perten == 6) matrix.drawBitmap(0, 0, counter_bmp7, 8, 8, LED_ON);
      if(perten == 7) matrix.drawBitmap(0, 0, counter_bmp8, 8, 8, LED_ON);
      if(perten == 8) matrix.drawBitmap(0, 0, counter_bmp9, 8, 8, LED_ON);
      if(perten == 9) matrix.drawBitmap(0, 0, counter_bmp10, 8, 8, LED_ON);
      //*/
      /*
      matrix.setCursor(3,0);
      matrix.print( String(perten) );
      for(int i=0; i<max(1,floor(percent*8)); i++) {
        matrix.drawPixel(0, 7-i, LED_ON);  
        matrix.drawPixel(1, 7-i, LED_ON);  
      }
      //*/
    }
    
    if(perten < 7) {
      waitForGoingDown = false;
    
    
    //User is drunk, throw bills at him
    }else if(perten >= 10 && !waitForGoingDown) {
      
      matrix.clear();
      matrix.writeDisplay();
      matrix.setTextSize(1);
      matrix.setTextWrap(false);
      matrix.setTextColor(LED_ON);
      
      analogWrite(motorPin, 200);
      delay(rotateDuration);
      analogWrite(motorPin, 0);
      
      //Rotating rays animation
      for(int l = 0; l < 15; l++) {
        matrix.clear();
        matrix.drawBitmap(0, 0, win_bmp1, 8, 8, LED_ON);
        matrix.writeDisplay();
        delay(50);
        matrix.clear();
        matrix.drawBitmap(0, 0, win_bmp2, 8, 8, LED_ON);
        matrix.writeDisplay();
        delay(50);
        matrix.clear();
        matrix.drawBitmap(0, 0, win_bmp3, 8, 8, LED_ON);
        matrix.writeDisplay();
        delay(50);
      }
      
      
      //Pick a random message to display
      String message = "";
      int rand = floor(random(0,12));
      if(rand == 0) message = "EPIIIIIC !";
      if(rand == 1) message = "BRAVOOOO !!!";
      if(rand == 2) message = "T'ES KEN M'SIEUR";
      if(rand == 3) message = "VA VITE TE FAIRE VOMIR";
      if(rand == 4) message = "EPIC WIN";
      if(rand == 5) message = "T'ES PAS EN BON ETAT A PRIORI";
      if(rand == 6) message = "RENTRE CHEZ TOI";
      if(rand == 7) message = "ARRETE DE BOIRE";
      if(rand == 8) message = "APPELEZ VITE LE 18 !";
      if(rand == 9) message = "GRAND DIEU...";
      if(rand == 10) message = "ARRETE DE BOIRE";
      if(rand == 11) message = "VIEILLE POCHE";
      message = "  "+message+"  ";
      
      int frameDelay = 5;
      int len = (message.length() * 6 * frameDelay);
      
      //Display message
      for (int x=0; x>=-len; x--) {
        //Scroll message
        matrix.clear();
        matrix.setCursor(round(x/frameDelay),0);
        matrix.print( message );
        matrix.writeDisplay();
        delay(5);
      }
        
      //Reset "blow here" animation
      arrowFrame = 0;  
      textScrollIndex = 0;
      
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
