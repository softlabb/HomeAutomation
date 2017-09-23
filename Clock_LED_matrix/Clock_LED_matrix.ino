//***************************************************************************************
// Clock LED matrix Firmware v1.0
//***************************************************************************************
//

// Enable debug prints to serial monitor
//#define MY_DEBUG
// Enable and select radio type attached
#define MY_RADIO_NRF24

#include <MySensors.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <TimeLib.h>
#include <ButtonV2.h>


/* LED Matrix
* ----------------------------------------
* ArduinoUNO	ATmega328	ArduinoMega	LED matrix
* ----------------------------------------
* GND			GND
* +5V			VCC
* A4			CLK
* A5			CS
* A6			DIN
*/

int pinCS = 8; // Attach CS to this pin, DIN to MOSI(11) and CLK to SCK(13) (cf http://arduino.cc/en/Reference/SPI )

int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

//int wait   = 80; // In milliseconds
int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels

//***************************************************************************************
// Clock DS3231

bool timeReceived = false;
unsigned long lastUpdate=0, lastRequest=0;

//***************************************************************************************

void before() {
	// Optional method - for initialisations that needs to take place before MySensors transport has been setup (eg: SPI devices).
	matrix.fillScreen(LOW);
}

//***************************************************************************************

void presentation() {
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("RTC Clock matrix", "1.0");
}

//***************************************************************************************

void setup() {
	matrix.setIntensity(0);
	
	anime2(); 

	// Request latest time from controller at startup
	requestTime();
   
  //pinMode(ButtonPinIntensity, INPUT_PULLUP);
  //buttonIntensity.SetStateAndTime(LOW);

  //pinMode(ButtonPinMode, INPUT_PULLUP);
  //buttonMode.SetStateAndTime(LOW);

}

//***************************************************************************************

// This is called when a new time value was received
void receiveTime(unsigned long controllerTime) {
	// Ok, set incoming time
	setTime(controllerTime);
	timeReceived = true;
}

//***************************************************************************************

void loop() 
{
	unsigned long now = millis();
  
	// If no time has been received yet, request it every 10 second from controller
	// When time has been received, request update every hour
	if ((!timeReceived && (now-lastRequest) > (10UL*1000UL))
		|| (timeReceived && (now-lastRequest) > (60UL*1000UL*60UL))) {
		// Request time from controller.
		requestTime();
		lastRequest = now;
	}
   
    // Update display every second
    if (now-lastUpdate > 1000) {
	    updateDisplay();
	    lastUpdate = now;
    }
/*	
  //################################################
  // przy zmianie godziny animacja
  if(Godzina != oldGodzina)
  {
    // przejscie z czasu letniego na zimowy
    if(weekday(teraz)==1 && month(teraz)==10 && day(teraz)>=25 && day(teraz)<=31 && Godzina==3 && Config.DST==2)
    {
      Config.DST=1;
      EEPROM.put(0,Config);  
      ntpSynchProc();
    }

    // przejscie z czasu zimowego na letni
    if(weekday(teraz)==1 && month(teraz)==3 && day(teraz)>=25 && day(teraz)<=31 && Godzina==2 && Config.DST==1)
    {
      Config.DST=2;
      EEPROM.put(0,Config);  
      ntpSynchProc();
    }

    if(Rok != oldRok)
    {
      NowyRok();
      oldRok=Rok;
    }
    
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

      //matrix.drawPixel(16,2,1); // gorny kropek
      //matrix.drawPixel(16,4,1); // dolny kropek
      
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
  
      delay(wait);
    }

    oldGodzina = Godzina;
    oldMinuta = Minuta;    
  }

  //################################################
  // przy zmianie minuty animacja
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
      
      //matrix.drawPixel(16,2,1); // gorny kropek
      //matrix.drawPixel(16,4,1); // dolny kropek
  
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
  
      delay(wait);
    }
    
    oldMinuta = Minuta;
  }
  // KONIEC animacji minuty
  //################################################

  //################################################
  // przy zmianie sec animacja

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
    
    switch(Mode)
    {
      case 1:
      {
        if(second(teraz)<30)
          matrix.drawPixel(1+second(teraz),7,1); // gorny kropek
        else
          matrix.drawPixel(60-second(teraz),7,1); // gorny kropek
        break;
      }
      case 2:
      {
        if(co2)
        {
          matrix.drawPixel(16,2,1); // gorny kropek
          matrix.drawPixel(16,4,1); // dolny kropek
          co2=false;
        }
        else
          co2=true;
        break;
      }
      case 3:
        matrix.drawPixel(15,5,1); // gorny kropek
        matrix.drawPixel(15,6,1); // dolny kropek      
        matrix.drawPixel(16,5,1); // gorny kropek
        matrix.drawPixel(16,6,1); // dolny kropek      
        break;

      case 4:
        matrix.drawPixel(16,6,1); // dolny kropek      
        break;
    }

    matrix.write(); // Send bitmap to display
    oldSec = Sec;
    getNtp+=1;
    if(getNtp>1800) // wywolywanie synchronizacji czasu co 30min=1800sec
      {
        ntpSynchProc();
        getNtp=1;
      }
  }
  // KONIEC animacji sec
  //################################################
  

  typeInt = buttonIntensity.CheckButton(ButtonPinIntensity); // current time and length of time to press the button as many times as you can ie. 1.5 seconds
  
  switch (typeInt)
  {
    case WAITING:
      break;
    case PRESSED:
      //Config.LEDintense+=1;
      //if(Config.LEDintense>7)
      //    Config.LEDintense=7;
      //matrix.setIntensity(Config.LEDintense);    
      break;
    case DOUBLE_PRESSED:
      //Config.LEDintense-=1;
      //if(Config.LEDintense<0)
      //  Config.LEDintense=0;
      //matrix.setIntensity(Config.LEDintense);    
      break;
    case MULTI_PRESSED:
      break;
    case HELD:
      matrix.fillScreen(LOW);
      matrix.setCursor(0,0);
      matrix.print("NTP");
      matrix.write(); // Send bitmap to display
      ntpSynchProc();
      break;
  }
 
  typeMode = buttonMode.CheckButton(ButtonPinMode); // current time and length of time to press the button as many times as you can ie. 1.5 seconds

  switch (typeMode)
  {
    case WAITING:
      break;
    case PRESSED:
      Mode=Mode+1;
      if(Mode>4)
        Mode=1;
      Config.Mode=Mode;
      EEPROM.put(0, Config);
      break;
    case DOUBLE_PRESSED:
      printDay();
      break;
    case MULTI_PRESSED:
      anime1();
      break;
    case HELD:
      anime3();
      break;
  }
*/
}

