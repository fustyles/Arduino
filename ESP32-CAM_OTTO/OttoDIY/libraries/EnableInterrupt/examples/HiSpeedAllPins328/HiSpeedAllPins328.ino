// EnableInterrupt Simple example sketch
// See the Wiki at http://code.google.com/p/arduino-pinchangeint/wiki for more information.

// This example demonstrates the use of the EnableInterrupt library on all pins.
// The library has only been tested on an Arduino Duemilanove and Mega ADK.

#define NEEDFORSPEED
#define INTERRUPT_FLAG_PIN2    iflag_pin2
#define INTERRUPT_FLAG_PIN3    iflag_pin3
#define INTERRUPT_FLAG_PIN4    iflag_pin4
#define INTERRUPT_FLAG_PIN5    iflag_pin5
#define INTERRUPT_FLAG_PIN6    iflag_pin6
#define INTERRUPT_FLAG_PIN7    iflag_pin7
#define INTERRUPT_FLAG_PIN8    iflag_pin8
#define INTERRUPT_FLAG_PIN9    iflag_pin9
#define INTERRUPT_FLAG_PIN10   iflag_pin10
#define INTERRUPT_FLAG_PIN11   iflag_pin11
#define INTERRUPT_FLAG_PIN12   iflag_pin12
#define INTERRUPT_FLAG_PIN13   iflag_pin13
#define INTERRUPT_FLAG_PINA0   iflag_pinA0
#define INTERRUPT_FLAG_PINA1   iflag_pinA1
#define INTERRUPT_FLAG_PINA2   iflag_pinA2
#define INTERRUPT_FLAG_PINA3   iflag_pinA3
#define INTERRUPT_FLAG_PINA4   iflag_pinA4
#define INTERRUPT_FLAG_PINA5   iflag_pinA5

#include <EnableInterrupt.h>

volatile uint8_t anyInterruptCounter=0;

#ifdef ARDUINO_328
#define PINCOUNT(x) iflag_pin ##x

#define updateOn(x) \
  if (PINCOUNT(x) != 0) { \
    printIt((char *) #x, PINCOUNT(x)); \
    PINCOUNT(x)=0; \
  }

#define disablePCInterrupt(x) \
  disableInterrupt( x | PINCHANGEINTERRUPT)

#define setupPCInterrupt(x) \
  EI_printPSTR("Add PinChange pin: "); \
  EI_printPSTR(#x); \
  EI_printPSTR("\r\n"); \
  pinMode( x, INPUT_PULLUP); \
  enableInterruptFast( x | PINCHANGEINTERRUPT, CHANGE)

#define setupInterrupt(x) \
  EI_printPSTR("Add pin: "); \
  EI_printPSTR(#x); \
  EI_printPSTR("\r\n"); \
  pinMode( x, INPUT_PULLUP); \
  enableInterruptFast( x, CHANGE)

#define setupExInterrupt(x) \
  EI_printPSTR("Add External pin: "); \
  EI_printPSTR(#x); \
  EI_printPSTR("\r\n"); \
  pinMode( x, INPUT_PULLUP); \
  enableInterruptFast( x , CHANGE)

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

uint8_t externalFlag=1;
uint8_t enabledToggle=1;
uint8_t disableCounter=0;
// In the loop, we just check to see where the interrupt count is at. The value gets updated by
// the interrupt routine.
void loop() {
  EI_printPSTR("------\r\n");
  delay(1000);                          // Perform the loop every second.
  if (disableCounter & 0x08) {
    EI_printPSTR("Toggle 2, 3, 4, 8, A0...");
    delay(1000);
    if (enabledToggle==1) {
      EI_printPSTR(" off\r\n");
      disablePCInterrupt(2);
      if (externalFlag == 1) {
        EI_printPSTR("Disable pin 3 external interrupt\r\n");
        disableInterrupt(3);
      }
      else {
        EI_printPSTR("Disable pin 3 pin change interrupt\r\n");
        disablePCInterrupt(3);
      }
      disableInterrupt(4);
      disableInterrupt(8);
      disableInterrupt(A0);
      enabledToggle=0;
    }
    else {
      if (externalFlag == 1) {
        EI_printPSTR("3 is now a Pin Change Interrupt.\r\n");
        setupPCInterrupt(3); // make sure we can switch functions.
        externalFlag=0;
      } else {
        EI_printPSTR("3 is now an External Interrupt.\r\n");
      	setupExInterrupt(3);
        externalFlag=1;
      }
      setupPCInterrupt(2);
      setupInterrupt(4);
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
  disableCounter++;
}

