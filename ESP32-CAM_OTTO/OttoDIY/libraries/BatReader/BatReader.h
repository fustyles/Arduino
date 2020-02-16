/******************************************************************************
* Zowi Battery Reader Library
* 
* @version 20150824
* @author Raul de Pablos Martin
*
******************************************************************************/
#ifndef __BATREADER_H__
#define __BATREADER_H__

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
#endif

////////////////////////////
// Definitions            //
////////////////////////////
#define BAT_PIN A7
#define BAT_MAX	4.2
#define BAT_MIN	3.25
#define ANA_REF	5
#define SLOPE	100/(BAT_MAX - BAT_MIN)
#define OFFSET	(100*BAT_MIN)/(BAT_MAX - BAT_MIN)

class BatReader
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
	// BatReader -- BatReader class constructor
	BatReader();

	// readBatPercent
	double readBatVoltage(void);
	
	// readBatPercent
	double readBatPercent(void);
	
	

private:	
	////////////////////////////
	// Enumerations           //
	////////////////////////////
	
	
	////////////////////////////
	// Variables              //
	////////////////////////////
	
	
	////////////////////////////
	// Functions              //
	////////////////////////////
	
	
};

#endif // BATREADER_H //
