/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik Ekblad
 *
 * DESCRIPTION
 * Example sketch showing how to control physical relays.
 * This example will remember relay state after power failure.
 * http://www.mysensors.org/build/relay
 */

#ifndef F_CPU
#define F_CPU 1600000UL
#endif

// Enable debug prints to serial monitor
//#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

#include <MySensors.h>
//#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <PCF8574.h>
#include <Wire.h>
#include <NewPing.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <TimedAction.h>
#include <ACS7xx_Allegro.h>

#define SENSOR_RELAY_PIN	4       // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define SENSOR_RELAY_ID		1
#define RELAY_ON			1       // GPIO value to write to turn on attached relay
#define RELAY_OFF			0		// GPIO value to write to turn off attached relay
#define SENSOR_SR04_ID		2
#define SENSOR_ACS_ID	    3		//SENSOR_ACS_ID

#define LED_PIN_MEASURE		3
#define LED_PIN_PUMP		2
#define LED_PIN_OVER		1
#define LED_PIN_STOP		0
#define LED_PIN_LCD			7

PCF8574 expander;

boolean backlight = true;

//Menu
int menuitem = 1;
int frame = 1;
int page = 1;
int lastMenuItem = 1;

String menuItem1 = "Back";
String menuItem2 = "Mode: AUTO";
String menuItem3 = "MIN Level";
String menuItem4 = "MAX Level";
String menuItem5 = "Measure time";
String menuItem6 = "Light: ON";
String menuItem7 = "Statistic";
String menuItem8 = "Reset";
String menuItem9 = "About";

boolean up = false;
boolean down = false;
boolean middle = false;

ClickEncoder *encoder;
int16_t last, value;

// example for ACS712 : bidir = true, A0 is the sensor pin, 5.0 is the volatge board, 0.1 is the sensibility of the chip
//ACS7XX_ALLEGRO currentSensor(true, 0, 5.0, 0.1);

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

#define TRIGGER_PIN  8  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     14  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 100 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
unsigned long sensorPoziom;

// 
//MyMessage msg_acs(SENSOR_ACS_ID, V_CURRENT);
MyMessage msg_rel(SENSOR_RELAY_ID, V_STATUS);
MyMessage msg_dis(SENSOR_SR04_ID, V_DISTANCE);

TimedAction  timerPomiar =	TimedAction(5000, pomiarIsr);
	
int wrkMode;

void before()
{
	expander.begin(0x20);   
	// Then set relay pins in output mode
    expander.pinMode(SENSOR_RELAY_PIN, OUTPUT);
    // Set relay to last known state (using eeprom storage)
    expander.digitalWrite(SENSOR_RELAY_PIN, loadState(SENSOR_RELAY_ID)?RELAY_ON:RELAY_OFF);
}

void setup()
{
	expander.pinMode(LED_PIN_STOP, OUTPUT);  // LED RED    - STOP
	expander.pinMode(LED_PIN_OVER, OUTPUT);  // LED YELLOW - OVER
	expander.pinMode(LED_PIN_PUMP, OUTPUT);  // LED GREEN  - Pump
	expander.pinMode(LED_PIN_MEASURE, OUTPUT);  // LED WHITE  - Measure
	expander.pullUp(5);
	expander.pinMode(5, INPUT);   // BUTTON - MANL
	expander.pullUp(6);
	expander.pinMode(6, INPUT);   // BUTTON - STOP
	expander.pinMode(LED_PIN_LCD, OUTPUT);   // Podświetlenie do LCD
	expander.clear();
	//expander.digitalWrite(0, LOW);
	//expander.digitalWrite(0, LOW);
	//expander.digitalWrite(4, HIGH);
	//expander.blink(4, 10, 500); //(4-pin, 10-ilosc mrugniec, 500-ms każdy stan)

	//currentSensor.begin();
    //expander.digitalWrite(LED_PIN_LCD, HIGH);
		
	expander.enableInterrupt(2, onKeyboard);  //obsługa przerwania od PCFa
	expander.attachInterrupt(5,onKeyStop,FALLING);
	expander.attachInterrupt(6,onKeyManl,FALLING);
	
	encoder = new ClickEncoder(A2, A1, A3);	//A3 button
	//encoder->setAccelerationEnabled(false);
	Timer1.initialize(1000);
	Timer1.attachInterrupt( encoderIsr );	  
	last = encoder->getValue();
	  
	display.begin();
    display.clearDisplay();
    EkranIntro();
	
	wrkMode=0;
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Pump", "1.0");

  present(SENSOR_RELAY_ID, S_BINARY);
  present(SENSOR_SR04_ID, S_DISTANCE);
  //present(SENSOR_ACS_ID, S_MULTIMETER);
}


