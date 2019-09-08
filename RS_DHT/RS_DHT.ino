/**
 * Based on: https://www.mysensors.org/build/humidity
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0.0  : Krzysztof Furmaniak
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
 * | 1.0.0       | 0x010000FF  | first version
 * | 2.0.0       | 0x020000FF  | for 2 DHT sensors
 ** 
 * 
 * DESCRIPTION
 * This sketch implement a DHT-22 humidity/temperature sensors
 * with RS485 connection.
 * 
 */

/*  *******************************

// Enable transport type
// Enable RS485 transport layer
#define MY_RS485

// Define this to enables DE-pin management on defined pin
#define MY_RS485_DE_PIN 2

// Set RS485 baud rate to use
#define MY_RS485_BAUD_RATE 9600

//  *******************************/

// Enable debug prints
//#define MY_DEBUG
//#define MY_NODE_ID 23
//  *******************************

// Enable transport type
// Enable RS485 transport layer
#define MY_RS485

// Define this to enables DE-pin management on defined pin
#define MY_RS485_DE_PIN 2

// Set RS485 baud rate to use
#define MY_RS485_BAUD_RATE 9600

#include <SPI.h>
#include <MySensors.h>  
#include <DHT.h>

// Set this to the pin you connected the DHT's data pin to
#define DHT_DATA_PIN  3
#define DHT1_DATA_PIN 4

// Set this to the pin you connected the DHT's data pin to
#define DHT_ON_PIN 8

// Set this offset if the sensor has a permanent small offset to the real temperatures.
// In Celsius degrees (as measured by the device)
#define SENSOR_TEMP_OFFSET 0

// Sleep time between sensor updates (in milliseconds)
// Must be >1000ms for DHT22 and >2000ms for DHT11
static const uint64_t UPDATE_INTERVAL = 600000;

// Force sending an update of the temperature after n sensor reads, so a controller showing the
// timestamp of the last update doesn't show something like 3 hours in the unlikely case, that
// the value didn't change since;
// i.e. the sensor would force sending an update every UPDATE_INTERVAL*FORCE_UPDATE_N_READS [ms]
static const uint8_t FORCE_UPDATE_N_READS = 10;

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD1_ID_HUM 2
#define CHILD1_ID_TEMP 3

float lastTemp, lastTemp1;
float lastHum, lastHum1;
uint8_t nNoUpdatesTemp, nNoUpdatesTemp1;
uint8_t nNoUpdatesHum, nNoUpdatesHum1;
bool metric = true;

MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgHum1(CHILD1_ID_HUM, V_HUM);
MyMessage msgTemp1(CHILD1_ID_TEMP, V_TEMP);
DHT dht, dht1;

void presentation()  
{ 
  // Send the sketch version information to the gateway
  sendSketchInfo("TemperatureAndHumidity", "2.0");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_TEMP, S_TEMP);
  present(CHILD1_ID_HUM, S_HUM);
  present(CHILD1_ID_TEMP, S_TEMP);

  metric = getControllerConfig().isMetric;
}

void setup()
{
  //Serial.begin(115200);
  pinMode(DHT_ON_PIN, OUTPUT);
  digitalWrite(DHT_ON_PIN, LOW);
  delay(3000);
  
  dht.setup(DHT_DATA_PIN); // set data pin of DHT sensor
  dht1.setup(DHT1_DATA_PIN); // set data pin of DHT sensor
  
  if (UPDATE_INTERVAL <= dht.getMinimumSamplingPeriod()) 
  {
    Serial.println("Warning: UPDATE_INTERVAL is smaller than supported by the sensor!");
  }
  // Sleep for the time of the minimum sampling period to give the sensor time to power up
  // (otherwise, timeout errors might occure for the first reading)
  sleep(dht.getMinimumSamplingPeriod());  
}


