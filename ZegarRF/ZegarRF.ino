
#define MY_RADIO_RF24
#define MY_RF24_PA_LEVEL                            RF24_PA_MAX
//#define MY_TRANSPORT_WAIT_READY_MS                15000 //Timeout in ms until transport is ready during startup, set to 0 for no timeout.
//#define MY_SLEEP_TRANSPORT_RECONNECT_TIMEOUT_MS   30000 //Timeout (in ms) to re-establish link if node is put to sleep and transport is not ready.
#define MY_REPEATER_FEATURE
#define TEMP_TIME                                 10    // time between reads (in min)
#define ONE_WIRE_BUS                              2     // Pin PD2 where dallase sensor is connected 

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <TimeLib.h>
#include <Wire.h>
#include "DS3231.h"
#include <DallasTemperature.h>
#include <OneWire.h>

#include <MySensors.h>

RTClib RTC;
DS3231 Clock;

OneWire oneWire(ONE_WIRE_BUS);        // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);  // Pass the oneWire reference to Dallas Temperature. 

int readMinuta= 10;
int numSensors= 0;

int pinCS = 8; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays  = 4;
int numberOfVerticalDisplays    = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait_time = 40; // In milliseconds
int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels
bool timeReceived = false;

// Clock DS3231

int Godzina, oldGodzina=0;
int Minuta, oldMinuta=0;
int Sec, oldSec=0;
int Rok, oldRok;

// Initialize temperature message
MyMessage msg(1, V_TEMP);

//*************************************************************************************************************

void intro()
{
    matrix.setIntensity(0); // Use a value between 0 and 15 for brightnes
  
  for ( int x = 0; x < matrix.width() - 1; x++ ) 
  {
    matrix.fillScreen(LOW);
    matrix.drawLine(x, 0, x, matrix.height() - 1, HIGH);
    matrix.write(); // Send bitmap to display
    wait(50);
  }
  for ( int x = matrix.width() - 1; x > 0; x-- ) 
  {
    matrix.fillScreen(LOW);
    matrix.drawLine(x, 0, x, matrix.height() - 1, HIGH);
    matrix.write(); // Send bitmap to display
    wait(25);
  }

 /*
  matrix.setIntensity(5); // Use a value between 0 and 15 for brightnes
 
  matrix.fillScreen(LOW);
  matrix.write(); // Send bitmap to display
  delay(200);
  matrix.fillScreen(HIGH);
  matrix.write(); // Send bitmap to display
  delay(100);
  matrix.fillScreen(LOW);
  matrix.write(); // Send bitmap to display
  delay(100);
  matrix.fillScreen(HIGH);
  matrix.write(); // Send bitmap to display
  delay(200);
*/

  matrix.fillScreen(LOW);
  matrix.write(); // Send bitmap to display
}

void anime3()
{
 matrix.fillScreen(LOW);
 matrix.write(); // Send bitmap to display
 wait(100);
 matrix.fillScreen(HIGH);
 matrix.write(); // Send bitmap to display
 wait(100);
 matrix.fillScreen(LOW);
 matrix.write(); // Send bitmap to display
 wait(100);
 matrix.fillScreen(HIGH);
 matrix.write(); // Send bitmap to display
 wait(100);

 matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
 matrix.fillScreen(LOW);  
}

//*************************************************************************************************************

void setupMatrix()
{
     matrix.setIntensity(0); // Use a value between 0 and 15 for brightness

// Adjust to your own needs
//  matrix.setPosition(0, 0, 0); // The first display is at <0, 0>
//  matrix.setPosition(1, 1, 0); // The second display is at <1, 0>
//  matrix.setPosition(2, 2, 0); // The third display is at <2, 0>
//  matrix.setPosition(3, 3, 0); // And the last display is at <3, 0>
//  ...
    matrix.setRotation(0, 3);    // The first display is position upside down
    matrix.setRotation(1, 3);    // The same hold for the last display
    matrix.setRotation(2, 3);    // The same hold for the last display
    matrix.setRotation(3, 3);    // The same hold for the last display

    matrix.fillScreen(LOW);
    matrix.write(); // Send bitmap to display
}

