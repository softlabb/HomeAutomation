

// --------------------------------------------------------
// EKRANY
// --------------------------------------------------------
//
void EkranIntro()
{
	display.clearDisplay();

	display.setCursor(0,0);
	//display.setTextColor(WHITE, BLACK); // 'inverted' text
	display.println("");
	display.println("SoftLab(c)2017");
	display.println("  Pomp Module");
	display.println("     v1.0");
	
	display.display();
	delay(3000);
}

void EkranStop()
{
	display.clearDisplay();

	// ****************************************************************
	// Ramka
	
	uint8_t color = 1;
	for (int16_t i=0; i<6; i+=3)
	{
		// alternate colors
		display.fillRect(i, i, display.width()-i*2, display.height()-i*2, color%2);
		color++;
	}

	// ****************************************************************
	// napis STOP
	display.setTextSize(3);
	display.setCursor(8,15);
	display.println("STOP");

	display.display();
}

void EkranMenu()
{
	if (page==1)
	{
		display.setTextSize(1);
		display.clearDisplay();
		display.setTextColor(BLACK, WHITE);
		display.setCursor(15, 0);
		display.print("MAIN MENU");
		display.drawFastHLine(0,10,83,BLACK);

		if(menuitem==1 && frame ==1)
		{
			displayMenuItem(menuItem1, 15,true);
			displayMenuItem(menuItem2, 25,false);
			displayMenuItem(menuItem3, 35,false);
		}
		else if(menuitem == 2 && frame == 1)
		{
			displayMenuItem(menuItem1, 15,false);
			displayMenuItem(menuItem2, 25,true);
			displayMenuItem(menuItem3, 35,false);
		}
		else if(menuitem == 3 && frame == 1)
		{
			displayMenuItem(menuItem1, 15,false);
			displayMenuItem(menuItem2, 25,false);
			displayMenuItem(menuItem3, 35,true);
		}
		else if(menuitem == 4 && frame == 2)
		{
			displayMenuItem(menuItem2, 15,false);
			displayMenuItem(menuItem3, 25,false);
			displayMenuItem(menuItem4, 35,true);
		}
		else if(menuitem == 3 && frame == 2)
		{
			displayMenuItem(menuItem2, 15,false);
			displayMenuItem(menuItem3, 25,true);
			displayMenuItem(menuItem4, 35,false);
		}
		else if(menuitem == 2 && frame == 2)
		{
			displayMenuItem(menuItem2, 15,true);
			displayMenuItem(menuItem3, 25,false);
			displayMenuItem(menuItem4, 35,false);
		}
		else if(menuitem == 5 && frame == 3)
		{
			displayMenuItem(menuItem3, 15,false);
			displayMenuItem(menuItem4, 25,false);
			displayMenuItem(menuItem5, 35,true);
		}
		else if(menuitem == 4 && frame == 3)
		{
			displayMenuItem(menuItem3, 15,false);
			displayMenuItem(menuItem4, 25,true);
			displayMenuItem(menuItem5, 35,false);
		}
		else if(menuitem == 3 && frame == 3)
		{
			displayMenuItem(menuItem3, 15,true);
			displayMenuItem(menuItem4, 25,false);
			displayMenuItem(menuItem5, 35,false);
		}		
		else if(menuitem == 6 && frame == 4)
		{
			displayMenuItem(menuItem4, 15,false);
			displayMenuItem(menuItem5, 25,false);
			displayMenuItem(menuItem6, 35,true);
		}
		else if(menuitem == 5 && frame == 4)
		{
			displayMenuItem(menuItem4, 15,false);
			displayMenuItem(menuItem5, 25,true);
			displayMenuItem(menuItem6, 35,false);
		}
		else if(menuitem == 4 && frame == 4)
		{
			displayMenuItem(menuItem4, 15,true);
			displayMenuItem(menuItem5, 25,false);
			displayMenuItem(menuItem6, 35,false);
		}
		else if(menuitem == 7 && frame == 5)
		{
			displayMenuItem(menuItem5, 15,false);
			displayMenuItem(menuItem6, 25,false);
			displayMenuItem(menuItem7, 35,true);
		}
		else if(menuitem == 6 && frame == 5)
		{
			displayMenuItem(menuItem5, 15,false);
			displayMenuItem(menuItem6, 25,true);
			displayMenuItem(menuItem7, 35,false);
		}
		else if(menuitem == 5 && frame == 5)
		{
			displayMenuItem(menuItem5, 15,true);
			displayMenuItem(menuItem6, 25,false);
			displayMenuItem(menuItem7, 35,false);
		}
		else if(menuitem == 8 && frame == 6)
		{
			displayMenuItem(menuItem6, 15,false);
			displayMenuItem(menuItem7, 25,false);
			displayMenuItem(menuItem8, 35,true);
		}
		else if(menuitem == 7 && frame == 6)
		{
			displayMenuItem(menuItem6, 15,false);
			displayMenuItem(menuItem7, 25,true);
			displayMenuItem(menuItem8, 35,false);
		}
		else if(menuitem == 6 && frame == 6)
		{
			displayMenuItem(menuItem6, 15,true);
			displayMenuItem(menuItem7, 25,false);
			displayMenuItem(menuItem8, 35,false);
		}
		else if(menuitem == 9 && frame == 7)
		{
			displayMenuItem(menuItem7, 15,false);
			displayMenuItem(menuItem8, 25,false);
			displayMenuItem(menuItem9, 35,true);
		}
		else if(menuitem == 8 && frame == 7)
		{
			displayMenuItem(menuItem7, 15,false);
			displayMenuItem(menuItem8, 25,true);
			displayMenuItem(menuItem9, 35,false);
		}
		else if(menuitem == 7 && frame == 7)
		{
			displayMenuItem(menuItem7, 15,true);
			displayMenuItem(menuItem8, 25,false);
			displayMenuItem(menuItem9, 35,false);
		}
		display.display();
	}	
	
	// ---------------------------------------------
	// dla page > 1 czyli drugi poziom menu
	
	/*
	else if (page==2 && menuitem == 1)
	{
		displayIntMenuPage(menuItem1, contrast);
	}
	else if (page==2 && menuitem == 2)
	{
		displayIntMenuPage(menuItem2, volume);
	}
	else if (page==2 && menuitem == 3)
	{
		displayStringMenuPage(menuItem3, language[selectedLanguage]);
	}
	else if (page==2 && menuitem == 4)
	{
		displayStringMenuPage(menuItem4, difficulty[selectedDifficulty]);
	}
	else if (page==2 && menuitem == 4)
	{
		displayStringMenuPage(menuItem4, difficulty[selectedDifficulty]);
	}
	*/
}

