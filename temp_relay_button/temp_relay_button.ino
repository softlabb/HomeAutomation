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

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <MySensors.h>

#define COMPARE_TEMP 1 // Send temperature only if changed? 1 = Yes 0 = No
#define ONE_WIRE_BUS 8 // Pin where dallase sensor is connected 
#define MAX_ATTACHED_DS18B20 3
unsigned long SLEEP_TIME = 300000; // Sleep time between reads (in milliseconds) 5min

#define RELAY_PIN  3    // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define CHILD_ID 1   // Id of the sensor child
#define RELAY_ON 1    // GPIO value to write to turn on attached relay
#define RELAY_OFF 0   // GPIO value to write to turn off attached relay
#define LED_PIN 4         // GPIO value to write to turn off attached relay
#define BUTTON_PIN 5  // Arduino Digital I/O pin number for button

OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 
float lastTemperature[MAX_ATTACHED_DS18B20];
int numSensors=0;
bool receivedConfig = false;
bool metric = true;

int oldValue=0;
bool state;

unsigned long previousMillis = 0;
const long interval = 300000; //ms, 5min

// Initialize temperature message
MyMessage msg_temp(0,V_TEMP);
// Initialize relay message
MyMessage msg(CHILD_ID, V_LIGHT);

void before()
{
  // Startup up the OneWire library
  sensors.begin();
}

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  
    // Setup the button
  pinMode(BUTTON_PIN,INPUT);
  // Activate internal pull-up
  digitalWrite(BUTTON_PIN,HIGH);

   // Make sure relays are off when starting up
  digitalWrite(RELAY_PIN, RELAY_OFF);
  // Then set relay pins in output mode
  pinMode(RELAY_PIN, OUTPUT);   

  // Set relay to last known state (using eeprom storage) 
  state = loadState(CHILD_ID);
  digitalWrite(RELAY_PIN, state?RELAY_ON:RELAY_OFF);
  
  digitalWrite(LED_PIN, state?RELAY_ON:RELAY_OFF);

  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Relay & Button & Temp", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(0, S_TEMP);
  present(CHILD_ID, S_LIGHT);
}


void loop()
{ 
  int value = digitalRead(BUTTON_PIN);
  
  if (value != oldValue && value==0) 
   {
      send(msg.set(state?false:true), true); // Send new state and request ack back
   }
  
  oldValue = value;

  unsigned long currentMillis = millis();

   if (currentMillis - previousMillis >= interval) 
   {    
     // save the last time you blinked the LED
        previousMillis = currentMillis;
        
      // Fetch temperatures from Dallas sensors
      sensors.requestTemperatures();

      // query conversion time and sleep until conversion completed
      int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
      // sleep() call can be replaced by wait() call if node need to process incoming messages (or if node is repeater)
      sleep(conversionTime);
       // Fetch and round temperature to one decimal
      float temperature = static_cast<float>(static_cast<int>((getControllerConfig().isMetric?sensors.getTempCByIndex(0):sensors.getTempFByIndex(0)) * 10.)) / 10.;
    
      // Only send data if temperature has changed and no error
      #if COMPARE_TEMP == 1
        if (lastTemperature[0] != temperature && temperature != -127.00 && temperature != 85.00) 
          {
      #else
        if (temperature != -127.00 && temperature != 85.00) 
          {
      #endif

            // Send in the new temperature
            send(msg_temp.setSensor(0).set(temperature,1));
            // Save new temperatures for next compare
            lastTemperature[0]=temperature;
         }
        else
          sendHeartbeat();  

 
   }
}

void receive(const MyMessage &message)
{
   // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
     //This is an ack from gateway
  }

  if (message.type == V_LIGHT) {
     // Change relay state
     state = message.getBool();
     digitalWrite(RELAY_PIN, state?RELAY_ON:RELAY_OFF);
     // Store state in eeprom
     saveState(CHILD_ID, state);

    digitalWrite(LED_PIN, state?RELAY_ON:RELAY_OFF);
   } 
}
