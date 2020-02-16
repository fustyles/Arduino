/******************************************************************************
* Zowi LED Matrix Library
* 
* @version 20150710
* @author Raul de Pablos Martin
*         José Alberca Pita-Romero (Mouth's definitions)
******************************************************************************/

#include "LedMatrix.h"

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

LedMatrix::LedMatrix(char ser_pin, char clk_pin, char rck_pin) {
	memory = 0x00000000;
	SER = ser_pin;
	CLK = clk_pin;
	RCK = rck_pin;
	pinMode(SER, OUTPUT);
	pinMode(CLK, OUTPUT);
	pinMode(RCK, OUTPUT);
	digitalWrite(SER, LOW);
	digitalWrite(CLK, LOW);
	digitalWrite(RCK, LOW);
	sendMemory();
}

void LedMatrix::writeFull(unsigned long value) {
	memory = value;
	sendMemory();
}

unsigned long LedMatrix::readFull(void) {
	return memory;
}

void LedMatrix::setLed(char row, char column) {
	if(row >= 1 && row <= ROWS && column >= 1 && column <= COLUMNS) {
		memory |= (1L << (MATRIX_LENGTH - (row-1)*COLUMNS - (column)));
		sendMemory();
	}
}

void LedMatrix::unsetLed(char row, char column) {
	if(row >= 1 && row <= ROWS && column >= 1 && column <= COLUMNS) {
		memory &= ~(1L << (MATRIX_LENGTH - (row-1)*COLUMNS - (column)));
		sendMemory();
	}
}

void LedMatrix::clearMatrix(void) {
	memory = 0x00000000;
	sendMemory();
}

void LedMatrix::setEntireMatrix(void) {
	memory = 0x3FFFFFFF;
	sendMemory();
}

void LedMatrix::sendMemory(void) {
	int i;
	
	for(i = 0; i < MATRIX_LENGTH; i++) {
		digitalWrite(SER, 1L & (memory >> i));	
		// ## adjust this delay to match with 74HC595 timing
		asm volatile ("nop");
		asm volatile ("nop");
		asm volatile ("nop");
		digitalWrite(CLK, 1);
		// ## adjust this delay to match with 74HC595 timing
		asm volatile ("nop");
		asm volatile ("nop");
		asm volatile ("nop");
		digitalWrite(CLK, 0);	
	}
	
	digitalWrite(RCK, 1);
	// ## adjust this delay to match with 74HC595 timing
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	digitalWrite(RCK, 0);	
}

