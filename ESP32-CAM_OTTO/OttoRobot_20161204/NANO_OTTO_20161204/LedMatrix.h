/******************************************************************************
* Zowi LED Matrix Library
* 
* @version 20150710
* @author Raul de Pablos Martin
*		  José Alberca Pita-Romero (Mouth's definitions)
*
******************************************************************************/
#ifndef __LEDMATRIX_H__
#define __LEDMATRIX_H__

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
#endif

////////////////////////////
// Definitions            //
////////////////////////////
#define ROWS 5
#define COLUMNS 6
#define MATRIX_LENGTH ROWS*COLUMNS





class LedMatrix
{
public:
	////////////////////////////
	// Enumerations           //
	////////////////////////////

	////////////////////////////
	// Variables              //
	////////////////////////////

	////////////////////////////
	// Functions              //
	////////////////////////////
	// LedMatrix -- LedMatrix class constructor
	LedMatrix(char ser_pin=11, char clk_pin=13, char rck_pin=12);
		
	// writeFull
	void writeFull(unsigned long value);
	
	// readFull
	unsigned long readFull(void);
	
	// setLed
	void setLed(char row, char column);
	
	// unsetLed
	void unsetLed(char row, char column);
	
	// readLed
	bool readLed(char row, char column);
	
	// clearMatrix
	void clearMatrix(void);
	
	// setEntireMatrix
	void setEntireMatrix(void);



private:	
	////////////////////////////
	// Enumerations           //
	////////////////////////////
	
	
	////////////////////////////
	// Variables              //
	////////////////////////////
    unsigned long memory;
    char SER;
    char CLK;
    char RCK;
	
	
	////////////////////////////
	// Functions              //
	////////////////////////////
	void sendMemory(void);
	
	
};

#endif // LEDMATRIX_H //
