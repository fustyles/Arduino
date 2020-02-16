#ifndef US_h
#define US_h
#include "Arduino.h"

class US
{
public:
	US();
	void init(int pinTrigger, int pinEcho);
	US(int pinTrigger, int pinEcho);
	float read();

private:
	int _pinTrigger;
	int _pinEcho;
	long TP_init();

};

#endif //US_h