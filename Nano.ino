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

int counter = 0;
int buttonPin = 2;
int buttonState;
int State = LOW;
int instances = 11; // The amount of menu logic options.
float X, Y, Z;
int R, G, B;
float proxy = 0;
bool sleepState = false;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

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
}
void loop() {
  // put your main code here, to run repeatedly:
  managebutton();
  modeManage(State);
}
void managebutton(){
  int reading = digitalRead(buttonPin);

  if (reading != State) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

    }
  }
  State = reading;
}
void modeManage(int status){
  if (status == LOW){
    if (counter != instances){
    counter++;
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    sleepState = false;
    }
    else {
    counter = 0;
    }
  }

  switch (counter) {
  case 0: runAccelerometer(); break;
  case 1: runGyroscope(); break;
  case 2: runPressure(); break;
  case 3: runAltitude(); break;
  case 4: runMagnetometer(); break;
  case 5: runTemperature(); break;
  case 6: runHumidity(); break;
  case 7: runColor(); break;
  case 8: runProxy(); break;
  case 9: runMicrophone(); break;
  case 10: runFlashlight(); break;
  default: // This acts as a "sleep" mode for the system. The amount of instances exceeds the case count by 1, so one menu option will trigger this default.
  if (sleepState == false){
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
  break;
  }
}
void runAccelerometer(){
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
void runGyroscope(){
  IMU.readGyroscope(X, Y, Z);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Gyroscope");
  tft.setCursor(0, 50);
  tft.fillRect(15, 50, 50, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("X: ");
  tft.println(X);
  analogWrite(LEDR, 255 - fabs(X));
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Y: ");
  tft.println(Y);
  analogWrite(LEDG, 255 - fabs(Y));
  tft.setTextColor(ST77XX_RED);
  tft.print("Z: ");
  tft.println(Z);
  analogWrite(LEDB, 255 - fabs(Z));
}
void runPressure(){
  float P = BARO.readPressure();
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Air Pressure");
  tft.setCursor(0, 50);
  tft.fillRect(15, 50, 45, 10, ST77XX_BLACK);
  tft.print("P: ");
  tft.print(P);
  tft.println("  kPa");
  analogWrite(LEDR, 255 - fabs(P));
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
void runMagnetometer(){
  IMU.readMagneticField(X, Y, Z);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Magnetometer");
  tft.setCursor(0, 50);
  tft.fillRect(15, 50, 45, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("X: ");
  tft.println(X);
  analogWrite(LEDR, 255 - fabs(X));
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Y: ");
  tft.println(Y);
  analogWrite(LEDG, 255 - fabs(Y));
  tft.setTextColor(ST77XX_RED);
  tft.print("Z: ");
  tft.println(Z);
  analogWrite(LEDB, 255 - fabs(Z));
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
int axisToPWM(float v) {
  v = fabs(v);               // absolute acceleration
  v = constrain(v, 0, 2);    // typical range when tilting
  return map(v * 100, 0, 200, 0, 255);
}
void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();

  // read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}
