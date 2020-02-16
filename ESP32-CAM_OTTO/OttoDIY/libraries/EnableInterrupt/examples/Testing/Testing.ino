// EnableInterrupt Simple example sketch
// See the Wiki at http://code.google.com/p/arduino-pinchangeint/wiki for more information.

#include <EnableInterrupt.h>

volatile uint8_t externalInterruptCounter=0;
volatile uint8_t anyInterruptCounter=0;

#ifdef ARDUINO_MEGA
#define PINCOUNT(x) pin ##x ##Count

register uint8_t rx_rsvd2 asm("r2");
register uint8_t rx_rsvd3 asm("r3");
register uint8_t rx_rsvd4 asm("r4");
register uint8_t rx_rsvd5 asm("r5");
register uint8_t rx_rsvd6 asm("r6");
register uint8_t rx_rsvd7 asm("r7");
register uint8_t rx_rsvd8 asm("r8");
register uint8_t rx_rsvd9 asm("r9");
register uint8_t rx_rsvd10 asm("r10");

void many_registers1() {
  uint8_t thing1=0;
  uint8_t thing2=0;
  uint8_t thing3=0;
  uint8_t thing4=0;
  uint8_t thing5=0;
  uint8_t thing6=0;
  uint8_t thing7=0;
  uint8_t thing8=0;
  uint8_t thing9=0;
  uint8_t thing10=0;
  uint8_t thing11=0;
  uint8_t thing12=0;
  uint8_t thing13=0;
  uint8_t thing14=0;
  uint8_t thing15=0;
  uint8_t thing16=0;
  uint8_t thing17=0;
  uint8_t thing18=0;
  uint8_t thing19=0;
  uint8_t thing20=0;
  uint8_t thing21=0;
  uint8_t thing22=0;
  uint8_t thing23=0;
  uint8_t thing24=0;
  uint8_t thing25=0;
  uint8_t thing26=0;
  uint8_t thing27=0;

  thing1=PINJ; thing1++; PORTH=thing1;
  thing2=PORTJ; thing2++; PORTH=thing2;
  thing3=DDRJ; thing3++; PORTH=thing3;

  thing4=PINK; thing4++; PORTH=thing4;
  thing5=PORTK; thing5++; PORTH=thing5;
  thing6=DDRK; thing6++; PORTH=thing6;
  PORTH=thing1; PORTH=thing2; PORTH=thing3;
  PORTH=thing4; PORTH=thing5; PORTH=thing6;

  thing7=PINL; thing7++; PORTH=thing7;
  thing8=PORTL; thing8++; PORTH=thing8;
  thing9=DDRL; thing9++; PORTH=thing9;
  PORTH=thing1; PORTH=thing2; PORTH=thing3;
  PORTH=thing4; PORTH=thing5; PORTH=thing6;
  PORTH=thing7; PORTH=thing8; PORTH=thing9;

  thing10=UCSR2C; thing10++; PORTH=thing10;
  thing11=UCSR2B; thing11++; PORTH=thing11;
  thing12=UCSR2A; thing12++; PORTH=thing12;
  PORTH=thing1; PORTH=thing2; PORTH=thing3;
  PORTH=thing4; PORTH=thing5; PORTH=thing6;
  PORTH=thing7; PORTH=thing8; PORTH=thing9;
  PORTH=thing10; PORTH=thing11; PORTH=thing12;

  thing13=TCNT1H; thing13++; PORTH=thing13;
  thing14=TCNT1L; thing14++; PORTH=thing14;
  thing15=OCR1AH; thing15++; PORTH=thing15;

  thing16=OCR0B; thing16++; PORTH=thing16;
  thing17=OCR0A; thing17++; PORTH=thing17;
  thing18=TCNT0; thing18++; PORTH=thing18;
  thing19=PINE; thing19++; PORTH=thing19;
  thing20=DDRE; thing20++; PORTH=thing20;
  thing21=PORTE; thing21++; PORTH=thing21;
  thing22=PINF; thing22++; PORTH=thing22;
  thing23=PORTF; thing23++; PORTH=thing23;
  thing24=DDRF; thing24++; PORTH=thing24;
  thing25=PING; thing25++;
  thing26=PORTG; thing26++;
  thing27=DDRG; thing27++;

  rx_rsvd2=PINC; rx_rsvd2++;
  rx_rsvd3=DDRC; rx_rsvd3++;
  rx_rsvd4=PORTC; rx_rsvd4++;
  rx_rsvd5=PIND; rx_rsvd5++;
  rx_rsvd6=DDRD; rx_rsvd6++;
  rx_rsvd7=PORTD; rx_rsvd7++;
  rx_rsvd8=PINA; rx_rsvd8++;
  rx_rsvd9=DDRA; rx_rsvd9++;
  rx_rsvd10=PORTA; rx_rsvd10++;

  PORTH=rx_rsvd2; PORTH=rx_rsvd3; PORTH=rx_rsvd4;
  PORTH=rx_rsvd5; PORTH=rx_rsvd6; PORTH=rx_rsvd7;
  PORTH=rx_rsvd8; PORTH=rx_rsvd9; PORTH=rx_rsvd10;
  PORTH=thing1; PORTH=thing2; PORTH=thing3;
  PORTH=thing4; PORTH=thing5; PORTH=thing6;
  PORTH=thing7; PORTH=thing8; PORTH=thing9;
  PORTH=thing10; PORTH=thing11; PORTH=thing12;
  PORTH=thing13; PORTH=thing14; PORTH=thing15;
  PORTH=thing16; PORTH=thing17; PORTH=thing18;
  PORTH=thing19; PORTH=thing20; PORTH=thing21;
  PORTH=thing22; PORTH=thing23; PORTH=thing24;
  PORTH=thing25; PORTH=thing26; PORTH=thing27;
}

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
  //Serial.begin(115200);
  //EI_printPSTR("---------------------------------------");
  // PINS 0 and 1 NOT USED BECAUSE OF Serial.print()
  setupInterrupt(SS);
  ////
  DDRJ |=0b01111100; // Non-Arduino Port J pins all become output.
  PORTJ|=0b01111100; // Turn them all high.
  ////
  DDRE |=0b11000000; // Non-Arduino Port E pins all become output.
  PORTE|=0b11000000; // Turn them all high.
}

void loop() {
  //uint8_t jbits =0b01111110; // PJ2
  //uint8_t njbits=0b00000001; // PJ2
  //uint8_t ebits =0b11000000; // PE6/7
  //uint8_t nebits=0b00111111; // PE6/7

  many_registers1();

  //EI_printPSTR("------\r\n");
  //PORTE &= nebits;
  //PORTJ &= njbits;
  // *out &= njbits;
  delay(1);
  //PORTE |= ebits;
  //PORTJ |= jbits;
  // *out |= jbits;
  updateOn(SS);
}

