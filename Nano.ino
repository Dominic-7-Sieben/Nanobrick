/* Notes:
  Colors on the screen go as follows: BGR, not RGB. So red = blue, and blue = red. Green still equals green.

*/
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include "Arduino_BMI270_BMM150.h"
#include <Arduino_LPS22HB.h>
#include <Arduino_HS300x.h>
#include <Arduino_APDS9960.h>
#include <PDM.h>

#define TFT_CS          8
#define TFT_RST        10 
#define TFT_DC          9

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
int backlightpin = 4;

int tallycount = 0;
bool calibrated = false;
int reading = 0;
int counter = 0;
int buttonPin = 2;
int lastsavedState = HIGH;
int currentState = 0;
int instances = 13; // The amount of menu logic options.
float X, Y, Z;
float XAdjust, YAdjust, ZAdjust;
int R, G, B;
float proxy = 0;
bool sleepState = false;
int State = 0;
int count = 0;
int count2 = 0;
int hour = 0;
int minute = 0;
int day = 0;
int month = 0;
int lastUpdatedMinute = 0;
bool clockisset = false;

unsigned long startTime = 0;
unsigned long singleClickDelay = 50;
unsigned long pressTime = 0;
unsigned long releaseTime = 0;
unsigned long setTime = 0;
unsigned long lastUpdate = 0;

