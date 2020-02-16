// EnableInterrupt example sketch
// See the Wiki at http://code.google.com/p/arduino-pinchangeint/wiki for more information.

// This example demonstrates the use of the EnableInterrupt library on all pins.
// It tests the EI_ARDUINO_INTERRUPTED_PIN facility.
// The library has only been tested on an Arduino Duemilanove and Mega ADK.

#define EI_ARDUINO_INTERRUPTED_PIN
#include <EnableInterrupt.h>

volatile uint8_t externalInterruptFlag=0;
volatile uint8_t pinChangeInterruptFlag=0;
volatile uint8_t pin70Flag=0;

#ifdef ARDUINO_MEGA

void interruptFunctionPin70 () {
  pinChangeInterruptFlag=arduinoInterruptedPin;
  pin70Flag++;
}

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

#define PIN70_PORTBITMAP 0b00000100
#define PIN71_PORTBITMAP 0b00001000
#define PIN72_PORTBITMAP 0b00010000
#define PIN73_PORTBITMAP 0b00100000
#define PIN74_PORTBITMAP 0b01000000

#define PIN75_PORTBITMAP 0b01000000
#define PIN76_PORTBITMAP 0b10000000
// Attach the interrupt in setup()
// NOTE: PORTJ2-6 (aka, "Pin '70', '71', '72', '73', '74'" are turned on as OUTPUT.
// These are not true pins on the Arduino Mega series!
void setup() {
  Serial.begin(115200);
  EI_printPSTR("--- START ------------------------------------\r\n");
  setupInterrupt(SS);
  setupInterrupt(SCK);
  setupInterrupt(MOSI);
  setupInterrupt(MISO);
  setupInterrupt(10);
  setupInterrupt(11);
  setupInterrupt(12);
  setupInterrupt(13);
  setupInterrupt(14);
  setupInterrupt(15);
  setupInterrupt(A8);
  setupInterrupt(A9);
  setupInterrupt(A10);
  setupInterrupt(A11);
  setupInterrupt(A12);
  setupInterrupt(A13);
  setupInterrupt(A14);
  setupInterrupt(A15);
  //// 0b01111100
  DDRJ  |= PIN70_PORTBITMAP | PIN71_PORTBITMAP | PIN72_PORTBITMAP
	  | PIN73_PORTBITMAP | PIN74_PORTBITMAP ; // Non-Arduino Port J pins all become output.
  PORTJ |= PIN70_PORTBITMAP | PIN71_PORTBITMAP | PIN72_PORTBITMAP
	  | PIN73_PORTBITMAP | PIN74_PORTBITMAP ; // Turn them all high.
  enableInterrupt(70, interruptFunctionPin70, CHANGE);
  enableInterrupt(71, interruptFunction, CHANGE);
  enableInterrupt(72, interruptFunction, CHANGE);
  enableInterrupt(73, interruptFunction, CHANGE);
  enableInterrupt(74, interruptFunction, CHANGE);
  // External Interrupts
  setupExInterrupt(21);
  setupExInterrupt(20);
  setupExInterrupt(19);
  setupExInterrupt(18);
  setupExInterrupt(2);
  setupExInterrupt(3);
  //// 0b11000000
  DDRE  |= PIN75_PORTBITMAP | PIN76_PORTBITMAP; // Non-Arduino Port E pins all become output.
  PORTE |= PIN75_PORTBITMAP | PIN76_PORTBITMAP; // Turn them all high.
  enableInterrupt(75, interruptExFunction, CHANGE);
  enableInterrupt(76, interruptExFunction, CHANGE);
}

#define IS_PINCHANGE 0
#define IS_EXTERNAL 1
#define FLIP 0
#define FLOP 1
uint8_t pin3state=IS_PINCHANGE;
uint8_t enabledToggle=FLOP;
uint8_t toggleCounter=0;
uint16_t loopCounter=0;
uint8_t portj_follower = PIN70_PORTBITMAP;
uint8_t porte_follower = PIN75_PORTBITMAP;
uint8_t fake_pin_state = 0;
// In the loop, we just check to see where the interrupt count is at. The value gets updated by
// the interrupt routine.
void loop() {

  if (toggleCounter & 0x80) {
    EI_printPSTR("Toggle 20, 71, 75, A8, 15, MISO...");
    delay(200);
    if (enabledToggle==FLOP) {
      disableInterrupt(20);
      disableInterrupt(71);
      disableInterrupt(75);
      disableInterrupt(15);
      disableInterrupt(A8);
      disableInterrupt(MISO);
      enabledToggle=FLIP;
    }
    else {
      EI_printPSTR("***ON***");
      setupExInterrupt(20);
      setupInterrupt(71);
      setupExInterrupt(75);
      setupInterrupt(15);
      setupInterrupt(A8);
      setupInterrupt(MISO);
      enabledToggle=FLOP;
    }
    toggleCounter=0;
  }
  loopCounter++;
  //  portjfollower           00000100  PORTJ 00000000
  //  portefollower  00000000           PORTE 00000000
  //                                    fake_pin_state 0
  if (loopCounter == 0xFFFF) {
    if ((portj_follower <= PIN70_PORTBITMAP) || (porte_follower >= PIN75_PORTBITMAP)) {
      EI_printPSTR("Interrupt fake pins...");
      if (portj_follower <= PIN70_PORTBITMAP) {
        if (fake_pin_state) {
          PORTJ ^= portj_follower; // turn it off
          portj_follower <<= 1;
          fake_pin_state = 0;
        } else { 
          PORTJ ^= portj_follower; // turn it on
          fake_pin_state=1;
        }
      } else { // if porte_follower >= PIN75_PORTBITMAP
        if (fake_pin_state) {
          PORTE ^= porte_follower; // turn it off
          porte_follower <<= 1;
          fake_pin_state=0;
        } else {
          PORTE ^= porte_follower; // turn it on
          fake_pin_state=1;
        }
      }
      delay(1); // run a few instructions after triggering the interrupt.
    } else {
      portj_follower=PIN70_PORTBITMAP;
      porte_follower=PIN75_PORTBITMAP;
    }
    loopCounter=0;
  }
  if (pinChangeInterruptFlag) {
    //EI_printPSTR("Pin Change interrupt, pin "); Serial.println(arduinoInterruptedPin);
    EI_printPSTR("pci: "); Serial.println(pinChangeInterruptFlag);
    arduinoInterruptedPin=0;
    pinChangeInterruptFlag=0;
    toggleCounter++;
    if (pin70Flag) {
      EI_printPSTR("Pin70 interrupt!\n");
      pin70Flag=0;
    }
  }
  if (externalInterruptFlag) {
    //EI_printPSTR("External interrupt, pin "); Serial.println(arduinoInterruptedPin);
    EI_printPSTR("ext: "); Serial.println(externalInterruptFlag);
    arduinoInterruptedPin=0;
    externalInterruptFlag=0;
    toggleCounter++;
  }
}
