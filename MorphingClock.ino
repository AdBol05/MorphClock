// Morphing Clock by Hari Wiguna, July 2018
// Modified at Laborky.cz 2021/2022 by Adam Bolemant
unsigned int R = 11; //default display colour
unsigned int G = 100;
unsigned int B = 35;
// exception in NTPClient.cpp on line 135

bool update = false;
int analog; 

#define double_buffer
#include <PxMatrix.h>

#ifdef ESP32

#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 2
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#endif

#ifdef ESP8266

#include <Ticker.h>
Ticker display_ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2

#endif

// Pins for LED MATRIX
PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);

#ifdef ESP8266
// ISR for display refresh
void display_updater()
{
  display.display(70);
}
#endif

#ifdef ESP32
void IRAM_ATTR display_updater() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  display.displayTestPattern(70);
  portEXIT_CRITICAL_ISR(&timerMux);
}
#endif

#include "Digit.h"
#include "NTPClient.h"
NTPClient ntpClient;
unsigned long prevEpoch;
byte prevhh = 99;
byte prevmm = 99;
byte prevss = 99;

void setup() {
  Serial.begin(9600);
  display.begin(16);
#ifdef ESP8266
  display_ticker.attach(0.002, display_updater);
#endif

#ifdef ESP32
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &display_updater, true);
  timerAlarmWrite(timer, 2000, true);
  timerAlarmEnable(timer);
#endif

   ntpClient.Setup(&display);
   display.fillScreen(display.color565(0, 0, 0));
}


void loop() {
 randomSeed(analogRead(0));
 Digit digit0(&display, 0, 63 - 1 - 9*1, 8, display.color565(R, G, B));
 Digit digit1(&display, 0, 63 - 1 - 9*2, 8, display.color565(R, G, B));
 Digit digit2(&display, 0, 63 - 4 - 9*3, 8, display.color565(R, G, B));
 Digit digit3(&display, 0, 63 - 4 - 9*4, 8, display.color565(R, G, B));
 Digit digit4(&display, 0, 63 - 7 - 9*5, 8, display.color565(R, G, B));
 Digit digit5(&display, 0, 63 - 7 - 9*6, 8, display.color565(R, G, B));
 unsigned long epoch = ntpClient.GetCurrentTime();
 int hh = ntpClient.GetHours();
 int mm = ntpClient.GetMinutes();
 int ss = ntpClient.GetSeconds();
 display.fillScreen(display.color565(0, 0, 0));
 digit1.DrawColon(display.color565(R, G, B));
 digit3.DrawColon(display.color565(R, G, B));
 update = false;
 delay(1000);
 
 while(update == false){
  epoch = ntpClient.GetCurrentTime();
  if (epoch != 0) ntpClient.PrintTime();
  hh = ntpClient.GetHours();
  mm = ntpClient.GetMinutes();
  ss = ntpClient.GetSeconds();
  if (ss == prevss && mm == prevmm && hh == prevhh) { // If current time is the same as previous draw it without morphing.
   digit0.Draw(ss % 10);
   digit1.Draw(ss / 10);
   digit2.Draw(mm % 10);
   digit3.Draw(mm / 10);
   digit4.Draw(hh % 10);
   digit5.Draw(hh / 10);
  }
  else
  {
    if (ss!=prevss) { 
      int s0 = ss % 10;
      int s1 = ss / 10;
      if (s0!=digit0.Value()) digit0.Morph(s0);
      if (s1!=digit1.Value()) digit1.Morph(s1);
      prevss = ss;
    }
    if (mm!=prevmm) {
      int m0 = mm % 10;
      int m1 = mm / 10;
      if (m0!=digit2.Value()) digit2.Morph(m0);
      if (m1!=digit3.Value()) digit3.Morph(m1);
      prevmm = mm;
    }    
    if (hh!=prevhh) {
      int h0 = hh % 10;
      int h1 = hh / 10;
      if (h0!=digit4.Value()) digit4.Morph(h0);
      if (h1!=digit5.Value()) digit5.Morph(h1);
      prevhh = hh;
    }
  }
  analog = analogRead(0);
  if(analog == 0){ //set random display colour (based on analog input)
   Serial.println("---------------------"); 
   Serial.println("RGBvalues:");
   R = random(256);
   G = random(256);
   B = random(256);
   Serial.print("R: ");
   Serial.println(R);
   Serial.print("G: ");
   Serial.println(G);
   Serial.print("B: ");
   Serial.println(B);
   Serial.print("---------------------");
   update = true;
  }
 }
}
