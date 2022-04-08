/* ====================================================
 * Teleoperated Mobile Manipulation (Smart Rod Khen)
 * Power Control Circuit
 * use Arduino Nano on OF-TMSRK-CT011 circuit Board 
 * Human Computer Interface Lab (HCI)
 * written by owl_hor, FRAB7 FIBO KMUTT.
 * Academic Usage
 * ====================================================
 */

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "Adafruit_GFX.h"
#include "Adafruit_SH1106.h"

int buttonPin = 2;
int relay_pin = 9;
int buttonState = 0,lockState=0,RelsState = 0;

int count = 0,delta;
int mbut_lock = 3,mbut_rels = 4, motState = 3;

int shutdown_cnt = 0; 							// shutdown time counter (delay must = 1)
int shutdown_time = 50; 						// Define time gap before Jetson nano Shutdown here

static enum {INIT,READY,WORKING,ERROR,LOCK} GrandState = INIT;

uint32_t timestamp_onoff = 0;
uint32_t timestamp_display = 0;

// displaydefine
#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);

//// ACS758-------------------------------
//// bit resolution 10 bit ADC
//// 0-1023 bit for bidirection / unidirection amplified
//// 512 - 1023 for unidirection unamplified
float I_range = 50; 						// I amp need 30-ACS712 50-ACS758
float ADC_range = 899;						// ADC work range, [512,1023]=512 -> unamplified, 
float resolute = I_range / ADC_range; 		// [125,1023]=899 -> opampV0, [0,1023]=1024 -> opampV-
float ana_offset = 125; 					// 512

//VoltageDivide Sense constant
float resi_1 = 5100.0; // resistance in ohm
float resi_2 = 1000.0;
float v_multiply = (resi_1+resi_2)/ resi_2;
float maxVrange = (resi_2 /resi_1) * 26.0; 


void setup()
{ // pin
  pinMode(relay_pin, OUTPUT); 
  pinMode(buttonPin, INPUT);
  //pinMode(mbut_ena, INPUT);
  pinMode(mbut_lock, INPUT);
  pinMode(mbut_rels, INPUT);
  
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  
  Serial.begin(115200);
}

// Edge Detect
/*
int myfunc(int x){
  static int buf1,buf,delta;
  //buf2 = buf1;
  buf1 = buf;
  buf = x;
  delta = x - buf1;
  return delta;
}*/


// current read ACS758-------------------------------
float getC() {
    float a = analogRead(A0);
    float ct = resolute * (a - ana_offset);
    // unamp = (50/512) * (a - 512)
    // ampV0 = (50/(1024-125)) * (a-125)
    // ampV- = (50/1024)* a;
    return ct;}
  
float VoltageDivide(int avi){
  float read_volt = maxVrange * (avi / 1024.0);
  Serial.print("maaxvrt = "); Serial.println(read_volt);
  return v_multiply * read_volt;
}

/////////loop///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  buttonState = digitalRead(buttonPin);
  lockState = digitalRead(mbut_lock);
  RelsState = digitalRead(mbut_rels);
  //delta = myfunc(buttonState);
  float currnt = getC();
  int acrnt = analogRead(A0);
  int avolt = analogRead(A1);
  
  float volt_result = VoltageDivide(avolt);

   

  //Serial.print("current = "); Serial.println(currnt);

  //DislayDrive
  if (millis() - timestamp_display >= 10){
  timestamp_display = millis(); 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5, 5);     //  range from left,range from up
  display.println("I= ");
  
  display.setCursor(25, 5);
  display.println(currnt); // , DEC
  
  display.setCursor(5, 15);
  display.println("rwA0:");
  display.setCursor(40, 15);
  display.println(acrnt);
  display.setCursor(65, 15);
  display.println(int(acrnt-ana_offset));
  
  display.setCursor(0, 17);
  display.println("_____________________");
  
  display.setCursor(5, 25);
  display.println("Volt:"); // 
  display.setCursor(40, 25);
  display.println(volt_result); // 
  display.setCursor(85, 25);
  display.println(avolt); //
  display.setCursor(115, 25);
  display.println("V");
  /*
  display.setCursor(5, 35);
  display.println("ShV:");
  display.setCursor(40, 35);
  display.println(_____); 
  display.setCursor(115, 35);
  display.println("mV");
  
  display.setCursor(5, 45);
  display.println("Ish:");
  display.setCursor(40, 45);
  display.println(_____);
  display.setCursor(115, 45);
  display.println("mA");
  
  display.setCursor(5, 55);
  display.println("Pwr:");
  display.setCursor(40, 55);
  display.println(_____);
  display.setCursor(115, 55);
  display.println("mW");
  */
  display.setCursor(105, 15);
  display.println(shutdown_cnt);
  
  //display.setTextSize(2);
  display.setCursor(115, 5);
  display.println("A");
  
  display.display();
    }

	// Btn Counter Driver
  if (millis() - timestamp_onoff >= 25){//run every x msec  
    timestamp_onoff = millis();
    
    switch (buttonState) {
    default:
    case 1:
    digitalWrite(relay_pin, 1);
    shutdown_cnt = 0;
    break;
    
    case 0:
    if (shutdown_cnt >= shutdown_time){ // or 
      digitalWrite(relay_pin, 0);
    }
    else{ shutdown_cnt++; }
    break;
    }
}
  
}