void displayMenuItem(String item, int position, boolean selected)
{
	if(selected)
	{
		display.setTextColor(WHITE, BLACK);
	}
	else
	{
		display.setTextColor(BLACK, WHITE);
	}
	
	display.setCursor(0, position);
	display.print(">"+item);
}

void displayStringMenuPage(String menuItem, String value)
{
	display.setTextSize(1);
	display.clearDisplay();
	display.setTextColor(BLACK, WHITE);
	display.setCursor(15, 0);
	display.print(menuItem);
	display.drawFastHLine(0,10,83,BLACK);
	display.setCursor(5, 15);
	display.print("Value");
	display.setTextSize(2);
	display.setCursor(5, 25);
	display.print(value);
	display.setTextSize(2);
	display.display();
}

void EkranPomiar(boolean oAUTO, boolean oMANL, boolean oPOMP)
{
	int maxBar, curBar=0;
	
	maxBar = 20;//sensorMin-sensorMax;
	
	display.clearDisplay();

	// ****************************************************************
	// Opis MIN, MAX
	
	display.setTextSize(1);
	display.setCursor(0,0);
	display.write(24);
	//display.println("Max");
	display.setCursor(0,41);
	display.write(25);
	//display.println("Min");

	// ****************************************************************
	// Znaki UP, DOWN,

	switch(1) //##################### forecast
	{
		case 0:
		display.setCursor(0,26);  // znak DWOWN jak woda opada
		display.write(31);
		break;
		
		case 1:
		display.setCursor(0,20);  // znak rownosci jak nic sie nie zmienia w kolejnych odczytach
		display.write(61);
		break;
		
		case 2:
		display.setCursor(0,14);  // znak UP jak woda przybiera
		display.write(30);
	}

	// ****************************************************************
	// BAR
	display.drawLine(10,0,18,0,BLACK); //pozioma gorna
	display.drawLine(13,0,13,47,BLACK); //pionowa lewa
	display.drawLine(18,0,18,47,BLACK); //pionowa prawa
	display.drawLine(10,47,18,47,BLACK); //pozioma dolna

	// skala
	display.drawLine(12,11,13,11,BLACK); // 3/4 MALE
	display.drawLine(10,23,13,23,BLACK); // 1/2 duzy
	display.drawLine(12,35,13,35,BLACK); // 1/4 MALE
	
	// Wypelnienie BARA
	curBar = sensorPoziom*46/maxBar; // wartosc maxBar moze byc max dla BAR = 46
	
	for(int y=0; y<curBar ; y++)
	{
		switch (y)
		{
			case 11:
			display.drawLine(16,46-y,18,46-y,BLACK); //3/4 duzy
			break;
			case 23:
			display.drawLine(16,46-y,18,46-y,BLACK); //1/2 duzy
			break;
			case 35:
			display.drawLine(16,46-y,18,46-y,BLACK); //1/4 duzy
			break;
			default:
			display.drawLine(13,46-y,18,46-y,BLACK); //pozioma dolna
		}
		
	}

	// ****************************************************************
	// Liczba - poziom MAX
	display.setCursor(20,0); //30
	display.print(20);    // ##################### wartosc sensorMAX sensorMin-sensorMax
	display.print("cm");

	// ****************************************************************
	// Liczba - procent wypelnienia zbiornika
	int procent = sensorPoziom*100/20; //######################## maxBar
	display.setCursor(20,23);
	display.print(procent);    // wartosc sensorMAX
	display.print("%");
	
	// ****************************************************************
	// Liczba duza - aktualny poziom
	display.setTextSize(2);
	display.setCursor(20,34);
	display.println(sensorPoziom);
	display.setTextSize(1);
	display.setCursor(43,41);
	display.println("cm");

	// ****************************************************************
	// Ramka Statusu
	display.drawLine(55,0,83,0,BLACK); //pozioma gorna
	display.drawLine(55,41,83,41,BLACK); //poziona dolna
	display.drawLine(55,0,55,41,BLACK); //pion lewy
	display.drawLine(83,0,83,41,BLACK); //pion prawy

	// ****************************************************************
	// Stan pracy
	if(oAUTO)
	{
		display.setCursor(58,1);
		display.print("AUTO");
	}
	
	if(oMANL)
	{
		display.setCursor(58,9);
		/*
		if(timeFlaga1)
		display.setTextColor(WHITE, BLACK); // 'inverted' text
		else
		display.setTextColor(BLACK, WHITE); // 'inverted' text
		*/
		display.print("MANL");
	}
	
	if(oPOMP)
	{
		display.setCursor(58,17);
		/*
		if(timeFlaga1)
		display.setTextColor(WHITE, BLACK); // 'inverted' text
		else
		display.setTextColor(BLACK, WHITE); // 'inverted' text
		*/
		display.print("PUMP");
	}

	if(procent>100)
	{
		digitalWrite(LED_PIN_OVER, HIGH);   // turn the LED on (HIGH is the voltage level)
		display.setCursor(58,25);
		/*
		if(timeFlaga1)
			display.setTextColor(WHITE, BLACK); // 'inverted' text
		else
			display.setTextColor(BLACK, WHITE); // 'inverted' text
		*/
		display.print("OVER");
		
	}
	else
		digitalWrite(LED_PIN_OVER, LOW);   // turn the LED on (HIGH is the voltage level)

	display.setTextColor(BLACK, WHITE); // 'inverted' text
	display.setCursor(58,33);
	//display.print(EEPROM.read(EEPROM_CON));
	
	display.display();
}

