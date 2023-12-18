#define BLYNK_TEMPLATE_ID "TMPL6IbGXx4s-"
#define BLYNK_TEMPLATE_NAME "LED"
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
#define APP_DEBUG
#define USE_NODE_MCU_BOARD
const int sensorIn = A0;
int mVperAmp = 80; // use 185 for 5A, 100 for 20A Module and 66 for 30A Module
//defining libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <String.h>
#include "BlynkEdgent.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
BlynkTimer timer;
WiFiUDP ntpUDP;
//const long utcOffsetInSeconds = 198000;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#define OLED_RESET     -1 // Reset pin
//defining variables
int startTime1;
int endTime1;

//turning on and off the bulb using blynk

BLYNK_WRITE(V0)
{
    int value = param.asInt();
   if(value == 1)
      digitalWrite(D5, HIGH);
    else
      digitalWrite(D5, LOW);
}

//saving the time from blynk into variables

BLYNK_WRITE(V2){
 startTime1 = param[0].asInt();
 endTime1 = param[1].asInt();
 if(startTime1 == 0 && endTime1 == 0)
 {
  startTime1 = 999999;
  endTime1 = 999999;
 }
}
//EDITING OF OLED CURRENT TEXT DESCRIPTION

void maincut(double ct)
{
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0,30);
  display.print("Amps:");
  display.setCursor(60,30);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print(ct);
  display.display();
  delay(2000);
  display.clearDisplay();
}

//intialization of blynk,timer and pins

void setup()
{
  Serial.begin(115200);
    pinMode(D5, OUTPUT);
    pinMode(sensorIn, INPUT);
    digitalWrite(D5, LOW);
    delay(100);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
}
   BlynkEdgent.begin();
   timeClient.begin();
   timeClient.setTimeOffset(18000);
   timer.setInterval(3000L, sendCurrentData); 
}

void loop() {
   
    BlynkEdgent.run();
    timer.run(); 
    timeClient.update();
    
  int HH = timeClient.getHours();
  int MM = timeClient.getMinutes();
 // int SS = timeClient.getSeconds();
  //converting the time into seconds
  int server_time = 3600*HH + 60*MM;
 // Serial.print(timeClient.getFormattedTime());
  Serial.print('\n');
  Serial.print(server_time);
  Serial.print('\n');
  delay(500);
//if the time from blynk matches, then turn on the bulb and vice versa
   if(startTime1 == server_time)
   {
    digitalWrite(D5, HIGH);
    Serial.print("HIGH");  
   }

   if(endTime1 == server_time)
   {
    digitalWrite(D5, LOW);
    Serial.print("LOW");
   }

  
}

void sendCurrentData() {
  
 double Voltage = getVPP();
 double VRMS = (Voltage/2.0) *0.707; // sq root
 unsigned long previousMillis = 0;
 double AmpsRMS = (VRMS * 1023)/mVperAmp;
 if (AmpsRMS < 0)
 {
  AmpsRMS ==0.00;
 }
 float Wattage = (220*AmpsRMS)-4; //Observed 18-20 Watt when no load was connected, so substracting offset value to get real consumption.
  Serial.println("AmpsRMS ");
  Serial.print(AmpsRMS);
  Serial.println("Watt ");
  Serial.print(Wattage); 
  maincut(AmpsRMS);
  Blynk.virtualWrite(V1, AmpsRMS);
  
}



float getVPP()
{
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  uint32_t start_time = millis();

   while((millis()-start_time) < 1000) //sample for 2 Sec
   {
       readValue = analogRead(sensorIn);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the maximum sensor value*/
           minValue = readValue;
       }
/*       Serial.print(readValue);
       Serial.println(" readValue ");
       Serial.print(maxValue);
       Serial.println(" maxValue ");
       Serial.print(minValue);
       Serial.println(" minValue ");
       delay(1000); */
    }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 5)/1024.0;
      
   return result;
 }