//*************************************************************************************************************

void before()
{
  // Request latest time from controller at startup
  Wire.begin();
  setupMatrix(); 

  // Startup up the OneWire library
  sensors.begin();
}

//*************************************************************************************************************


void setup() 
{
  // the function to get the time from the RTC
  //setSyncProvider(RTC.get);  

  intro(); 
  
  requestTime();

  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);
}


//*************************************************************************************************************

void presentation()
{
  //Send the sensor node sketch version information to the gateway
  sendSketchInfo("RTC Clock Temp", "3.1");
  present( 0, S_INFO );

  // Fetch the number of attached temperature sensors  
  numSensors = sensors.getDeviceCount();
    // Present all sensors to controller
  if(numSensors!=0)
  {
    present(1, S_TEMP);
  }

}
//*************************************************************************************************************

void receive(const MyMessage &message)
{
    String tape = message.data;
    int opuznienie = 20; // In milliseconds
    // We only expect one type of message from controller. But we better check anyway.
    
    if (message.type==V_TEXT) 
    {
      //message.getString(buff);
      
      for ( int a = 0 ; a <3 ; a++)
        for ( int i = 0 ; i < width * tape.length() + matrix.width() - 1 - spacer; i++ ) 
        {
          matrix.fillScreen(LOW);
          int letter = i / width;
          int x = (matrix.width() - 1) - i % width;
          int y = (matrix.height() - 8) / 2; // center the text vertically

          while ( x + width - spacer >= 0 && letter >= 0 ) 
          {
            if ( letter < tape.length() ) 
            {
              matrix.drawChar(x, y, tape[letter], HIGH, LOW, 1);
            }

            letter--;
            x -= width;
          }

          matrix.write(); // Send bitmap to display
          wait(opuznienie);
        }
     }
}

void receiveTime(unsigned long controllerTime) {
  // Ok, set incoming time 
  //RTC.set(controllerTime); // this sets the RTC to the time from controller - which we do want periodically
  setTime(controllerTime);
  
  Clock.setClockMode(false);  // set to 24h
  Clock.setYear(year(controllerTime));
  Clock.setMonth(month(controllerTime));
  Clock.setDate(day(controllerTime));
  Clock.setDoW(weekday(controllerTime));
  Clock.setHour(hour(controllerTime));
  Clock.setMinute(minute(controllerTime));
  Clock.setSecond(second(controllerTime));
    
  timeReceived = true;
}

//*************************************************************************************************************

