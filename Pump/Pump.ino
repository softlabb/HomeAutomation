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

// Enable debug prints to serial monitor
//#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

#include <MySensors.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <PCF8574.h>
#include <Wire.h>
#include <HCSR04.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <ACS7xx_Allegro.h>


#define RELAY_1  4        // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 1        // GPIO value to write to turn on attached relay
#define RELAY_OFF 0       // GPIO value to write to turn off attached relay
#define SENSOR_RELAY  1
#define SENSOR_SR04   2
#define SENSOR_ACS    3

PCF8574 expander;

// example for ACS712 : bidir = true, A0 is the sensor pin, 5.0 is the volatge board, 0.1 is the sensibility of the chip
ACS7XX_ALLEGRO currentSensor(true, 0, 5.0, 0.1);


int triggerPin = 0;
int echoPin = 1;
UltraSonicDistanceSensor distanceSensor(triggerPin, echoPin);

// 
MyMessage msg_acs(SENSOR_ACS, V_CURRENT);
MyMessage msg_dis(SENSOR_SR04, V_DISTANCE);

void before()
{
  for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS; sensor++, pin++) {
    // Then set relay pins in output mode
    pinMode(pin, OUTPUT);
    // Set relay to last known state (using eeprom storage)
    digitalWrite(pin, loadState(sensor)?RELAY_ON:RELAY_OFF);
  }
}

void setup()
{
	expander.begin(0x20);
	expander.pinMode(0, OUTPUT);  // LED RED    - STOP
	expander.pinMode(1, OUTPUT);  // LED YELLOW - OVER
	expander.pinMode(2, OUTPUT);  // LED GREEN  - Pump
	expander.pinMode(3, OUTPUT);  // LED WHITE  - Measure
	expander.pinMode(RELAY_1, OUTPUT);  // RelaySensor
	expander.pinMode(5, INPUT);   // BUTTON - MANL
	expander.pinMode(6, INPUT);   // BUTTON - STOP

	expander.clear();
  //expander.digitalWrite(0, LOW);
  //expander.digitalWrite(0, LOW);
  //expander.digitalWrite(4, HIGH);
  //expander.blink(4, 10, 500); //(4-pin, 10-ilosc mrugniec, 500-ms każdy stan)

  currentSensor.begin();
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Pump", "1.0");

  present(SENSOR_RELAY, S_BINARY);
  present(SENSOR_SR04, S_DISTANCE);
  present(SENSOR_ACS, S_MULTIMETER);
}


void loop()
{
	double currentNow;
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
}

void receive(const MyMessage &message)
{
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_STATUS) 
  {
    // Change relay state
    expander.digitalWrite(message.sensor-1+RELAY_1, message.getBool()?RELAY_ON:RELAY_OFF);
    expander.digitalWrite(2, message.getBool()?RELAY_ON:RELAY_OFF);
	// Store state in eeprom
    saveState(message.sensor, message.getBool());
    // Write some debug info
    //Serial.print("Incoming change for sensor:");
    //Serial.print(message.sensor);
    //Serial.print(", New status: ");
    //Serial.println(message.getBool());
  }
}
