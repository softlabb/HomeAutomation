
// Enable debug prints to serial monitor
// pamiętać o kompilowaniu na zwykłe ARDUINO
// hardware działa z zewnętrznym kwarcem 16Mhz
//#define MY_DEBUG 
//#ifdef F_CPU
//#undef F_CPU
//#define F_CPU 8000000L
//#endif

// Enable and select radio type attached
#define MY_RADIO_NRF24
#define MY_RF24_PA_LEVEL  RF24_PA_MAX //RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH or (default) RF24_PA_MAX
#define MY_REPEATER_FEATURE

//#include <SPI.h>
#include <MySensors.h>  
#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS 8 // Pin where dallase sensor is connected 

unsigned long SLEEP_TIME = 600000; // Sleep time between reads (in milliseconds) = 10min
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 
// Force sending an update of the temperature after n sensor reads, so a controller showing the
// timestamp of the last update doesn't show something like 3 hours in the unlikely case, that
// the value didn't change since;
// i.e. the sensor would force sending an update every UPDATE_INTERVAL*FORCE_UPDATE_N_READS [ms] = co daje 30min
uint8_t FORCE_UPDATE_N_READS = 3;

float lastTemperature;
int numSensors=0;
uint8_t nNoUpdatesTemp=1;



// Initialize temperature message
MyMessage msg(0,V_TEMP);

// **************************************************************************
void before()
{
  // Startup up the OneWire library
  sensors.begin();
}

// **************************************************************************
void setup()  
{ 
  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);
}

// **************************************************************************
void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("RepeaterTemp Sensor", "1.0");

  present(0, S_TEMP);
}

// **************************************************************************
void loop()     
{     
  // Fetch temperatures from Dallas sensors
  sensors.requestTemperatures();

  // query conversion time and sleep until conversion completed
  int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
  // sleep() call can be replaced by wait() call if node need to process incoming messages (or if node is repeater)
  sleep(conversionTime);

  // Fetch and round temperature to one decimal
  float temperature = static_cast<float>(static_cast<int>((getControllerConfig().isMetric?sensors.getTempCByIndex(0):sensors.getTempFByIndex(0)) * 10.)) / 10.;

  // Only send data if temperature has changed and no error
  if (lastTemperature != temperature && temperature != -127.00 && temperature != 85.00 || nNoUpdatesTemp >= FORCE_UPDATE_N_READS) 
  {
    // Reset no updates counter
    nNoUpdatesTemp = 1;
      
    // Send in the new temperature
    send(msg.setSensor(0).set(temperature,1));
      
    // Save new temperatures for next compare
    lastTemperature = temperature;
  }
  else 
  {
    // Increase no update counter if the humidity stayed the same
    nNoUpdatesTemp++;
  }
  
  wait(SLEEP_TIME);
}