void loop() 
{
  //time_t teraz;
  
  DateTime now = RTC.now();
      
  Godzina = now.hour();
  Minuta  = now.minute();
  Sec     = now.second();

  //teraz = now(); // czytamy czas CPU    

  //Godzina = hour(teraz);
  //Minuta  = minute(teraz);
  //Sec     = second(teraz);


  //################################################
  // START przy zmianie godziny animacja
  if(Godzina != oldGodzina)
  {
    /*
    if(Rok != oldRok)
    {
      NowyRok();
      oldRok=Rok;
    }
    */
    
    anime3();
    
    for ( int i = 0 ; i < 8; i++ ) 
    {
      matrix.fillScreen(LOW);

      //nowa godzina
      if(Godzina<10)
        matrix.setCursor(9,i-8);      
      else
        matrix.setCursor(3,i-8);
      matrix.print(Godzina);

      //stara godzina
      if(Godzina<10)
        matrix.setCursor(9,i-8);      
      else
        matrix.setCursor(3,i);
      matrix.print(oldGodzina);

      
      // nowa minuta
      matrix.setCursor(18,i-8);
      if(Minuta<10)
        matrix.print("0");
      matrix.print(Minuta);

      // stara minuta
      matrix.setCursor(18,i);
      if(Minuta<10)
        matrix.print("0");
      matrix.print(oldMinuta);

      matrix.write(); // Send bitmap to display
  
      wait(wait_time);//delay(wait);
    }

    oldGodzina = Godzina;
    oldMinuta = Minuta;    
  }
  
  // KONIEC przy zmianie godziny animacja
  //################################################

  //################################################
  // START przy zmianie minuty animacja
  if(Minuta != oldMinuta)
  {
    for ( int i = 0 ; i < 8; i++ ) 
    {
      matrix.fillScreen(LOW);

      if(Godzina<10)
        matrix.setCursor(9,0);      
      else
        matrix.setCursor(3,0);
      matrix.print(Godzina);
       
      // nowa minuta
      matrix.setCursor(18,i-8);
      if(Minuta<10)
        matrix.print("0");
      matrix.print(Minuta);

      // stara minuta
      matrix.setCursor(18,i);
      if(Minuta<10)
        matrix.print("0");
      matrix.print(oldMinuta);

      matrix.write(); // Send bitmap to display
  
      wait(wait_time);//delay(wait);
    }
    
    oldMinuta = Minuta;

    if(numSensors!=0)
    {
      if(readMinuta>=TEMP_TIME)
      {
        //wchodzimy jeżeli upłynęło minuta
        // Fetch temperatures from Dallas sensors
        sensors.requestTemperatures();

        // query conversion time and sleep until conversion completed
        int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
        // sleep() call can be replaced by wait() call if node need to process incoming messages (or if node is repeater)
        wait(conversionTime);
        
        // Fetch and round temperature to one decimal
        float temperature = static_cast<float>(static_cast<int>((getControllerConfig().isMetric?sensors.getTempCByIndex(0):sensors.getTempFByIndex(0)) * 10.)) / 10.;
        if (temperature != -127.00 && temperature != 85.00) 
        {
          // Send in the new temperature
          send(msg.setSensor(1).set(temperature,1));
          // Save new temperatures for next compare
        }
      
        readMinuta=1;
      }
      else
        readMinuta++;
    }
  }
  // KONIEC animacji minuty
  //################################################

  //################################################
  // STARTprzy zmianie sec animacja

  if(Sec != oldSec)
  {
    matrix.fillScreen(LOW);
    
    // godzina
    if(Godzina<10)
      matrix.setCursor(9,0);      
    else
      matrix.setCursor(3,0);
    matrix.print(Godzina);

    // minuta
    matrix.setCursor(18,0);
    if(Minuta<10)
      matrix.print("0");
    matrix.print(Minuta);

  
    if(now.second()<30)
      matrix.drawPixel(1+now.second(),7,1); // gorny kropek
    else
      matrix.drawPixel(60-now.second(),7,1); // gorny kropek
   /*
    if(second(teraz)<30)
      matrix.drawPixel(1+second(teraz),7,1); // gorny kropek
    else
      matrix.drawPixel(60-second(teraz),7,1); // gorny kropek
    */
          
    matrix.write(); // Send bitmap to display
    oldSec = now.second();
    //oldSec = second(teraz);
    
    
    /*
    getNtp+=1;
    if(getNtp>1800) // wywolywanie synchronizacji czasu co 30min=1800sec
      {
        ntpSynchProc();
        getNtp=1;
      }
      */
  }
  // KONIEC animacji sec
  //################################################

/*    
  for ( int i = 0 ; i < width * tape.length() + matrix.width() - 1 - spacer; i++ ) {

    matrix.fillScreen(LOW);

    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically

    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < tape.length() ) {
        matrix.drawChar(x, y, tape[letter], HIGH, LOW, 1);
      }

      letter--;
      x -= width;
    }

    matrix.write(); // Send bitmap to display

    delay(wait);
  }
  */
}