void loop()
{
	timerPomiar.check();

	readRotaryEncoder();

	ClickEncoder::Button b = encoder->getButton();
	if (b != ClickEncoder::Open)
	{
		switch (b)
		{
			case ClickEncoder::Clicked:
			middle=true;
			break;
		}
	}
	
	if (up && page == 1 )
	{
		up = false;
		if(menuitem==2 && frame ==2)
		{
			frame--;
		}
		if(menuitem==3 && frame ==3)
		{
			frame--;
		}
		if(menuitem==4 && frame ==4)
		{
			frame--;
		}
		if(menuitem==5 && frame ==5)
		{
			frame--;
		}
		if(menuitem==6 && frame ==6)
		{
			frame--;
		}
		if(menuitem==7 && frame ==7)
		{
			frame--;
		}
		
		lastMenuItem = menuitem;
		menuitem--;
		if (menuitem==0)
		{
			menuitem=1;
		}
	}
	else if (up && page == 2 && menuitem==1 ) 
	{
		up = false;
		/*
		contrast--;
		setContrast();
		*/
	}
	else if (up && page == 2 && menuitem==2 ) 
	{
		up = false;
		//volume--;
	}
	else if (up && page == 2 && menuitem==3 ) 
	{
		up = false;
		/*
		selectedLanguage--;
		if(selectedLanguage == -1)
		{
			selectedLanguage = 2;
		}
		*/
	}
	else if (up && page == 2 && menuitem==4 ) 
	{
		up = false;
		/*
		selectedDifficulty--;
		if(selectedDifficulty == -1)
		{
			selectedDifficulty = 1;
		}
		*/
	}

	if (down && page == 1) //We have turned the Rotary Encoder Clockwise
	{
		down = false;
		if(menuitem==3 && lastMenuItem == 2)
		{
			frame ++;
		}
		else if(menuitem==4 && lastMenuItem == 3)
		{
			frame ++;
		}
		else if(menuitem==5 && lastMenuItem == 4)
		{
			frame ++;
		}
		else if(menuitem==6 && lastMenuItem == 5)
		{
			frame ++;
		}
		else if(menuitem==7 && lastMenuItem == 6)
		{
			frame ++;
		}
		else if(menuitem==8 && lastMenuItem == 7 && frame!=7)
		{
			frame ++;
		}
				
		lastMenuItem = menuitem;
		menuitem++;
		if (menuitem==10)
		{
			menuitem--;
		}
	}
	else if (down && page == 2 && menuitem==1) 
	{
		down = false;
		/*
		contrast++;
		setContrast();
		*/
	}
	else if (down && page == 2 && menuitem==2) 
	{
		down = false;
		//volume++;
	}
	else if (down && page == 2 && menuitem==3 ) 
	{
		down = false;
		/*
		selectedLanguage++;
		if(selectedLanguage == 3)
		{
			selectedLanguage = 0;
		}
		*/
	}
	else if (down && page == 2 && menuitem==4 ) 
	{
		down = false;
		/*
		selectedDifficulty++;
		if(selectedDifficulty == 2)
		{
			selectedDifficulty = 0;
		}
		*/
	}
 	
	if (middle) //Rotarry Middle Button is Pressed
	{
		middle = false;
		
		if(wrkMode!=1)
			wrkMode=1;
		else
		{							
			if(page == 1 && menuitem == 1)
				wrkMode=0;
			
			if (page == 1 && menuitem == 6) // Backlight Control
			{
				if (backlight)
				{
					backlight = false;
					menuItem6 = "Light: OFF";
					//turnBacklightOff();
				}
				else
				{
					backlight = true;
					menuItem6 = "Light: ON";
					//turnBacklightOn();
				}
			}

			if(page == 1 && menuitem == 8)// Reset
			{
				resetDefaults();
			}
			
			if(page == 2 && menuitem == 9)// About
			{
				page=1;
			}
		}
	}	// koniec obsługi funkcji po naciśnięciu przycisku Rottary
 
	switch(wrkMode)
	{
		case 0:
		// TRYB AUTO, PUMP OFF
		 EkranPomiar(true, false, false);
		 break;
	 
		 case 1:
		 // menu
		 EkranMenu();
		 break;
	 
		 case 2:
		 EkranStop();
	}
  
  /*
  double distance = distanceSensor.measureDistanceCm();
  send(msg_dis.set(distance));  
  
  currentSensor.instantCurrent(&currentNow);
  Serial.print("courant mesure: ");
  Serial.println(currentNow, DEC);
  Serial.print("moyenne mouvante: ");
  Serial.println(currentSensor.getMovingAvgExp(), DEC);

  send(msg_acs.set(currentNow)); //V_CURRENT
  */
  
  //send(msg_dis.set(distance,2));  
}

void receive(const MyMessage &message)
{
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_STATUS) 
  {
    // Change relay state
    expander.digitalWrite(SENSOR_RELAY_PIN, message.getBool()?RELAY_ON:RELAY_OFF);
    expander.digitalWrite(LED_PIN_PUMP, message.getBool()?RELAY_ON:RELAY_OFF);
	// Store state in eeprom
    saveState(SENSOR_RELAY_ID, message.getBool());
    // Write some debug info
    //Serial.print("Incoming change for sensor:");
    //Serial.print(message.sensor);
    //Serial.print(", New status: ");
    //Serial.println(message.getBool());
  }
}

void pomiarIsr()
{
	// migniemy LED MEASURE
	expander.digitalWrite(LED_PIN_MEASURE, HIGH);   // turn the LED on (HIGH is the voltage level)
	delay(5);
	expander.digitalWrite(LED_PIN_MEASURE, LOW);    // turn the LED off by making the voltage LOW

	sensorPoziom = sonar.ping_cm();
	//send(msg_dis.set(sensorPoziom));
}

void resetDefaults()
{
	menuItem5 = "Light: ON";
}

// -------------------------------------------------
// Encoder
// -------------------------------------------------
//

void encoderIsr()
{
	encoder->service();
}

void readRotaryEncoder()
{
	value += encoder->getValue();
	
	if (value/2 > last) 
	{
		last = value/2;
		down = true;
		delay(150);
	}
	else if (value/2 < last) 
	{
		last = value/2;
		up = true;
		delay(150);
	}
}