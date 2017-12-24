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
 * DESCRIPTION
 *
 */

// --------------------------------------------------------
// EKRANY
// --------------------------------------------------------
//
void EkranIntro()
{
	for ( int x = 0; x < matrix.width() - 1; x++ )
	  {
		matrix.fillScreen(LOW);
		matrix.drawLine(x, 0, matrix.width() - 1 - x, matrix.height() - 1, HIGH);
		matrix.write(); // Send bitmap to display
		delay(80);
	  }
}

void EkranStop()
{
  matrix.fillScreen(LOW);
  matrix.drawRect(0,0,16,8,1);
  matrix.drawLine(0,0,15,7,1);
  matrix.drawLine(0,7,15,0,1);
  matrix.write(); 
}

void EkranMenu()
{
	if(page==1)
	{
		switch(menuitem)
		{
			case 1:
				displayStringMenu("1. wyjscie z menu");
				break;
			case 2:
				displayStringMenu("2. tryb pracy");
				break;
			case 3:
				displayStringMenu("3. min poziom w zbiorniku");
				break;
			case 4:
				displayStringMenu("4. max poziom w zbiorniku");
				break;
			case 5:
				displayStringMenu("5. pomiar co ile sec");
		}
	}
	
	if(page==2)
	{
		matrix.fillScreen(LOW);
		matrix.setCursor(0,0);
		switch(menuitem)
		{
			case 2:	//Auto/Manual
				{
					if(konfig.trybPracy)
						matrix.print("Au");
					else
						matrix.print("Mn");					
				}
				break;
			
			case 3:	//min value
				matrix.print(selectedValue);
				break;
			
			case 4:	//max value
				matrix.print(selectedValue);
				break;
			
			case 5:	//co ile pomiar
				matrix.print(selectedValue);
		}
		matrix.write();
	}
}

void displayStringMenu(String menuTekst)
{
	unsigned long now1 = millis();
		
	if(now1-lastt > inter)
	{
		matrix.fillScreen(LOW);
		int letter = i / width;
		int x = (matrix.width() - 1) - i % width;
		int y = (matrix.height() - 8) / 2; // center the text vertically

		while ( x + width - spacer >= 0 && letter >= 0 )
		{
			if ( letter < menuTekst.length() )
			{
				matrix.drawChar(x, y, menuTekst[letter], HIGH, LOW, 1);
			}

			letter--;
			x -= width;
		}

		matrix.write(); // Send bitmap to display
		
		if(i < width * menuTekst.length() + matrix.width() - 1 - spacer)
			i++;
		else
			i=0;
		lastt = now1;
	}	
}

void EkranPomiar(boolean oOver, boolean oMANL, boolean oPOMP)
{
	int maxBar, curBar=0;
	unsigned long now1 = millis();
	
	maxBar = konfig.sensorMin-konfig.sensorMax;
	curBar = zbiornikPoziom*8/maxBar; // wartosc maxBar moze byc max dla BAR = 46

	matrix.fillScreen(LOW);
	
	if(blinkPomiar)
	{	
		matrix.drawPixel(15,0,HIGH);
		if(now1-lat > 100)
		{
			matrix.drawPixel(15,0,LOW);
			blinkPomiar=false;
			lat = now1;
		}
			
	}
	else
		lat=now1;
	
	if(oPOMP)
	{
		// LED ON je¿eli pompa w³¹czona
		matrix.drawPixel(15,2,HIGH);
	}
	else
		matrix.drawPixel(15,2,LOW);

	if(konfig.trybPracy)
	{
		// LED ON je¿eli tryb pracy Manual
		matrix.drawPixel(15,4,LOW);
	}
	else
		matrix.drawPixel(15,4,HIGH);
	
	if(oOver)
	{
		matrix.drawPixel(15,6,HIGH);		
	}
	else
		matrix.drawPixel(15,6,LOW);
	
	
	matrix.setCursor(2,0);
	matrix.drawLine(0,7,0,8-curBar,HIGH);
	matrix.setCursor(2,0);
	matrix.print(String(zbiornikPoziom));
	matrix.write(); // Send bitmap to display	
}

