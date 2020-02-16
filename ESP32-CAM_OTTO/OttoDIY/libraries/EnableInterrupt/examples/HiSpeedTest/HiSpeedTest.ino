// EnableInterrupt Simple example sketch
// See https://github.com/GreyGnome/EnableInterrupt and the README.md for more information.
//
// https://github.com/GreyGnome/EnableInterrupt/wiki/Usage#Summary

//#define OLDLIBRARY

#define NEEDFORSPEED

#define THEINTERRUPTPIN8
// Define the above, and undefine this for External Interrupt measurements.
//#define THEINTERRUPTPIN2 

#if defined THEINTERRUPTPIN8
#define PININTERRUPT 8
#define PININTERRUPT_ON PORTB |= (1 << PB0)
#define PININTERRUPT_OFF PORTB &= ~(1 << PB0)
#define THEINTERRUPTVARIABLE myvariable_pin8
#ifdef NEEDFORSPEED
#define INTERRUPT_FLAG_PIN8 myvariable_pin8 // NOTICE: NO semicolon!!! This needs to track the PININTERRUPT pin
#else
volatile uint8_t myvariable_pin8=0;
#endif
#endif

#if defined THEINTERRUPTPIN2
#define PININTERRUPT 2
#define PININTERRUPT_ON PORTD |= (1 << PD2)
#define PININTERRUPT_OFF PORTD &= ~(1 << PD2)
#define THEINTERRUPTVARIABLE myvariable_pin2
#ifdef NEEDFORSPEED
#define INTERRUPT_FLAG_PIN2 myvariable_pin2 // NOTICE: NO semicolon!!! This needs to track the PININTERRUPT pin
#else
volatile uint8_t myvariable_pin2=0;
#endif
#endif


#ifdef OLDLIBRARY
#include <PinChangeInt.h>
#define EI_printPSTR(x) SerialPrint_P(PSTR(x))
void SerialPrint_P(const char *str) {
	  for (uint8_t c; (c = pgm_read_byte(str)); str++) Serial.write(c);
}
#else
#include <EnableInterrupt.h>
#endif
#include <avr/cpufunc.h>

void incrementMyVariable() { // The compiler is smart: only uses memory if needed (ie, NEEDFORSPEED not defined)
  THEINTERRUPTVARIABLE++;
}

// Attach the interrupt in setup()
void setup() {
  //uint8_t pind, pink;
  Serial.begin(115200);
  EI_printPSTR("---------------------------------------\r\n");
  pinMode(PININTERRUPT, OUTPUT);  // Configure the pin as an output
  PININTERRUPT_OFF;
#ifdef OLDLIBRARY
  attachPinChangeInterrupt(PININTERRUPT, incrementMyVariable, CHANGE);
#else
#ifdef NEEDFORSPEED
  enableInterruptFast(PININTERRUPT, CHANGE);
#else
  enableInterrupt(PININTERRUPT, incrementMyVariable, CHANGE);
#endif
#endif
  EI_printPSTR("Interrupt enabled, let's go!");
}

// In the loop, we just check to see where the interrupt count is at. The value gets updated by the
// interrupt routine.
void loop() {

  EI_printPSTR("---------------------------------------\r\n");
  delay(1000);                          // Every second,
  PININTERRUPT_ON; 	// software interrupt
  _NOP();		// See "8-bit AVR Microcontroller with 4/8/16/32K Bytes In-System Programmable
  _NOP();		// Flash" document, Rev. 8271C-AVR-08/10, p. 71.
  _NOP();		// The interrupt signal must be held until PCIFR is changed, in order for the
#if defined THEINTERRUPTPIN8
  _NOP();               // CPU to execute the ISR. This requires 4 nop assembler instructions.
#else
                        // (three nop's for an External Interrupt)
#endif
  PININTERRUPT_OFF;     // ...This is after the interrupt.
  if (THEINTERRUPTVARIABLE > 0) {
    EI_printPSTR("Pin was interrupted: ");
    Serial.print(THEINTERRUPTVARIABLE, DEC);      // print the interrupt count.
    EI_printPSTR(" times so far.\r\n");
    THEINTERRUPTVARIABLE=0;
  }
  else {
    EI_printPSTR("No interrupts.\r\n");
  }
}
