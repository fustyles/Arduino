// EnableInterrupt example sketch
// See the Wiki at http://code.google.com/p/arduino-pinchangeint/wiki for more information.

// This example demonstrates the use of the EnableInterrupt library on all pins.
// It tests the EI_ARDUINO_INTERRUPTED_PIN facility.
// The library has only been tested on an Arduino Duemilanove and Mega ADK.

#define EI_ARDUINO_INTERRUPTED_PIN
#include <EnableInterrupt.h>

volatile uint8_t externalInterruptFlag=0;
volatile uint8_t pinChangeInterruptFlag=0;

#ifdef ARDUINO_328
#define PINCOUNT(x) pin ##x ##Count

void interruptFunction () {
  pinChangeInterruptFlag=arduinoInterruptedPin;
}

void interruptExFunction () {
  externalInterruptFlag=arduinoInterruptedPin;
}

#define disablePCInterrupt(x) \
  disableInterrupt( x | PINCHANGEINTERRUPT)

#define setupPCInterrupt(x) \
  EI_printPSTR("Add PinChange pin: "); \
  EI_printPSTR(#x); \
  EI_printPSTR("\r\n"); \
  pinMode( x, INPUT_PULLUP); \
  enableInterrupt( x | PINCHANGEINTERRUPT, interruptFunction, CHANGE)

#define setupInterrupt(x) \
  EI_printPSTR("Add pin: "); \
  EI_printPSTR(#x); \
  EI_printPSTR("\r\n"); \
  pinMode( x, INPUT_PULLUP); \
  enableInterrupt( x, interruptFunction, CHANGE)

#define setupExInterrupt(x) \
  EI_printPSTR("Add External pin: "); \
  EI_printPSTR(#x); \
  EI_printPSTR("\r\n"); \
  pinMode( x, INPUT_PULLUP); \
  enableInterrupt( x , interruptExFunction, CHANGE)
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
  setupPCInterrupt(2); // by default, would be External Interrupt
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
  // will trigger an interrupt. The Uno has an op-amp buffer/driver to the LED, so will not have
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

#define IS_PINCHANGE 0
#define IS_EXTERNAL 1
#define FLIP 0
#define FLOP 1
uint8_t pin3state=IS_PINCHANGE;
uint8_t enabledToggle=FLOP;
uint8_t toggleCounter=0;
// In the loop, we just check to see where the interrupt count is at. The value gets updated by
// the interrupt routine.
void loop() {
  if (toggleCounter & 0x10) {
    EI_printPSTR("Toggle 2, 3, 8, A0...");
    delay(200);
    if (enabledToggle==FLOP) {
      if (pin3state==IS_PINCHANGE)
        disablePCInterrupt(3);
      else
        disableInterrupt(3);
      disablePCInterrupt(2);
      disableInterrupt(8);
      disableInterrupt(A0);
      enabledToggle=FLIP;
    }
    else {
      EI_printPSTR("3 is now a");
      if (pin3state == IS_PINCHANGE) {
        setupExInterrupt(3);
        EI_printPSTR("n external interrupt.\r\n");
        pin3state=IS_EXTERNAL;
      } else {
      	setupExInterrupt(3);
        EI_printPSTR(" pin change interrupt.\r\n");
        pin3state=IS_PINCHANGE;
      }
      setupPCInterrupt(2);
      setupInterrupt(8);
      setupInterrupt(A0);
      enabledToggle=FLOP;
    }
    toggleCounter=0;
  }
  // Bug: 0 is not (technically) a proper test because (technically) we have Arduino pin 0.
  // But we don't support pin 0 in this sketch (it's used for Serial print).
  if (pinChangeInterruptFlag) {
    EI_printPSTR("Pin Change interrupt, pin "); Serial.println(pinChangeInterruptFlag);
    pinChangeInterruptFlag=0;
    toggleCounter++;
  }
  if (externalInterruptFlag) {
    EI_printPSTR("External interrupt, pin "); Serial.println(externalInterruptFlag);
    externalInterruptFlag=0;
    toggleCounter++;
  }
}
