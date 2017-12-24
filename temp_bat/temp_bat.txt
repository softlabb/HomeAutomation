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
 ****************************************************
 *
 * REVISION HISTORY
 * ver 1.0
 * Created by Krzysztof Furmaniak
 * Copyright (C) February 2017 Krzysztof Furmaniak
 * 
 */


// Enable debug prints to serial monitor
//#define MY_DEBUG 

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69


#define MY_RF24_CE_PIN 49
#define MY_RF24_CS_PIN 53

#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <MySensors.h>  

#define COMPARE_TEMP 1 // Send temperature only if changed? 1 = Yes 0 = No

#define ONE_WIRE_BUS 8 // Pin where dallase sensor is connected 
#define MAX_ATTACHED_DS18B20 3
unsigned long SLEEP_TIME = 300000; // Sleep time between reads (in milliseconds) 5min
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 
float lastTemperature[MAX_ATTACHED_DS18B20];
int numSensors=0;
bool receivedConfig = false;
bool metric = true;

int BATTERY_SENSE_PIN = A0;  // select the input pin for the battery sense point
int oldBatteryPcnt = 0;

// Initialize temperature message
MyMessage msg(0,V_TEMP);

void before()
{
  // Startup up the OneWire library
  sensors.begin();
}

void setup()  
{ 
      // use the 1.1 V internal reference
#if defined(__AVR_ATmega2560__)
    analogReference(INTERNAL1V1);
#else
    analogReference(INTERNAL);
#endif

  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);
}

void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("TEST Meter", "1.0");

  // Fetch the number of attached temperature sensors  
  numSensors = sensors.getDeviceCount();

  // Present all sensors to controller
  for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) {   
     present(i, S_TEMP);
  }
}

void loop()     
{     
      // get the battery Voltage
    int sensorValue = analogRead(BATTERY_SENSE_PIN);
    int batteryPcnt = sensorValue / 10;

	if(numSensors!=0)
	{      
	  // Fetch temperatures from Dallas sensors
	  sensors.requestTemperatures();

	  // query conversion time and sleep until conversion completed
	  int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
	  // sleep() call can be replaced by wait() call if node need to process incoming messages (or if node is repeater)
	  sleep(conversionTime);
	}
	
	// Read temperatures and send them to controller 
	for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) 
	{
		// Fetch and round temperature to one decimal
		float temperature = static_cast<float>(static_cast<int>((getControllerConfig().isMetric?sensors.getTempCByIndex(i):sensors.getTempFByIndex(i)) * 10.)) / 10.;

		// Only send data if temperature has changed and no error
		#if COMPARE_TEMP == 1
		if (lastTemperature[i] != temperature && temperature != -127.00 && temperature != 85.00) {
		#else
		if (temperature != -127.00 && temperature != 85.00) {
		#endif

			  // Send in the new temperature
			  send(msg.setSensor(i).set(temperature,1));
			  // Save new temperatures for next compare
			  lastTemperature[i]=temperature;
		}
		else
			sendHeartbeat();
	}
    
	if (oldBatteryPcnt != batteryPcnt) 
	{
        // Power up radio after sleep
        sendBatteryLevel(batteryPcnt);
        oldBatteryPcnt = batteryPcnt;
    }
    
	sleep(SLEEP_TIME);
}