void updateDisplay(){
	matrix.fillScreen(LOW);
    
	// hour
	if(hour()<10)
		matrix.setCursor(9,0);
	else
		matrix.setCursor(3,0);
	
	matrix.print(hour());

	// minute
	matrix.setCursor(18,0);
	if(minute()<10)
	    matrix.print("0");
	
	matrix.print(minute());

	// second
	if(second()<30)
		matrix.drawPixel(1+second(),7,1); 
	else
		matrix.drawPixel(60-second(),7,1); 

	matrix.write(); // Send bitmap to display		
}

void NowyRok()
{
	String tape = "Szczesliwego NOWEGO ";

	tape += year();
	tape += " ROKU !!!";
	
	matrix.setIntensity(3);
	matrix.fillScreen(LOW);
	
	for(int j=0;j<4;j++)
	{
		for(int i=0;i<4;i++)
		{
			matrix.drawRect(i,i,31-2*i,8-2*i,1);
			matrix.write(); // Send bitmap to display
			delay(150);
		}
		matrix.fillScreen(LOW);
	}

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
		
		delay(40);
	}

	for(int j=0;j<4;j++)
	{
		for(int i=0;i<4;i++)
		{
			matrix.drawRect(i,i,31-2*i,8-2*i,1);
			matrix.write(); // Send bitmap to display
			delay(150);
		}
		matrix.fillScreen(LOW);
	}

	matrix.setIntensity(0);
	matrix.fillScreen(LOW);
	matrix.write(); // Send bitmap to display
}

void anime1()
{
	for ( int x = 0; x < matrix.width() - 1; x++ )
	{
		matrix.fillScreen(LOW);
		matrix.drawLine(x, 0, matrix.width() - 1 - x, matrix.height() - 1, HIGH);
		matrix.write(); // Send bitmap to display
		delay(80);
	}
}

void anime2()
{
	matrix.setIntensity(0); // Use a value between 0 and 15 for brightnes
	
	for ( int x = 0; x < matrix.width() - 1; x++ )
	{
		matrix.fillScreen(LOW);
		matrix.drawLine(x, 0, x, matrix.height() - 1, HIGH);
		matrix.write(); // Send bitmap to display
		delay(50);
	}
	for ( int x = matrix.width() - 1; x > 0; x-- )
	{
		matrix.fillScreen(LOW);
		matrix.drawLine(x, 0, x, matrix.height() - 1, HIGH);
		matrix.write(); // Send bitmap to display
		delay(25);
	}

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

	matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
	matrix.fillScreen(LOW);
	matrix.write(); // Send bitmap to display
}

void anime3()
{
	matrix.fillScreen(LOW);
	matrix.write(); // Send bitmap to display
	delay(100);
	matrix.fillScreen(HIGH);
	matrix.write(); // Send bitmap to display
	delay(100);
	matrix.fillScreen(LOW);
	matrix.write(); // Send bitmap to display
	delay(100);
	matrix.fillScreen(HIGH);
	matrix.write(); // Send bitmap to display
	delay(100);

	matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
	matrix.fillScreen(LOW);
}

