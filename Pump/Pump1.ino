
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
 * 9/10/2017 Version 1.0 - Krzysztof Furmaniak
 *
 * These helper macros generate a numerical and alphanumerical (see http://www.semver.org) representation of the library version number, i.e
 *
 * Given a version number MAJOR.MINOR.PATCH, increment the:
 *
 * MAJOR version when you make incompatible API changes,
 * MINOR version when you add functionality in a backwards compatible manner, and
 * PATCH version when you make backwards compatible bug fixes.
 * Additional labels for pre-release and build metadata are available as extensions to the MAJOR.MINOR.PATCH format.
 *
 * | SemVer      | Numerical   | Comments
 * |-------------|-------------|------------------
 * | 1.0.0       | 0x010000FF  | final
 * | 1.0.1	 | 0x010001FF  | fix funckji autoStop, w przypadku braku komunikacji z SR04 lub błędnym odczycie w trakcie pompowania, nie następowało wyłączenie pompy
 * |		 |	       | teraz wchodzimy w tryb STOP, dodatkowo zmniejszono czas pompowania do 60s
 * | 
 *
 */

#ifndef F_CPU
#define F_CPU 1600000UL
#endif

/* ################## RADIO
// Enable debug prints to serial monitor
//#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_REPEATER_FEATURE
	
#include <MySensors.h>
 ################## RADIO */

#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <PCF8574.h>
#include <Wire.h>
#include <NewPing.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <TimedAction.h>
#include <Timelib.h>
#include <ACS7xx_Allegro.h>
#include <avr/wdt.h>
#include <EEPROM.h>

#define PUMP_PIN				8       // Arduino Digital I/O pin number
#define PUMP_ON					1       
#define PUMP_OFF				0		

#define BTN_MANL				0		// PCF expander address for button MANUAL
#define BTN_STOP				1
#define PCF_INT_PIN				2

// mySensors child node ID
#define SENSOR_PUMP_ID			1		// pump ON/OFF
#define SENSOR_STOP_ID			2		// wrkMode ON/OFF (0-pomiar, 2-stop)
#define SENSOR_INFO_ID			3		// zbiornikMax
#define SENSOR_WATER_ID			4		// current level of water

#define SR04_TRIGGER_PIN		4		// Arduino pin tied to trigger pin on the ultrasonic sensor.
#define SR04_ECHO_PIN			3		// Arduino pin tied to echo pin on the ultrasonic sensor.
#define SR04_MAX_DISTANCE		100		// Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

// Adresy zmiennych w pamięcie EEPROM
#define EEPROM_wrkMode			0
#define EEPROM_trybPracy		1	// AUTO/MANUAL
#define EEPROM_pomiarInterwal	2
#define EEPROM_sensorMin		3
#define EEPROM_sensorMax		4

// Menu
int menuitem					= 1;
int frame						= 1;
int page						= 1;
int lastMenuItem				= 1;
long inter						= 40;
int	i							= 0;
int waitt						= 50; // In milliseconds
int spacer						= 1;
int width						= 5 + spacer; // The font width is 5 pixels
unsigned long lastt				= 0;
unsigned long lat				= 0;
boolean blinkPomiar;
int autoStop					= 0;
// Encoder
unsigned int selectedValue;
boolean up						= false;
boolean down					= false;
boolean middle					= false;
boolean press					= true;
int16_t last, value;

// LED matrix
int pinCS						= 7; // Attach CS to this pin, DIN to MOSI(11) and CLK to SCK(13) (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays	= 2;
int numberOfVerticalDisplays	= 1;

// Tank description
unsigned int sensorPoziom		= 0;		// aktualna wartosc odczytana z sensora
unsigned int zbiornikPoziom		= 0;		// aktualna wartosc odczytana z sensora
unsigned int zbiornikPoziom_l	= 0;
unsigned int zbiornikMax;
unsigned int uS;

//Parametry pracy zapisywane w pamięcie EEPROM
struct
{
	int wrkMode;
	boolean	trybPracy			= false;	//true-Auto, false-Manual
	unsigned int pomiarInterwal = 1;		// 1-90 sec
	unsigned int sensorMin		= 28;		// 1-99 cm, dno zbiornika czyli z sensora duza wartość
	unsigned int sensorMax		= 8;		// 2-99 cm, gora zbiornika czyli z sensowa mala wartosc sensorMax<sensorMin
}konfig;

NewPing sonar(SR04_TRIGGER_PIN, SR04_ECHO_PIN, SR04_MAX_DISTANCE);

PCF8574 expander;

void pomiarIsr();

ClickEncoder *encoder;

// ACS712 configuration : bidir = true, A0 is the sensor pin, 5.0 is the volatge board, 0.1 is the sensibility of the chip
ACS7XX_ALLEGRO currentSensor(true, 0, 5.0, 0.1);

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

TimedAction  timerPomiar =	TimedAction(1000, pomiarIsr);

/* ################## RADIO
MyMessage msg_stop(SENSOR_STOP_ID, V_STATUS);
MyMessage msg_pump(SENSOR_PUMP_ID, V_STATUS);
MyMessage msg_max(SENSOR_INFO_ID, V_TEXT);
MyMessage msg_poziom(SENSOR_WATER_ID, V_DISTANCE);
 
// -------------------------------------------------
void before()
{
	pinMode(PUMP_PIN, OUTPUT);
	digitalWrite(PUMP_PIN, PUMP_OFF);
	matrix.setIntensity(0);
	matrix.fillScreen(LOW);
	matrix.print(">");
	matrix.write();
	// -----------------------------------------
	// zapis startowy w pamięcie EEPROM, używane tylko podczas programowania
	//
	//saveState(EEPROM_wrkMode, 0);
	//saveState(EEPROM_trybPracy, false);
	//saveState(EEPROM_pomiarInterwal, 1);
	//saveState(EEPROM_sensorMin, 16);
	//saveState(EEPROM_sensorMax, 6);
	//
	// -----------------------------------------
	
}
// -------------------------------------------------
void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Pump", "1.0");
	present(SENSOR_PUMP_ID, S_BINARY);
	present(SENSOR_STOP_ID, S_BINARY);
	present(SENSOR_INFO_ID, S_INFO);
	present(SENSOR_WATER_ID, S_DISTANCE);
}
// -------------------------------------------------
void receive(const MyMessage &message)
{
	// We only expect one type of message from controller. But we better check anyway.
	if (message.type==V_STATUS && message.sensor==SENSOR_STOP_ID)
	{
		if(konfig.wrkMode!=2 && message.getBool()==true)
		{
			// wejście do ekranu STOP
			konfig.wrkMode=2;
			digitalWrite(PUMP_PIN, LOW);
			timerPomiar.disable();
			saveState(EEPROM_wrkMode, konfig.wrkMode);
		}
		else
		{
			if(konfig.wrkMode==2 && message.getBool()==false)
			{
				konfig.wrkMode=0;
				timerPomiar.enable();
				timerPomiar.setInterval(konfig.pomiarInterwal*1000);
				saveState(EEPROM_wrkMode, konfig.wrkMode);				
			}
		}
	}
	
	if (message.type==V_STATUS && message.sensor==SENSOR_PUMP_ID)
	{
		if(konfig.wrkMode==0 && message.getBool()==true)
		{	
		//wejście jezeli sterownik w stanie pomiaru oraz COntroller zarządał włączenia pompy 
			konfig.wrkMode=3;
			digitalWrite(PUMP_PIN, HIGH);
			timerPomiar.setInterval(250);
		}
		else
		{
			if(konfig.wrkMode==3 && message.getBool()==false)
			{
				konfig.wrkMode=0;
				digitalWrite(PUMP_PIN, LOW);
				timerPomiar.setInterval(konfig.pomiarInterwal*1000);
			}
		}
	}
}
################## RADIO */

// -------------------------------------------------

void setup()
{
	wdt_enable(WDTO_2S);	// ustawienie WATCHDOG na 2sec
	
	// Pump relay pins in output mode
	// zdefiniowane w funkcji before() jeżeli mamy radio, wtedy poniższe zablokować
	// jeżeli nie ma radia tutaj musimy to wpisać
	//
	//pinMode(PUMP_PIN, OUTPUT);
	//digitalWrite(PUMP_PIN, PUMP_OFF);	
	pinMode(PUMP_PIN, OUTPUT);
	digitalWrite(PUMP_PIN, PUMP_OFF);

	matrix.setIntensity(0);
	matrix.fillScreen(LOW);
	matrix.print(">");
	matrix.write();
	
	wdt_reset();	// kasowanie WATCHDOG
	
	//Buttons on PCF8574 
	expander.begin(0x20);
	expander.pinMode(BTN_MANL, INPUT);							// BUTTON - MANL
	expander.pullUp(BTN_MANL);
	expander.pinMode(BTN_STOP, INPUT);							// BUTTON - STOP
	expander.pullUp(BTN_STOP);
	
	pinMode(PCF_INT_PIN, INPUT);								// PIN przerwan od PCF
	digitalWrite(PCF_INT_PIN, HIGH);
	expander.enableInterrupt(PCF_INT_PIN, onKeyboard);			//obsługa przerwania od PCFa
	expander.attachInterrupt(BTN_STOP, onKeyStop, FALLING);
	expander.attachInterrupt(BTN_MANL, onKeyManl, FALLING);

	wdt_reset();	// kasowanie WATCHDOG
	
	// encoder
	encoder = new ClickEncoder(A2, A1, A3);						//A3 button
	encoder->setAccelerationEnabled(true);
	Timer1.initialize(1000);
	Timer1.attachInterrupt( encoderIsr );
	last = encoder->getValue();
	
	wdt_reset();	// kasowanie WATCHDOG
	
	// display on LED matrix
	matrix.setIntensity(0);
	EkranIntro();
	matrix.fillScreen(LOW);
	matrix.write();
	
	wdt_reset();	// kasowanie WATCHDOG
	
	// -----------------------------------------
	// zapis startowy w pamięcie EEPROM, używane tylko podczas programowania
	//
	//EEPROM.write(EEPROM_wrkMode, 0);//saveState(EEPROM_wrkMode, 0);
	//EEPROM.write(EEPROM_trybPracy, false);//saveState(EEPROM_trybPracy, false);
	//EEPROM.write(EEPROM_pomiarInterwal, 1);//saveState(EEPROM_pomiarInterwal, 1);
	//EEPROM.write(EEPROM_sensorMin, 18);//saveState(EEPROM_sensorMin, 16);
	//EEPROM.write(EEPROM_sensorMax, 8);//saveState(EEPROM_sensorMax, 8);
	//
	// -----------------------------------------
		// setup initial configuration stored in EEPROM
	konfig.wrkMode			= EEPROM.read(EEPROM_wrkMode);//loadState(EEPROM_wrkMode);
	konfig.trybPracy		= EEPROM.read(EEPROM_trybPracy);//loadState(EEPROM_trybPracy);
	konfig.pomiarInterwal		= EEPROM.read(EEPROM_pomiarInterwal);//loadState(EEPROM_pomiarInterwal);
	konfig.sensorMin		= EEPROM.read(EEPROM_sensorMin);//loadState(EEPROM_sensorMin);
	konfig.sensorMax		= EEPROM.read(EEPROM_sensorMax);//loadState(EEPROM_sensorMax);

	wdt_reset();	// kasowanie WATCHDOG
	
	pomiarIsr();
	
	zbiornikMax	= konfig.sensorMin - konfig.sensorMax;
	timerPomiar.setInterval(konfig.pomiarInterwal*1000);
	/* ################## RADIO
	
	send(msg_max.set(zbiornikMax));
	send(msg_poziom.set(zbiornikPoziom));
	
	################## RADIO */
}

// -------------------------------------------------
// Główna pętla programu
// -------------------------------------------------
//
void loop()
{	
 	wdt_reset();	// kasowanie WATCHDOG
	 
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
		i=0;
		if(menuitem==1)
		{	
			// AUTO/MANUAL
			menuitem=2;
		}
		else if(menuitem==2)
		{
			// sensorMin
			menuitem=3;
			selectedValue = konfig.sensorMin;
		}
		else if(menuitem==3)
		{
			// zbiornikMax
			menuitem=4;
			selectedValue = zbiornikMax;			
		}
		else if(menuitem==4)
		{
			// pomiarInterwal
			menuitem=5;
			selectedValue = konfig.pomiarInterwal;	
		}
		else if(menuitem==5)
		{
			// wyjście z menu
			menuitem=1;
		}		
	}
	else if (up && page == 2 && menuitem==2 ) 
	{	
		// Auto/Manual
		up = false;
		if(konfig.trybPracy)
			konfig.trybPracy=false;		//Manual
		else
			konfig.trybPracy=true;		//Auto
	}
	else if (up && page == 2 && menuitem==3 ) 
	{
		// min value
		up = false;
		selectedValue++;
		if(selectedValue > 99)
			selectedValue=99;
	}
	else if (up && page == 2 && menuitem==4 ) 
	{
		// max value
		up = false;
		selectedValue++;
		if(selectedValue>99)
			selectedValue=99;
	}
	else if (up && page == 2 && menuitem==5 ) 
	{
		// co ile sec pomiar
		up = false;
		selectedValue++;
		if(selectedValue>90)
			selectedValue=90;
	}


	if (down && page == 1) //We have turned the Rotary Encoder Clockwise
	{
		down = false;
		i=0;
		if(menuitem==1)
		{
			menuitem		= 5;
			selectedValue	= konfig.pomiarInterwal;			
		}
		
		else if(menuitem==2)
		{
			menuitem=1;
		}
		
		else if(menuitem==3)
		{
			menuitem		= 2;
			selectedValue	= konfig.sensorMin;
		}
		
		else if(menuitem==4)
		{
			// min value poziom w zbiorniku
			menuitem		= 3;
			selectedValue	= konfig.sensorMin;
		}
		
		else if(menuitem==5)
		{
			// max value poziom w zbiorniku
			menuitem		= 4;
			selectedValue	= zbiornikMax;			
		}
	}
	else if (down && page == 2 && menuitem==2) 
	{
		// Auto/Manual
		down = false;
		if(konfig.trybPracy)
			konfig.trybPracy=false;	//Manual
		else
			konfig.trybPracy=true;		//Auto
	}
	else if (down && page == 2 && menuitem==3) 
	{
		// min value poziom w zbiorniku
		down = false;
		selectedValue--;
		if(selectedValue < 2)	// SR04 realizuje pomiar od 2 cm
			selectedValue=2;
	}
	else if (down && page == 2 && menuitem==4 ) 
	{
		// max value
		down = false;
		selectedValue--;
		if(selectedValue < 2)	// nie moze być mniejsze od min
			selectedValue=2;
	}
	else if (down && page == 2 && menuitem==5 ) 
	{
		// co ile sec pomiar
		down = false;
		selectedValue--;
		if(selectedValue < 1)
			selectedValue=1;
	}
 	
	// Rottary Middle Button is pressed - PROG state
	if (middle) 
	{
		middle = false;
		
		if(konfig.wrkMode!=4)
		{
			//wejscie do menu
			konfig.wrkMode	= 4;	
			page			= 1;
			menuitem		= 1;
		}
		else
		{							
			if(page == 1 && menuitem == 1)	// wyjscie z menu		
			{
				konfig.wrkMode=0;
				timerPomiar.setInterval(konfig.pomiarInterwal*1000);
			}
			else if (page == 1 && menuitem == 2) 
			{
				// Auto/Manual
				page=2;
			}
			else if(page == 1 && menuitem == 3)	
			{
				// min value
				// set sensorMin
				page=2;
			}
			
			else if(page == 1 && menuitem == 4)	
			{
				// max value
				// set sensorMax
				page=2;
			}

			else if(page == 1 && menuitem == 5)	
			{
				// co ile min pomiar
				page=2;
			}	
			
			else if(page==2)
			{
				// zapisujemy parametry i wracamy do głównego menu
				switch(menuitem)
				{
					case 2:	// ustawienie Auto/Manual
						EEPROM.write(EEPROM_trybPracy, konfig.trybPracy);//saveState(EEPROM_trybPracy, konfig.trybPracy);
						break;
					
					case 3:	// ustawienie sensorMin
						konfig.sensorMin = selectedValue;
						EEPROM.write(EEPROM_sensorMin, konfig.sensorMin);//saveState(EEPROM_sensorMin, konfig.sensorMin);
						break;
					
					case 4:	// ustawienie sensorMax
						konfig.sensorMax = konfig.sensorMin - selectedValue;
						zbiornikMax	= selectedValue;
						EEPROM.write(EEPROM_sensorMax, konfig.sensorMax);//saveState(EEPROM_sensorMax, konfig.sensorMax);
						/* ################## RADIO
						
						send(msg_max.set(zbiornikMax));
						
						################## RADIO */
						break;
					
					case 5:	// ustawienie pomiarInterwal
						konfig.pomiarInterwal=selectedValue;
						EEPROM.write(EEPROM_pomiarInterwal, selectedValue);//saveState(EEPROM_pomiarInterwal, selectedValue);
				}
				page=1;				
			}
		}
	}	// koniec obsługi funkcji po naciśnięciu przycisku Rottary

	switch(konfig.wrkMode)
	{
		case 0:
			// tryb pracy POMIAR
			if(zbiornikPoziom>zbiornikMax)
			{
				// wchodzimy gdy został przekroczony poziom do wypompowania
				// tryb pracy POMIAR / przekroczenie poziomu
				if(konfig.trybPracy)
				{
					// wchodzimy gdy został przekroczony poziom do wypompowania i jesteśmy w trybie AUTO
					// tryb pracy POMIAR / przekroczenie poziomu  / AUTO pompa
					// wrkMode 0,3 realizują histereze pompy dla trybu Auto
					konfig.wrkMode=3;
					digitalWrite(PUMP_PIN, HIGH);
					timerPomiar.setInterval(250);
					autoStop=0;
					/* ################## RADIO
					
					send(msg_pump.set(true));
					
					################## RADIO */
				}	
				else
					// tryb pracy POMIAR / przekroczenie poziomu  / MANUAL pompa
					EkranPomiar(true, false, false);							
			}
			else
				EkranPomiar(false, false, false);
			
			/*################## RADIO
			
			if(zbiornikPoziom_l!=zbiornikPoziom)
			{
				zbiornikPoziom_l=zbiornikPoziom;
				
				send(msg_poziom.set(zbiornikPoziom));
				
			}
			################## RADIO */
			break;
	 	
		case 1:
			selectedValue = sensorPoziom;
			EkranMenu();
			break;
		 
		case 2:
			EkranStop();
			break;
		 		
		case 3:
			// wchodzimy gdy został przekroczony poziom do wypompowania i wypompowujemy wodę
			// wrkMode 0,3 realizują histereze pompy dla trybu Auto
			if(zbiornikPoziom<=0 || autoStop>=120)
			{
				// wchodzimy jeżeli woda została wypompowana bo zbiornikPoziom<=0
				// lub zadziałało zabezpiecznie autoStop>=120(przekroczono czas pompowania ustawiony na 30s) tj. woda nie została wypompowana w tym czasie
				if(autoStop>=120)
				{
					// skoro woda nie została wypompowana w czasie 30s to jest jakaś awaria lub za dużo wody i nalezy zatrzymać proces
					// wchodząc w tryb STOP
					konfig.wrkMode=2;
					digitalWrite(PUMP_PIN, LOW);
					timerPomiar.disable();
					EEPROM.write(EEPROM_wrkMode, konfig.wrkMode);//saveState(EEPROM_wrkMode, konfig.wrkMode);
				}
				else
				{
					// wchodzimy jeżeli woda została wypompowana bo zbiornikPoziom<=0 w zadanym czasie bezpieczeństwa
					konfig.wrkMode=0;				
					digitalWrite(PUMP_PIN, LOW);
					timerPomiar.setInterval(konfig.pomiarInterwal*1000);
				}
				
				autoStop=0;
				
				/* ################## RADIO
				
				send(msg_pump.set(false));
				
				################## RADIO */
			}
			// sprawdzenia w trybie MANUAL
			if(zbiornikPoziom>zbiornikMax)
				// pokaż OVERFLOW, pompa włączona
				EkranPomiar(true, false, true);			
			else
				// pompa włączona
				EkranPomiar(false, false, true);
			break;
		
		case 4:
				EkranMenu();
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

void pomiarIsr()
{
	blinkPomiar=true;
	
	if(konfig.wrkMode==3)
		autoStop=autoStop+1;
		
	sensorPoziom = 	sonar.convert_cm(sonar.ping_median(3));	//sonar.ping_cm();

	if( konfig.sensorMin < sensorPoziom)
		zbiornikPoziom = 0;
	else
		zbiornikPoziom = konfig.sensorMin - sensorPoziom;
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
		up = true;
		delay(150);
	}
	else if (value/2 < last) 
	{
		last = value/2;
		down = true;
		delay(150);
	}
}

// -------------------------------------------------
// Keyboard
// -------------------------------------------------
//

void onKeyboard()
{
	expander.checkForInterrupt();
}

void onKeyStop()
{
	if(konfig.wrkMode!=2)
	{
		// wejście do ekranu STOP
		
		/* ################## RADIO	
		
		send(msg_stop.set(true));
		send(msg_pump.set(false));
		
		################## RADIO */
  
		digitalWrite(PUMP_PIN, LOW);
		konfig.wrkMode=2;
		timerPomiar.disable();
		EEPROM.write(EEPROM_wrkMode, konfig.wrkMode);//saveState(EEPROM_wrkMode, konfig.wrkMode);
	}
	else
	{
		/*################## RADIO
		
		send(msg_stop.set(false));
		
		################## RADIO */
		konfig.wrkMode=0;
		timerPomiar.enable();
		timerPomiar.setInterval(konfig.pomiarInterwal*1000);
		EEPROM.write(EEPROM_wrkMode, konfig.wrkMode);//saveState(EEPROM_wrkMode, konfig.wrkMode);
	}
}

void onKeyManl()
{
	 switch(konfig.wrkMode)
	 {
		case 0:
			//przycisk MANL zostal nacisniety, wchodzimy pierwszy raz, czyli wlaczamy pompe
			if(zbiornikPoziom>0)
			{
				expander.attachInterrupt(BTN_MANL, onKeyManl, CHANGE);
				digitalWrite(PUMP_PIN, HIGH);
				konfig.wrkMode = 3;
				timerPomiar.setInterval(250);				
			}
			break;
		
		case 1:
			expander.attachInterrupt(BTN_MANL, onKeyManl, FALLING);
			digitalWrite(PUMP_PIN, LOW);
			timerPomiar.setInterval(konfig.pomiarInterwal*1000);
			konfig.wrkMode = 4;
			break;
		
		case 3:
			// przycisk MANL zostal zwolniony, czyli wyłączmy pompe
			expander.attachInterrupt(BTN_MANL, onKeyManl, FALLING);
			digitalWrite(PUMP_PIN, LOW);
			timerPomiar.setInterval(konfig.pomiarInterwal*1000);
			konfig.wrkMode = 0;
			break;
			
		case 4:
			if(page==2 && menuitem==3)
			{
				// wchodzimy jeżeli jesteśmy w menu, oraz w trybie ustawiania min		
				expander.attachInterrupt(BTN_MANL, onKeyManl, CHANGE);
				digitalWrite(PUMP_PIN, HIGH);
				konfig.wrkMode = 1;
				timerPomiar.setInterval(250);
			}
	 }
}