void loop()      
{    
  // Force reading sensor, so it works also after sleep()
  dht.readSensor(true);
  dht1.readSensor(true);

  //******************* DHT TEMP **************************
  // Get temperature from DHT library
  float temperature = dht.getTemperature();

  if (isnan(temperature)) 
  {
    Serial.println("Failed reading temperature from DHT!");
  } 
  else 
    if (temperature != lastTemp || nNoUpdatesTemp == FORCE_UPDATE_N_READS) 
    {
      // Only send temperature if it changed since the last measurement or if we didn't send an update for n times
      lastTemp = temperature;

      // apply the offset before converting to something different than Celsius degrees
      temperature += SENSOR_TEMP_OFFSET;
      if (!metric) 
      {
        temperature = dht.toFahrenheit(temperature);
      }
      
      // Reset no updates counter
      nNoUpdatesTemp = 0;
      send(msgTemp.set(temperature, 1));
    
      #ifdef MY_DEBUG
        Serial.print("T: ");
        Serial.println(temperature);
      #endif
    } 
    else 
    {
      // Increase no update counter if the temperature stayed the same
      nNoUpdatesTemp++;
    }

  //******************* DHT1 TEMP **************************

  float temperature1 = dht1.getTemperature();

  if (isnan(temperature1)) 
  {
    Serial.println("Failed reading temperature from DHT!");
  } 
  else 
    if (temperature1 != lastTemp1 || nNoUpdatesTemp1 == FORCE_UPDATE_N_READS) 
    {
      // Only send temperature if it changed since the last measurement or if we didn't send an update for n times
      lastTemp1 = temperature1;

      // apply the offset before converting to something different than Celsius degrees
      temperature1 += SENSOR_TEMP_OFFSET;
      if (!metric) 
      {
        temperature1 = dht1.toFahrenheit(temperature1);
      }
      
      // Reset no updates counter
      nNoUpdatesTemp1 = 0;
      send(msgTemp1.set(temperature1, 1));
    
      #ifdef MY_DEBUG
        Serial.print("T: ");
        Serial.println(temperature1);
      #endif
    } 
    else 
    {
      // Increase no update counter if the temperature stayed the same
      nNoUpdatesTemp1++;
    }

  //******************* DHT HUM **************************

  // Get humidity from DHT library
  float humidity = dht.getHumidity();
  if (isnan(humidity)) 
  {
    Serial.println("Failed reading humidity from DHT");
  } 
  else 
    if (humidity != lastHum || nNoUpdatesHum == FORCE_UPDATE_N_READS) 
    {
      // Only send humidity if it changed since the last measurement or if we didn't send an update for n times
      lastHum = humidity;
      // Reset no updates counter
      nNoUpdatesHum = 0;
      send(msgHum.set(humidity, 1));
      
      #ifdef MY_DEBUG
        Serial.print("H: ");
        Serial.println(humidity);
      #endif
    } 
    else 
    {
      // Increase no update counter if the humidity stayed the same
      nNoUpdatesHum++;
    }

  //******************* DHT1 HUM **************************

  // Get humidity from DHT library
  float humidity1 = dht1.getHumidity();
  if (isnan(humidity1)) 
  {
    Serial.println("Failed reading humidity from DHT");
  } 
  else 
    if (humidity1 != lastHum1 || nNoUpdatesHum1 == FORCE_UPDATE_N_READS) 
    {
      // Only send humidity if it changed since the last measurement or if we didn't send an update for n times
      lastHum1 = humidity1;
      // Reset no updates counter
      nNoUpdatesHum1 = 0;
      send(msgHum1.set(humidity1, 1));
      
      #ifdef MY_DEBUG
        Serial.print("H: ");
        Serial.println(humidity1);
      #endif
    } 
    else 
    {
      // Increase no update counter if the humidity stayed the same
      nNoUpdatesHum1++;
    }

  // Sleep for a while to save energy
  digitalWrite(DHT_ON_PIN, HIGH);
  sleep(UPDATE_INTERVAL); 
  digitalWrite(DHT_ON_PIN, LOW);
  sleep(3000);
}
