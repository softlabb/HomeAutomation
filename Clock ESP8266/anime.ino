#include "Arduino.h"

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

/*
void police()
{ 
  matrix.setIntensity(5);
  
  matrix.fillScreen(LOW);
  matrix.write(); // Send bitmap to display
  delay(100);
 
  for ( int y = 0; y < matrix.height()/2; y++ ) 
  {
    matrix.drawLine(0, y, matrix.width()/2, y, HIGH);
    matrix.write(); // Send bitmap to display
  }

  for ( int y = matrix.height()/2; y < matrix.height(); y++ ) 
  {
    matrix.drawLine(matrix.width()/2, y, matrix.width(), y, HIGH);
    matrix.write(); // Send bitmap to display
  }
  matrix.write(); // Send bitmap to display
  delay(100);

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++

  matrix.fillScreen(LOW);  
  for ( int y = 0; y < matrix.height()/2; y++ ) 
  {
    matrix.drawLine(matrix.width()/2, y, matrix.width(), y, HIGH);
    matrix.write(); // Send bitmap to display
  }

  for ( int y = matrix.height()/2; y < matrix.height(); y++ ) 
  {
    matrix.drawLine(0, y, matrix.width()/2, y, HIGH);
    matrix.write(); // Send bitmap to display
  }
  matrix.write(); // Send bitmap to display
  delay(100);
  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++
  matrix.fillScreen(LOW);
  matrix.write(); // Send bitmap to display
  delay(100);
 
  for ( int y = 0; y < matrix.height()/2; y++ ) 
  {
    matrix.drawLine(0, y, matrix.width()/2, y, HIGH);
    matrix.write(); // Send bitmap to display
  }

  for ( int y = matrix.height()/2; y < matrix.height(); y++ ) 
  {
    matrix.drawLine(matrix.width()/2, y, matrix.width(), y, HIGH);
    matrix.write(); // Send bitmap to display
  }
  matrix.write(); // Send bitmap to display
  delay(100);

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++

  matrix.fillScreen(LOW);  
  for ( int y = 0; y < matrix.height()/2; y++ ) 
  {
    matrix.drawLine(matrix.width()/2, y, matrix.width(), y, HIGH);
    matrix.write(); // Send bitmap to display
  }

  for ( int y = matrix.height()/2; y < matrix.height(); y++ ) 
  {
    matrix.drawLine(0, y, matrix.width()/2, y, HIGH);
    matrix.write(); // Send bitmap to display
  }
  matrix.write(); // Send bitmap to display
  delay(100);

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++

  matrix.fillScreen(LOW);  
  matrix.setIntensity(0);
}

*/

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

