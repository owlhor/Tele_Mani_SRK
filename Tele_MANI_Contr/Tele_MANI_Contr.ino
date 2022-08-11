/* ====================================================
 * Teleoperated Mobile Manipulation (Smart Rod Khen)
 * Power Control Circuit
 * use Arduino Nano on OF-TMSRK-CT011 circuit Board 
 * Human Computer Interface Lab (HCI)
 * written by owl_hor, FRAB7 FIBO KMUTT.
 * Academic Usage
 * ====================================================
 * ------------OF-TMSRK-CT011 Pin Map------------------
 * D2 <- R_sig	, Signal from ON/OFF button)
 * D3 -> Rctr 	, Xternal Relay Control
 * D4 <- I1 	, Button Input
 * D5 <- I2 	, Button Input
 * D6 <- I3 	, Button Input
 * D7 <- I4 	, Button Input
 * D8 -> L1		, LED-Pilot Lamp Drive
 * D9 -> L2		, LED-Pilot Lamp Drive & Turn ONOFF Jetson NANO Relay
 * D10 -> L3	, LED-Pilot Lamp Drive
 *
 * A0 <- Acr	, Current Sense
 * A1 <- Vsn	, Voltage Sense
 *
 * A4 -> SCLK	, SH1106 
 * A5 <-> SDA	, SH1106
  * ====================================================
 */

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "Adafruit_GFX.h"
#include "Adafruit_SH1106.h"

int ONOFF_Pin = 2;
int relay_pin = 3;
// 4 5 6 7 = button input
int emer_Pin = 5;
// 8 9 10 = LED out
int Pilot_L1 = 8;
int Pilot_L2 = 9;
int Pilot_L3 = 10;

int buttonState = 0;
int emerState = 0;

int grand_cnt = 0; 								// multipurpose counter in GrandState
int initsub_cnt =0;

int shutdown_cnt = 0; 							// shutdown time counter (delay must = 1)
int shutdown_time = 50; 						// Define time gap before Jetson nano Shutdown here

static enum {OFF,INIT,ON,SHUTDOWN,EMERGENCY} GrandState = INIT;
static enum {READY,STOP,WORKING,ERROR,NA} ActiveState = READY;

char* StBuffer;
 
uint32_t timestamp_onoff = 0;
uint32_t timestamp_display = 0;
uint32_t timestamp_grand = 0;

// displaydefine
#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);

//// ACS758-------------------------------
//// bit resolution 10 bit ADC
//// 0-1023 bit for bidirection / unidirection amplified
//// 512 - 1023 for unidirection unamplified
float I_range = 50; 						// I amp need 30-ACS712 50-ACS758
float ADC_range = 1024;						// ADC work range, [512,1023]=512 -> unamplified, 
											// [125,1023]=899 -> opampV0, [0,1023]=1024 -> opampV-
float resolute = I_range / ADC_range; 		
float ana_offset = 0; 						// 512, 125

//VoltageDivide Sense constant
float resi_1 = 5100.0; // resistance in ohm
float resi_2 = 1000.0;
float v_multiply = (resi_1+resi_2)/ resi_2;
//float maxVrange = (resi_2 /resi_1) * 26.0; 


void setup()
{ // pin
  pinMode(relay_pin, OUTPUT); 
  pinMode(ONOFF_Pin, INPUT);

  pinMode(Pilot_L1, OUTPUT);
  pinMode(Pilot_L2, OUTPUT);
  pinMode(Pilot_L3, OUTPUT); 
  
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
		// ampV0 = (50/(1024-125)) * (a - 125)
		// ampV- = (50/1024)* a;
    return ct;}
  
float VoltageDivide(int avi){
  //xxfloat read_volt = maxVrange * (avi / 1024.0);
  float read_volt = (avi+3) * (5.0/1024.0); // +3 to offset near meter measured
  return v_multiply * read_volt;
}

