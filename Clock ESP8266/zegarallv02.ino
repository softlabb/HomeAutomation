
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <TimeLib.h>
//#include "Time.h"

#include <ButtonV2.h>

#include <EEPROM.h>

#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

//***************************************************************************************
// ZEGAR Firmware v0.2

//***************************************************************************************
// LED Matrix

int pinCS = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait   = 80; // In milliseconds
int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels

//***************************************************************************************
// BUTTON
// The circuit:
// * LED attached from pin 13 to ground
// * pushbutton attached from pin 2 to +5V
// * 10K resistor attached from pin 2 to ground

ButtonV2 buttonIntensity;
ButtonV2 buttonMode;
const byte ButtonPinIntensity = 2;
const byte ButtonPinMode = 3;

//***************************************************************************************
// Clock DS3231

int Godzina, oldGodzina;
int Minuta, oldMinuta;
int Sec, oldSec;
int Rok, oldRok;

short DN;   //Returns the number of day in the year
short WN;   //Returns the number of the week in the year

//***************************************************************************************
// EEPROM parametry
// Arduino Uno:        1kb EEPROM storage.
//  EEPROM.write(addr, val);
// value = EEPROM.read(address);
//

struct MyEEPROMstruc 
{
  byte DST;         //flaga czasu letniego(2)/zimowego (1)
  byte LEDintense;
  byte Mode;
  char Ver[5];
};

MyEEPROMstruc Config;

//
//***************************************************************************************
// ESP8266
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

byte packetBuffer[48]; //buffer to hold incoming and outgoing packets 

// A UDP instance to let us send and receive packets over UDP
WiFiEspUDP Udp;

//***************************************************************************************

boolean co2 = true;
byte Mode;
int  getNtp = 0; 

//***************************************************************************************

void DayWeekNumber(unsigned int y, unsigned int m, unsigned int d, unsigned int w)
{
	int days[]={0,31,59,90,120,151,181,212,243,273,304,334};    // Number of days at the beginning of the month in a not leap year.
	//Start to calculate the number of day
	if (m==1 || m==2)
	{
		DN = days[(m-1)]+d;                     //for any type of year, it calculate the number of days for January or february
	}                        // Now, try to calculate for the other months
	else
	if ((y % 4 == 0 && y % 100 != 0) ||  y % 400 == 0)
	{  //those are the conditions to have a leap year
		DN = days[(m-1)]+d+1;     // if leap year, calculate in the same way but increasing one day
	}
	else
	{                                //if not a leap year, calculate in the normal way, such as January or February
		DN = days[(m-1)]+d;
	}
	// Now start to calculate Week number
	if (w==0)
	{
		WN = (DN-7+10)/7;             //if it is sunday (time library returns 0)
	}
	else
	{
		WN = (DN-w+10)/7;        // for the other days of week
	}
}

//***************************************************************************************
/*
// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(char *ntpSrv)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, 48);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)

  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(ntpSrv, 123); //NTP requests are to port 123

  Udp.write(packetBuffer, 48);

  Udp.endPacket();
}
*/

void ntpSynchProc()
{  
  //sendNTPpacket("vega.cbk.poznan.pl"); // send an NTP packet to a time server, timeServer
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, 48);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)

    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket("vega.cbk.poznan.pl", 123); //NTP requests are to port 123
    Udp.write(packetBuffer, 48);
    Udp.endPacket();
	delay(10);
	
	if (Udp.parsePacket()) 
	{
		// We've received a packet, read the data from it into the buffer

		Udp.read(packetBuffer, 48);
    
		// the timestamp starts at byte 40 of the received packet and is four bytes,
		// or two words, long. First, esxtract the two words:

		unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
		unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
		// combine the four bytes (two words) into a long integer
		// this is NTP time (seconds since Jan 1 1900):
		unsigned long secsSince1900 = highWord << 16 | lowWord;

		// now convert NTP time into everyday time:
		// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
		const unsigned long seventyYears = 2208988800UL;
		// subtract seventy years:
		unsigned long epoch = secsSince1900 - seventyYears + Config.DST*3600;
		
		setTime(epoch);   
		
		//matrix.drawPixel(19,6,1);		// trzeci œwiadczy ¿e po³¹czono siê z WiFi i nastêpuje po³¹czenie z czasem
		//matrix.write(); // Send bitmap to display
	}
	else
	{
		matrix.fillScreen(LOW);
		matrix.setCursor(0,0);
		matrix.print("zero");
		matrix.write(); // Send bitmap to display
	}
}

//***************************************************************************************

