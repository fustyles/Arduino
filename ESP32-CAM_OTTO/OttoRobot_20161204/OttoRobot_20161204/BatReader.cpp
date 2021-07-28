/******************************************************************************
* Zowi Battery Reader Library
* 
* @version 20150831
* @author Raul de Pablos Martin
*
******************************************************************************/

#include "BatReader.h"

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

BatReader::BatReader() {
}

double BatReader::readBatVoltage(void) {
	double readed = (double)(analogRead(BAT_PIN)*ANA_REF)/1024;
	if(readed > BAT_MAX) return BAT_MAX;
	else return readed;
}

double BatReader::readBatPercent(void) {
	double value = (SLOPE*readBatVoltage()) - OFFSET;
	if(value < 0) return 0;
	else return value;
}

