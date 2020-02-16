// EnableInterrupt Simple example sketch
// See the Wiki at http://code.google.com/p/arduino-pinchangeint/wiki for more information.

#include <EnableInterrupt.h>

volatile uint8_t externalInterruptCounter=0;
volatile uint8_t anyInterruptCounter=0;

#ifdef ARDUINO_MEGA
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
    PINCOUNT(x)=0; \
  }

#define disablePCInterrupt(x) \
  disableInterrupt( x | PINCHANGEINTERRUPT)

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
  enableInterrupt( x, interruptExFunction##x, CHANGE)

interruptFunction(SS);
interruptFunction(SCK);
interruptFunction(MOSI);
interruptFunction(MISO);
interruptFunction(10);
interruptFunction(11);
interruptFunction(12);
interruptFunction(13);
interruptFunction(14);
interruptFunction(15);
interruptFunction(A8);
interruptFunction(A9);
interruptFunction(A10);
interruptFunction(A11);
interruptFunction(A12);
interruptFunction(A13);
interruptFunction(A14);
interruptFunction(A15);
interruptFunction(70); // fake 70, trick to allow software interrupts on Port J. PJ2
interruptFunction(71); // fake 71. PJ3
interruptFunction(72); // fake 72. PJ4
interruptFunction(73); // fake 73. PJ5
interruptFunction(74); // fake 74. PJ6
// External Interrupts
interruptExFunction(21);
interruptExFunction(20);
interruptExFunction(19);
interruptExFunction(18);
interruptExFunction(2);
interruptExFunction(3);
interruptExFunction(75); // fake 75. PE6
interruptExFunction(76); // fake 76. PE7
#else
#error This sketch supports 2560-based Arduinos only.
#endif

void printIt(char *pinNumber, uint8_t count) {
    EI_printPSTR("Pin ");
    Serial.print(pinNumber);
    EI_printPSTR(" was interrupted: ");
    Serial.println(count, DEC);
}

// Attach the interrupt in setup()
// NOTE: PORTJ2-6 (aka, "Pin '70', '71', '72', '73', '74'" are turned on as OUTPUT.
// These are not true pins on the Arduino Mega series!
void setup() {
  //uint8_t pind, pink;
  Serial.begin(115200);
  EI_printPSTR("---------------------------------------");
  // PINS 0 and 1 NOT USED BECAUSE OF Serial.print()
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
  ////
  DDRJ |=0b01111100; // Non-Arduino Port J pins all become output.
  PORTJ|=0b01111100; // Turn them all high.
  enableInterrupt(70, interruptFunction70, CHANGE);
  enableInterrupt(71, interruptFunction71, CHANGE);
  enableInterrupt(72, interruptFunction72, CHANGE);
  enableInterrupt(73, interruptFunction73, CHANGE);
  enableInterrupt(74, interruptFunction74, CHANGE);
  // External Interrupts
  setupExInterrupt(21);
  setupExInterrupt(20);
  setupExInterrupt(19);
  setupExInterrupt(18);
  setupExInterrupt(2);
  setupExInterrupt(3);
  ////
  DDRE |=0b11000000; // Non-Arduino Port E pins all become output.
  PORTE|=0b11000000; // Turn them all high.
  enableInterrupt(75, interruptExFunction75, CHANGE);
  enableInterrupt(76, interruptExFunction76, CHANGE);
}

uint8_t otherToggle=1;
uint8_t enabledToggle=1;
uint8_t disableCounter=0;
// In the loop, we just check to see where the interrupt count is at. The value gets updated by the
// interrupt routine.
void loop() {
  uint8_t jbits =0b01111110; // PJ2
  uint8_t njbits=0b00000001; // PJ2
  uint8_t ebits =0b11000000; // PE6/7
  uint8_t nebits=0b00111111; // PE6/7

  EI_printPSTR("------\r\n");
  delay(1000);                          // Every second,
  if (disableCounter & 0x08) {
    EI_printPSTR("Toggle 20, 71, 75, A8, 15, MISO...");
    delay(1000);
    if (enabledToggle==1) {
      EI_printPSTR("---OFF---");
      disableInterrupt(20);
      disableInterrupt(71);
      disableInterrupt(75);
      disableInterrupt(15);
      disableInterrupt(A8);
      disableInterrupt(MISO);
      enabledToggle=0;
    }
    else {
      EI_printPSTR("***ON***");
      setupExInterrupt(20);
      setupInterrupt(71);
      setupExInterrupt(75);
      setupInterrupt(15);
      setupInterrupt(A8);
      setupInterrupt(MISO);
      enabledToggle=1;
    }
    disableCounter=0;
  }
  PORTE &= nebits;
  PORTJ &= njbits;
  // *out &= njbits;
  delay(1);
  PORTE |= ebits;
  PORTJ |= jbits;
  // *out |= jbits;
  delay(1);
  updateOn(SS);
  updateOn(SCK);
  updateOn(MOSI)
  updateOn(MISO);
  updateOn(10);
  updateOn(11);
  updateOn(12);
  updateOn(13);
  updateOn(14);
  updateOn(15);
  updateOn(A8);
  updateOn(A9);
  updateOn(A10);
  updateOn(A11);
  updateOn(A12);
  updateOn(A13);
  updateOn(A14);
  updateOn(A15);
  // External Interrupts
  updateOn(2);
  updateOn(3);
  updateOn(18);
  updateOn(19);
  updateOn(20);
  updateOn(21);
  // Fake Arduino Pins
  updateOn(70);
  updateOn(71);
  updateOn(72);
  updateOn(73);
  updateOn(74);
  // External Interrupts
  updateOn(75);
  updateOn(76);
  if (externalInterruptCounter > 0) { EI_printPSTR(" ext: "); Serial.println(externalInterruptCounter); }; \
  externalInterruptCounter=0;
  disableCounter++;
}

