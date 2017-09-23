// Enable debug prints to serial monitor
//#define MY_DEBUG 

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

// Enable repeater functionality for this node
// #define MY_REPEATER_FEATURE

#define MY_RF24_CE_PIN 49
#define MY_RF24_CS_PIN 53

#include <MySensors.h>  
#include <SPI.h>
#include <TimeLib.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

bool timeReceived = false;
unsigned long lastUpdate=0, lastRequest=0;

void before()
{
    // Optional method - for initialisations that needs to take place before MySensors transport has been setup (eg: SPI devices).
}

void setup()  
{  
	//Serial.begin(9600);
  // the function to get the time from the RTC
  //setSyncProvider(RTC.get);  

 // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
 display.begin(SSD1306_SWITCHCAPVCC);
 display.clearDisplay();
 display.drawPixel(10, 10, WHITE);
 display.display();
 delay(2000);
 display.clearDisplay();
   
// text display tests
//display.setTextSize(1);
//display.setTextColor(WHITE);

  // Request latest time from controller at startup
  requestTime();  
  
  // initialize the lcd for 16 chars 2 lines and turn on backlight
  //lcd.begin(16,2); 
}

void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("RTC Clock", "1.0");
}

// This is called when a new time value was received
void receiveTime(unsigned long controllerTime) {
  // Ok, set incoming time 
  //Serial.print("Time value received: ");
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(controllerTime);
  display.display();
  //RTC.set(controllerTime); // this sets the RTC to the time from controller - which we do want periodically
  setTime(controllerTime);
  timeReceived = true;
}
 
void loop()     
{     
  unsigned long now = millis();
  
  // If no time has been received yet, request it every 10 second from controller
  // When time has been received, request update every hour
  if ((!timeReceived && (now-lastRequest) > (10UL*1000UL))
    || (timeReceived && (now-lastRequest) > (60UL*1000UL*60UL))) {
    // Request time from controller. 
    //Serial.println("requesting time");
    requestTime();  
    lastRequest = now;
  }
  
  // Update display every second
  if (now-lastUpdate > 1000) {
    updateDisplay();  
    lastUpdate = now;
  }
}

void receive(const MyMessage &message)
{

}

void updateDisplay(){
  
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0,0);

	display.print(day());
	display.print("/");
	display.print(month());
	display.print("/");
	display.print(year());

	display.print(" ");
	printDigits(hour());
	display.print(":");
	printDigits(minute());
	display.print(":");
	printDigits(second());
	
	display.display();
}


void printDigits(int digits){
  if(digits < 10)
    display.print('0');
  display.print(digits);
}
