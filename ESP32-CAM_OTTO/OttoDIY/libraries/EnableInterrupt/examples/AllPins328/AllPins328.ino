// EnableInterrupt Simple example sketch
// See the Wiki at http://code.google.com/p/arduino-pinchangeint/wiki for more information.

// This example demonstrates the use of the EnableInterrupt library on all pins.
// The library has only been tested on an Arduino Duemilanove and Mega ADK.

#include <EnableInterrupt.h>

volatile uint8_t externalInterruptCounter=0;
volatile uint8_t anyInterruptCounter=0;

#ifdef ARDUINO_328
#define PINCOUNT(x) pin ##x ##Count

// Do not use any Serial.print() in interrupt subroutines. Serial.print() uses interrupts,
// and by default interrupts are off in interrupt subroutines. Interrupt routines should also
// be as fast as possible. Here we just increment counters.
#define interruptFunction(x) \
  volatile uint8_t PINCOUNT(x); \
  void interruptFunction ##x () { \
    anyInterruptCounter++; \
    PINCOUNT(x)++; \
  }

#define interruptExFunction(x) \
  volatile uint8_t PINCOUNT(x); \
  void interruptExFunction ##x () { \
    externalInterruptCounter++; \
    anyInterruptCounter++; \
    PINCOUNT(x)++; \
  }

#define updateOn(x) \
  if (PINCOUNT(x) != 0) { \
    printIt((char *) #x, PINCOUNT(x)); \
    if (externalInterruptCounter > 0) { \
	    EI_printPSTR(" ext: "); Serial.println(externalInterruptCounter); \
	    externalInterruptCounter=0; \
    }; \
    PINCOUNT(x)=0; \
  }

#define disablePCInterrupt(x) \
  disableInterrupt( x | PINCHANGEINTERRUPT)

#define setupPCInterrupt(x) \
  EI_printPSTR("Add PinChange pin: "); \
  EI_printPSTR(#x); \
  EI_printPSTR("\r\n"); \
  pinMode( x, INPUT_PULLUP); \
  enableInterrupt( x | PINCHANGEINTERRUPT, interruptFunction##x, CHANGE)

#define setupInterrupt(x) \
  EI_printPSTR("Add pin: "); \
  EI_printPSTR(#x); \
  EI_printPSTR("\r\n"); \
  pinMode( x, INPUT_PULLUP); \
  enableInterrupt( x, interruptFunction##x, CHANGE)

#define setupExInterrupt(x) \
  EI_printPSTR("Add External pin: "); \
  EI_printPSTR(#x); \
  EI_printPSTR("\r\n"); \
  pinMode( x, INPUT_PULLUP); \
  enableInterrupt( x , interruptExFunction##x, CHANGE)

interruptFunction(2);
interruptExFunction(3);
interruptFunction(4);
interruptFunction(5);
interruptFunction(6);
interruptFunction(7);
interruptFunction(8);
interruptFunction(9);
interruptFunction(10);
interruptFunction(11);
interruptFunction(12);
interruptFunction(13);
interruptFunction(A0);
interruptFunction(A1);
interruptFunction(A2);
interruptFunction(A3);
interruptFunction(A4);
interruptFunction(A5);

volatile uint8_t otherCounter=0;
void otherInterrupt3Function(void) { // Must appear after interruptFunction(3)
  pin3Count++;
  otherCounter++;
}
#else
#error This sketch supports 328-based Arduinos only.
#endif

void printIt(char *pinNumber, uint8_t count) {
    EI_printPSTR(" Pin ");
    Serial.print(pinNumber);
    EI_printPSTR(" was interrupted: ");
    Serial.println(count, DEC);
}

// Attach the interrupt in setup()
// NOTE: PORTJ2-6 (aka, "Pin '70', '71', '72', '73', '74'" are turned on as OUTPUT.
// These are not true pins on the Arduino Mega series!
void setup() {
  Serial.begin(115200);
  EI_printPSTR("--- START ------------------------------------\r\n");
#ifdef DEBUG
  pinMode(PINSIGNAL, OUTPUT);
#endif
  // PINS 0 and 1 NOT USED BECAUSE OF Serial.print()
  setupPCInterrupt(2); // by default, will be External Interrupt
  setupExInterrupt(3);
  setupInterrupt(4);
  setupInterrupt(5);
  setupInterrupt(6);
  setupInterrupt(7);
  setupInterrupt(8);
  setupInterrupt(9);
  setupInterrupt(10);
  setupInterrupt(11);
  setupInterrupt(12);
#ifndef DEBUG
  // NOTE: Because the Arduino Duemilanove has an LED to ground and a 1k resistor in series with
  // it to the pin, Voltage at the pin should be hovering between 1-3 volts. 'nearly' ground. So
  // a wire to ground will not trip an interrupt, even though we have INPUT_PULLUP. A wire to PWR
  // will trigger an interrupt. The Uno has a op-amp buffer/driver to the LED, so will not have
  // this problem; it will behave like the other pins.
  setupInterrupt(13);
#endif
  setupInterrupt(A0);
  setupInterrupt(A1);
  setupInterrupt(A2);
  setupInterrupt(A3);
  setupInterrupt(A4);
  setupInterrupt(A5);
}

uint8_t otherToggle=1;
uint8_t enabledToggle=1;
uint8_t disableCounter=0;
// In the loop, we just check to see where the interrupt count is at. The value gets updated by
// the interrupt routine.
void loop() {
  EI_printPSTR("------\r\n");
  delay(1000);                          // Perform the loop every second.
  if (disableCounter & 0x08) {
    EI_printPSTR("Toggle 2, 3, 8, A0...");
    delay(1000);
    if (enabledToggle==1) {
      disablePCInterrupt(2);
      disableInterrupt(3);
      disableInterrupt(8);
      disableInterrupt(A0);
      enabledToggle=0;
    }
    else {
      if (otherToggle == 1) {
        EI_printPSTR("3 is now a Pin Change Interrupt.\r\n");
        enableInterrupt(3, otherInterrupt3Function, CHANGE); // make sure we can switch functions.
        otherToggle=0;
      } else {
        EI_printPSTR("3 is now an External Interrupt.\r\n");
        otherToggle=1;
      	setupExInterrupt(3);
      }
      setupPCInterrupt(2);
      setupInterrupt(8);
      setupInterrupt(A0);
      enabledToggle=1;
    }
    disableCounter=0;
  }
  updateOn(2);
  updateOn(3);
  updateOn(4);
  updateOn(5);
  updateOn(6);
  updateOn(7);
  updateOn(8);
  updateOn(9);
  updateOn(10);
  updateOn(11);
  updateOn(12);
  updateOn(13);
  updateOn(A0);
  updateOn(A1);
  updateOn(A2);
  updateOn(A3);
  updateOn(A4);
  updateOn(A5);
  EI_printPSTR("Consolidated interrupt count: "); Serial.println(anyInterruptCounter);
  if (otherCounter) {
	  printIt((char *) "OTHER3", otherCounter);
    otherCounter=0;
  }
  externalInterruptCounter=0;
  disableCounter++;
}