void printDay()
 {
    char tape[60];
    //char dzien[10];

	DayWeekNumber(year(),month(),day(),weekday());

    strcat(tape,"Dzisiaj jest ");
    switch(weekday())
    {
      case 1:
        strcat(tape, "Niedziela");
        break;
      case 2:
        strcat(tape, "Poniedzialek");
        break;
      case 3:
        strcat(tape, "Wtorek");
        break;
      case 4:
        strcat(tape, "Sroda");
        break;
      case 5:
        strcat(tape, "Czwartek");
        break;
      case 6:
        strcat(tape, "Piatek");
        break;
      case 7:
        strcat(tape, "Sobota");
        break;
    }

    strcat(tape, ", ");	
    //strcat(tape, String(day(),DEC));
    strcat(tape, "/");
	//itoa(month(),buf,10);
    //strcat(tape, buf);
    strcat(tape, "/");
	//itoa(year(),buf,10);
    //strcat(tape, buf);
    //strcat(tape, ", ");
    
	//itoa(DN,buf,10);
	//strcat(tape, buf);
	
    strcat(tape, " dzien roku");
    
     for ( int i = 0 ; i < width * strlen(tape) + matrix.width() - 1 - spacer; i++ ) 
     {
      matrix.fillScreen(LOW);
  
      int letter = i / width;
      int x = (matrix.width() - 1) - i % width;
      int y = (matrix.height() - 8) / 2; // center the text vertically
  
      while ( x + width - spacer >= 0 && letter >= 0 ) 
      {
        if ( letter < strlen(tape) ) 
        {
          matrix.drawChar(x, y, tape[letter], HIGH, LOW, 1);
        }
  
        letter--;
        x -= width;
      }
  
      matrix.write(); // Send bitmap to display
  
      delay(40);
    } 
 }

//***************************************************************************************
//***************************************************************************************

void setup() 
{
  char ssid[] = "SKYNET3";					// your network SSID (name)
  char pass[] = "a1a1a1a1a1a1a1a1a1a1";     // your network password
  int status  = WL_IDLE_STATUS;				// the Wifi radio's status

  EEPROM.get(0, Config);
  matrix.setIntensity(0);
  Mode = Config.Mode;
    
/*
// tylko na czas programowanie
  Config.DST		= 2; // 1 tryb zimowy, 2 letni
  Config.LEDintense	= 0;
  Config.Mode		= 2;
  strcat(Config.Ver, "2.0");
  EEPROM.put(0, Config);
*/
  
  anime2(); 

  matrix.setCursor(0,0);
  matrix.print("NTP");
  matrix.drawPixel(16,6,1);	//pierwszy pixel przed uruchomieniem po³¹czenia z ESP
  matrix.write(); 
    
  Serial1.begin(19200);   // initialize serial for ESP module

  // initialize ESP module
  WiFi.init(&Serial1);
  
  // check for the presence of the shield
 
  if (WiFi.status() == WL_NO_SHIELD) 
  {
    matrix.fillScreen(LOW);
    matrix.setCursor(0,0);
    matrix.print("WFoff");
    matrix.write(); // Send bitmap to display
    while (true);
    // don't continue
  }
  
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) 
  {
    // Connect to WPA/WPA2 network
    matrix.drawPixel(17,6,1);	// drugi pixel œwiadczy o uruchomieniu po³¹czenia z WiFi
    matrix.write(); // Send bitmap to display
    status = WiFi.begin(ssid, pass);
  } 
  
  Udp.begin(2390);
  
  matrix.drawPixel(18,6,1);		// trzeci œwiadczy ¿e po³¹czono siê z WiFi i nastêpuje po³¹czenie z czasem
  matrix.write(); // Send bitmap to display
  
  ntpSynchProc();
    
  Godzina   = hour();
  Minuta    = minute();
  Sec       = second();
  Rok       = year();
  
  oldMinuta   = Minuta;
  oldGodzina  = Godzina;
  oldSec      = Sec;
  oldRok      = Rok; 

  pinMode(ButtonPinIntensity, INPUT_PULLUP);
  buttonIntensity.SetStateAndTime(LOW);

  pinMode(ButtonPinMode, INPUT_PULLUP);
  buttonMode.SetStateAndTime(LOW);
/*
  //clockDS3231.begin();
  // Set sketch compiling time
  //clockDS3231.setDateTime(__DATE__, __TIME__);
  // Lub recznie (YYYY, MM, DD, HH, II, SS
  //clockDS3231.setDateTime(2015, 12, 27, 16, 59, 30);
  
  //dt = clockDS3231.getDateTime();
    
  Godzina = hour();
  Minuta  = minute();
  Sec  = second();

  oldMinuta = Minuta;
  oldGodzina = Godzina;
  oldSec = Sec;

  //matrix.setIntensity(Config.LEDintense); // Use a value between 0 and 15 for brightness
  matrix.fillScreen(LOW);
  matrix.setCursor(3,0);
  matrix.print(Godzina, DEC);
  //matrix.drawPixel(16,2,1); // gorny kropek
  //matrix.drawPixel(16,4,1); // dolny kropek
  matrix.setCursor(18,0);
  matrix.print(Minuta, DEC);
  
  matrix.write(); // Send bitmap to display
*/
}

//***************************************************************************************

void loop() 
{
  time_t teraz;
  byte typeInt;
  byte typeMode;

  teraz = now(); // czytamy czas CPU    
  
  Godzina = hour(teraz);
  Minuta  = minute(teraz);
  Sec     = second(teraz);
  Rok     = year(teraz);

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

}