/////////loop///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  buttonState = digitalRead(ONOFF_Pin);
  emerState = digitalRead(emer_Pin);
  float currnt = getC();
  int acrnt = analogRead(A0);
  int avolt = analogRead(A1);
  
  float volt_result = VoltageDivide(avolt);

  //Serial.print("current = "); Serial.println(currnt);

  //DislayDrive
  if (millis() - timestamp_display >= 50){
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
  
  display.setCursor(105, 15);
  display.println(shutdown_cnt);
  
  display.setCursor(115, 5);
  display.println("A");

  display.setCursor(5, 45);
  display.println("St:");
  
  display.setTextSize(2);
  
  display.setCursor(40, 45);
  //char* buffzz = "555";
  display.println(StBuffer); 
  
  display.display();
    }

	// Btn Counter Driver
	/*
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
}*/
  
  // Grand State
  if (millis()- timestamp_grand >= 20){
	  timestamp_grand = millis();

  //// Pilot Lamp Set
	if (GrandState == ON){digitalWrite(Pilot_L1, 1);}
	else{digitalWrite(Pilot_L1, 0);}
	if (GrandState == INIT){}
	else{digitalWrite(Pilot_L2, 0);}
	if (GrandState == SHUTDOWN){digitalWrite(Pilot_L3, 1);}
	else{digitalWrite(Pilot_L3, 0);}

 // if (GrandState != INIT || GrandState != SHUTDOWN){digitalWrite(Pilot_L2, 0);}
  
    /////// Emergency /////// Emergency /////// Emergency /////// Emergency
	//if (emerState == 1){GrandState = EMERGENCY;}
	  switch (GrandState){
		  default:
		  case OFF://///////////////////////////////////////////////////////////////////////////////
		  StBuffer = "OFF";
     
		  if (buttonState == 1){
			  GrandState = INIT;
			  digitalWrite(relay_pin, 1);  // Active Relay
		  }
		  break;
		  
		  case INIT:////////////////////////////////////////////////////////////////////////////////
		  StBuffer = "INIT";
		  digitalWrite(relay_pin, 1);  // Active Relay
		
		// Jetson nano are ready / dummy as 8 sec pass
		  
		  //// RelayJS Mini Trig ON
      if (grand_cnt <= 5) //  rising edge
      { 
      digitalWrite(Pilot_L2, 1);
      }else{
      digitalWrite(Pilot_L2, 0);
      }
      
		  if (grand_cnt >= 40) 		
		  {
			  GrandState = ON;
			  ActiveState = READY;
			  grand_cnt = 0; 		// reset counter
			  shutdown_cnt = 0;
		  }else{grand_cnt++;}
		  
		if (buttonState == 0){
       GrandState = OFF;
        digitalWrite(relay_pin, 0);  // Deactive Relay
      }
		if (emerState == 1){
        GrandState = EMERGENCY;
        grand_cnt = 0;
        }
		  break;
		  
		  case ON://///////////////////////////////////////////////////////////////////////////////////
		  StBuffer = "ON";
		  if (buttonState == 0){
			  GrandState = SHUTDOWN;
			  ActiveState = NA;
			  }
       if (emerState == 1){GrandState = EMERGENCY;}
				/*
			  switch(ActiveState){
				default:
				case READY:
				  if (){ // get order to work
					  ActiveState = WORKING;
				  }
				  if (){ // get some error
					  ActiveState = ERROR;
				  }
				  if (){ // get stop button
					  ActiveState = STOP;
				  }
				  if (){ // get emer button
					  ActiveState = EMERGENCY;
				  }
				  break;
			  
				case ERROR:
				// report error & disable some order
				// no exit, force to shutdown
				break;
				
				case STOP:
				// mobility stop for 2 sec and free
				// maniP stop, release by button
				if (){ //stop button is 0
					ActiveState = READY;
				}
				break;
				
				case WORKING:
				if (){ // get stop button
					  ActiveState = STOP;
				  }
				break;
				
				
				
				case NA:
				// nothing, not in any state, machine is on grandstate
				break;
				} 
				*/
		  break;
		  ///////////////////// EMER //////////////////////////////////////////////
		  case EMERGENCY:
				StBuffer = "EMER";
				// Order Jetson nano to reboot itself
				if (emerState == 0){ // Release Emer button
					GrandState = INIT;
					//ActiveState = NA;
				}
				break;
		  /////////////////////////// Shutdown ////////////////////////////////////
		  case SHUTDOWN:
		  StBuffer = "SHTDWN";
		  shutdown_cnt++;
		  
		  //Drive relayJS to shutdowning 
		  digitalWrite(Pilot_L2, 1);
		  
		  if (shutdown_cnt >= shutdown_time ){ // || currnt < 1
			  digitalWrite(relay_pin, 0);  // Deactive Relay
			  digitalWrite(Pilot_L2, 0);   //Drive relay to shutdowning 
		  
			  GrandState = OFF;
			  delay(20);
		  }
     if(buttonState == 1){
      GrandState = ON;
      shutdown_cnt = 0;}
		  break;
		  
	  }
  }
}