short sampleBuffer[256];
volatile int samplesRead;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  tft.initR(INITR_MINI160x80); 
  tft.setRotation(2);
  tft.setTextWrap(true);
  tft.fillScreen(ST7735_BLACK);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(backlightpin, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  digitalWrite(LEDR, HIGH);         
  digitalWrite(LEDG, HIGH);      
  digitalWrite(LEDB, HIGH);
  IMU.begin();
  BARO.begin();
  HS300x.begin();
  APDS.begin();
  PDM.onReceive(onPDMdata);
  PDM.begin(1, 16000);
  digitalWrite(backlightpin, HIGH);
}
void loop() {
  unsigned long now = millis();
  unsigned long elapsed = now - lastUpdate;

  if (elapsed >= 60000) {
      minute += elapsed / 60000;
      lastUpdate = now;

      // Handle minute rollover
      while (minute >= 60) {
          minute -= 60;
          hour++;

          // Handle hour rollover
          if (hour >= 24) {
              hour = 0;
          }
      }
  }

  if (button() == 0){ // Checks if button is pressed
    pressTime = millis(); // Store when button was pressed
    while (button() == 0){} //This causes it to wait for the button to be unpressed
    releaseTime = millis(); // Once the button is no longer = 0, check what time it is.
    unsigned long duration = releaseTime - pressTime;
      if (duration >= 1500){
        modeManage(3); //Long Hold
      }
      else if (duration >= 500 && duration < 1000){
        modeManage(2); //Short Hold
      }
      else if (duration < 250){
        modeManage(1); //Quick Press
      }
  }
  modeManage(0);
}
int button(){
  int value = digitalRead(buttonPin);
  return value;
}
void modeManage(int manageint){
  if (manageint == 0){
    if (count2 != counter){
      count2 = counter;
      tft.fillScreen(ST7735_BLACK);
      analogWrite(LEDR, 255);
      analogWrite(LEDG, 255);
      analogWrite(LEDB, 255);
  }
  switch (counter) {
    case 0: clock(0); break;
    case 1: tally(0); break;
    case 2: runTemperature(); break;
    case 3: runHumidity(); break;
    case 4: runPressure(); break;
    case 5: runFlashlight(); break;
    case 6: runAccelerometer(0); break;
    case 7: runGyroscope(0); break;
    case 8: runMagnetometer(0); break;
    case 9: runMicrophone(); break;
    case 10: runProxy(); break;
    case 11: runColor(); break;
    case 12: runAltitude(); break;
    case 13: sleep(); break;
    default:
      counter = 0;
      sleepState = false;
      calibrated = false;
      tft.fillScreen(ST7735_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      digitalWrite(backlightpin, HIGH);
    break;
  }
  }
  else if (manageint == 1){
    counter++;
  }
  else if (manageint == 2){
    switch (counter) {
      case 0: clock(1); break;
      case 1: tally(1); break;
      case 2: runTemperature(); break;
      case 3: runHumidity(); break;
      case 4: runPressure(); break;
      case 5: runFlashlight(); break;
      case 6: runAccelerometer(1); break;
      case 7: runGyroscope(1); break;
      case 8: runMagnetometer(1); break;
      case 9: runMicrophone(); break;
      case 10: runProxy(); break;
      case 11: runColor(); break;
      case 12: runAltitude(); break;
      case 13: sleep(); break;
      default:
        counter = 0;
        sleepState = false;
        calibrated = false;
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST77XX_WHITE);
        digitalWrite(backlightpin, HIGH);
      break;
    }
  }
  else if (manageint == 3){
    switch (counter) {
      case 0: clock(2); break;
      case 1: tally(2); break;
      case 2: runTemperature(); break;
      case 3: runHumidity(); break;
      case 4: runPressure(); break;
      case 5: runFlashlight(); break;
      case 6: runAccelerometer(0); break;
      case 7: runGyroscope(0); break;
      case 8: runMagnetometer(0); break;
      case 9: runMicrophone(); break;
      case 10: runProxy(); break;
      case 11: runColor(); break;
      case 12: runAltitude(); break;
      case 13: sleep(); break;
      default:
        counter = 0;
        sleepState = false;
        calibrated = false;
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST77XX_WHITE);
        digitalWrite(backlightpin, HIGH);
      break;
    }
  }
}
void runAccelerometer(int check){
  if (check == 1){
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Calibrate...");
    tft.setCursor(0, 10);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Please Hold");
    tft.setCursor(0, 20);
    tft.print("Still!");
    delay(2000);
    IMU.readAcceleration(X, Y, Z);
    XAdjust = X;
    YAdjust = Y;
    ZAdjust = Z;
    tft.fillScreen(ST7735_BLACK);
    calibrated = true;
  }
  if (calibrated == false){
  IMU.readAcceleration(X, Y, Z);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Acceleration");
  tft.setCursor(0, 50);
  tft.fillRect(15, 50, 35, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("X: ");
  tft.println(X);
  analogWrite(LEDR, 255 - axisToPWMwithconstraint(X));
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Y: ");
  tft.println(Y);
  analogWrite(LEDG, 255 - axisToPWMwithconstraint(Y));
  tft.setTextColor(ST77XX_RED);
  tft.print("Z: ");
  tft.println(Z);
  analogWrite(LEDB, 255 - axisToPWMwithconstraint(Z));
  }
  if (calibrated == true){
  IMU.readAcceleration(X, Y, Z);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Acceleration");
  tft.setCursor(0, 50);
  tft.fillRect(15, 50, 35, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("X: ");
  tft.println(X - XAdjust);
  analogWrite(LEDR, 255 - axisToPWMwithconstraint(X - XAdjust));
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Y: ");
  tft.println(Y - YAdjust);
  analogWrite(LEDG, 255 - axisToPWMwithconstraint(Y - YAdjust));
  tft.setTextColor(ST77XX_RED);
  tft.print("Z: ");
  tft.println(Z - ZAdjust);
  analogWrite(LEDB, 255 - axisToPWMwithconstraint(Z - ZAdjust));
  }
}
void runGyroscope(int check){
  if (check == 1){
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Calibrate...");
    tft.setCursor(0, 10);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Please Hold");
    tft.setCursor(0, 20);
    tft.print("Still!");
    delay(2000);
    IMU.readGyroscope(X, Y, Z);
    XAdjust = X;
    YAdjust = Y;
    ZAdjust = Z;
    tft.fillScreen(ST7735_BLACK);
    calibrated = true;
  }
  if (calibrated == false){
  IMU.readGyroscope(X, Y, Z);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Gyroscope");
  tft.setCursor(0, 50);
  tft.fillRect(15, 50, 55, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("X: ");
  tft.println(X);
  analogWrite(LEDR, 255 - axisToPWM(X));
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Y: ");
  tft.println(Y);
  analogWrite(LEDG, 255 - axisToPWM(Y));
  tft.setTextColor(ST77XX_RED);
  tft.print("Z: ");
  tft.println(Z);
  analogWrite(LEDB, 255 - axisToPWM(Z));
  }
  if (calibrated == true){
  IMU.readGyroscope(X, Y, Z);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Gyroscope");
  tft.setCursor(0, 50);
  tft.fillRect(15, 50, 55, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("X: ");
  tft.println(X - XAdjust);
  analogWrite(LEDR, 255 - axisToPWM(X - XAdjust));
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Y: ");
  tft.println(Y - YAdjust);
  analogWrite(LEDG, 255 - axisToPWM(Y - YAdjust));
  tft.setTextColor(ST77XX_RED);
  tft.print("Z: ");
  tft.println(Z - ZAdjust);
  analogWrite(LEDB, 255 - axisToPWM(Z - ZAdjust));
  }
}
void runPressure(){
  float kPa = BARO.readPressure();
  float mBar = kPa * 10;
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Air Pressure");
  tft.setCursor(0, 50);
  tft.fillRect(0, 50, 80, 10, ST77XX_BLACK);
  tft.print(mBar);
  tft.println("  mBar");
  analogWrite(LEDR, 255 - fabs(kPa));
}
void runAltitude(){
  float A = BARO.readPressure();
  float altitude = 44330 * ( 1 - pow(A/101.325, 1/5.255) );
  float altitudeFeet = altitude * 3.28084;
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Altitude");
  tft.setCursor(0, 50);
  tft.fillRect(0, 50, 45, 10, ST77XX_BLACK);
  tft.print(altitudeFeet);
  tft.println("  ft");
  analogWrite(LEDG, 255 - fabs(altitudeFeet));
}
void runMagnetometer(int check){
    if (check == 1){
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Calibrate...");
    tft.setCursor(0, 10);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Please Hold");
    tft.setCursor(0, 20);
    tft.print("Still!");
    delay(2000);
    IMU.readMagneticField(X, Y, Z);
    XAdjust = X;
    YAdjust = Y;
    ZAdjust = Z;
    tft.fillScreen(ST7735_BLACK);
    calibrated = true;
  }
  if (calibrated == false){
  IMU.readMagneticField(X, Y, Z);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Magnetometer");
  tft.setCursor(0, 50);
  tft.fillRect(15, 50, 55, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("X: ");
  tft.println(X);
  analogWrite(LEDR, 255 - axisToPWM(X));
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Y: ");
  tft.println(Y);
  analogWrite(LEDG, 255 - axisToPWM(Y));
  tft.setTextColor(ST77XX_RED);
  tft.print("Z: ");
  tft.println(Z);
  analogWrite(LEDB, 255 - axisToPWM(Z));
  }
  if (calibrated == true){
  IMU.readMagneticField(X, Y, Z);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Magnetometer");
  tft.setCursor(0, 50);
  tft.fillRect(15, 50, 55, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("X: ");
  tft.println(X - XAdjust);
  analogWrite(LEDR, 255 - axisToPWM(X - XAdjust));
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Y: ");
  tft.println(Y - YAdjust);
  analogWrite(LEDG, 255 - axisToPWM(Y - YAdjust));
  tft.setTextColor(ST77XX_RED);
  tft.print("Z: ");
  tft.println(Z - ZAdjust);
  analogWrite(LEDB, 255 - axisToPWM(Z - ZAdjust));
  }
}
void runTemperature(){
  float CT = HS300x.readTemperature();
  float FT = (CT * 1.8) + 32;
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Temperature");
  tft.setCursor(0, 50);
  tft.fillRect(0, 50, 45, 30, ST77XX_BLACK);
  tft.print(FT);
  tft.print("  ");
  tft.write(248);
  tft.println("F");
  tft.print(CT);
  tft.print("  ");
  tft.write(248);
  tft.println("C");
}
void runHumidity(){
  float H = HS300x.readHumidity();
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Humidity");
  tft.setCursor(0, 50);
  tft.fillRect(0, 50, 40, 30, ST77XX_BLACK);
  tft.print(H);
  tft.println("  %");
  analogWrite(LEDB, 255 - fabs(H));
}
void runColor(){
  if (APDS.colorAvailable()){
  APDS.readColor(R, G, B);
  }
  analogWrite(LEDR, 255);
  analogWrite(LEDG, 255);
  analogWrite(LEDB, 255);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Light");
  tft.setCursor(0, 50);
  tft.fillRect(15, 50, 45, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("R: ");
  tft.println(R);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("G: ");
  tft.println(G);
  tft.setTextColor(ST77XX_RED);
  tft.print("B: ");
  tft.println(B);
}
void runProxy(){
  if (APDS.proximityAvailable()) {
    proxy = APDS.readProximity();
  }
  analogWrite(LEDR, 255);
  analogWrite(LEDG, 255);
  analogWrite(LEDB, 255);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Proximity");
  tft.setCursor(0, 50);
  tft.fillRect(35, 50, 45, 20, ST77XX_BLACK);
  tft.print("Prox: ");
  tft.println(proxy);
  tft.setCursor(0, 75);
  tft.print("Disclaimer: This sensor relies on reflectance. Results will vary with material.");
}
void runMicrophone() {
  if (samplesRead > 0) {
    long sum = 0;
    for (int i = 0; i < samplesRead; i++) {
      sum += abs(sampleBuffer[i]);
    }
    int volume = sum / samplesRead;   // average amplitude
    int ledValue = map(volume, 0, 2000, 0, 255);
    ledValue = constrain(ledValue, 0, 255);

    analogWrite(LEDR, 255 - ledValue);
    analogWrite(LEDG, 255 - ledValue);
    analogWrite(LEDB, 255 - ledValue);
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.println("Microphone");
    tft.setCursor(0, 50);
    tft.fillRect(25, 50, 45, 20, ST77XX_BLACK);
    tft.print("Mic: ");
    tft.println(volume);

    samplesRead = 0;   // reset so next buffer can be processed
  }
}
void runFlashlight() {
  tft.fillScreen(ST77XX_WHITE);
  analogWrite(LEDR, 0);
  analogWrite(LEDG, 0);
  analogWrite(LEDB, 0);
}
int axisToPWMwithconstraint(float v) {
  v = fabs(v);               // absolute acceleration
  v = constrain(v, 0, 2);    // typical range when tilting
  return map(v * 100, 0, 200, 0, 255);
}
int axisToPWM(float v) {
  v = fabs(v);               // absolute rotation speed
  v = constrain(v, 0, 500);  // typical usable gyro range
  return map(v, 0, 500, 0, 255);
}
void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();

  // read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}
void sleep(){
  if (sleepState == false){
  digitalWrite(backlightpin, LOW);
  tft.fillScreen(ST7735_BLACK);
  analogWrite(LEDR, 255);
  analogWrite(LEDG, 255);
  analogWrite(LEDB, 255);
  for (int i = 0; i <= 10; i++){
    analogWrite(LEDG, 0);
    delay(500);
    analogWrite(LEDG, 255);
    delay(500);
  }
  Serial.println("Sleeping...");
  sleepState = true;
  }
}
void tally(int tally){
  if (tally == 2){
    tft.fillScreen(ST77XX_BLACK);
    tallycount = 0;
  }
  if (tally == 1){
    tft.fillScreen(ST77XX_BLACK);
    tallycount++;
  }
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Tally Counter");
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(0, 50);
  tft.setTextSize(3);
  tft.print(tallycount);
}
void clock(int mode){
  if (lastUpdatedMinute != minute){
    lastUpdatedMinute = minute;
    tft.fillScreen(ST77XX_BLACK);
  }
  if (mode == 2){
    setclock();
  }
  if (mode == 0){
  tft.setCursor(0, 25);
  tft.setTextSize(5);
  tft.print(hour);
  tft.setCursor(0, 80);
  tft.setTextSize(5);
  tft.print(minute);
  }
}
void setclock(){
  int mode = 0;
  //Functionality not prepared yet.
  while (clockisset == false){
    int buttoner = generalButtonmanage();
    if (mode == 0){
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0, 25);
    tft.setTextSize(5);
    tft.print(hour);
    if (buttoner == 1 ){
      hour++;
    }
    else if (buttoner == 2){
      hour--;
    }
    if (buttoner == 3){
      mode++;
    }
    }
    else if (mode == 1){
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(0, 80);
      tft.setTextSize(5);
      tft.print(minute);
      if (buttoner == 1 ){
        minute++;
      }
      else if (buttoner == 2){
        minute--;
      }
      if (buttoner == 3){
        mode++;
      }
    }
    else if (mode == 2){
      mode = 0;
      clockisset = true;
    }
  }
  return;
}
int generalButtonmanage(){
  if (button() == 0){ // Checks if button is pressed
  pressTime = millis(); // Store when button was pressed
  while (button() == 0){} //This causes it to wait for the button to be unpressed
  releaseTime = millis(); // Once the button is no longer = 0, check what time it is.
  unsigned long duration = releaseTime - pressTime;
    if (duration >= 1500){
      return 3; //Long Hold
    }
    else if (duration >= 500 && duration < 1000){
      return 2; //Short Hold
    }
    else if (duration < 250){
      return 1; //Quick Press
    }
  }
  return 0;
}
