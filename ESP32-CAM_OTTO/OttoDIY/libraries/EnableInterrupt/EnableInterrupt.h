// in vim, :set ts=2 sts=2 sw=2 et

// EnableInterrupt, a library by GreyGnome.  Copyright 2014-2015 by Michael Anthony Schwager.

/*
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/ 

// Many definitions in /usr/avr/include/avr/io.h

#ifndef EnableInterrupt_h
#define EnableInterrupt_h
#include <Arduino.h>

#ifdef EI_ARDUINO_INTERRUPTED_PIN
uint8_t arduinoInterruptedPin=0;
#endif
// *************************************************************************************
// *************************************************************************************
// Function Prototypes *****************************************************************
// *************************************************************************************
// *************************************************************************************
// *** These are the only functions the end user (programmer) needs to consider.     ***
// *** This means you!                                                               ***
// *************************************************************************************
// *************************************************************************************

// Arduino Due (not Duemilanove) macros. Easy-peasy.
#if defined __SAM3U4E__ || defined __SAM3X8E__ || defined __SAM3X8H__
#ifdef NEEDFORSPEED
#error Due is already fast; the NEEDFORSPEED definition does not make sense on it.
#endif
define enableInterrupt(pin,userFunc,mode) attachInterrupt(pin, userFunc,mode)
define disableInterrupt(pin) detachInterrupt(pin)
#else

/* 
 * enableInterrupt- Sets up an interrupt on a selected Arduino pin.
 * or
 * enableInterruptFast- When used with the NEEDFORSPEED macro, sets up an interrupt on a selected Arduino pin.
 * 
 * Usage:
 * enableInterrupt(uint8_t pinNumber, void (*userFunction)(void), uint8_t mode);
 * or
 * enableInterrupt(uint8_t interruptDesignator, void (*userFunction)(void), uint8_t mode);
 *
 * For HiSpeed mode,
 * enableInterruptFast(uint8_t pinNumber, uint8_t mode);
 * or
 * enableInterruptFast(uint8_t interruptDesignator, uint8_t mode);
 *
 * ---------------------------------------------------------------------------------------
 *
 * disableInterrupt- Disables interrupt on a selected Arduino pin.
 *
 * Usage:
 *
 * disableInterrupt(uint8_t pinNumber);
 * or
 * disableInterrupt(uint8_t interruptDesignator);
 *
 * ---------------------------------------------------------------------------------------
 *
 * interruptDesignator: Essentially this is an Arduino pin, and if that's all you want to give
 * the function, it will work just fine. Why is it called an "interruptDesignator", then? Because
 * there's a twist: You can perform a bitwise "and" with the pin number and PINCHANGEINTERRUPT
 * to specify that you want to use a Pin Change Interrupt type of interrupt on those pins that
 * support both Pin Change and External Interrupts. Otherwise, the library will choose whatever
 * interrupt type (External, or Pin Change) normally applies to that pin, with priority to
 * External Interrupt. 
 *
 * The interruptDesignator is required because on the ATmega328 processor pins 2 and 3 support
 * ''either'' pin change or * external interrupts. On 644/1284-based systems, pin change interrupts
 * are supported on all pins and external interruptsare supported on pins 2, 10, and 11. 
 * Otherwise, each pin only supports a single type of interrupt and the
 * PINCHANGEINTERRUPT scheme changes nothing. This means you can ignore this whole discussion
 * for ATmega2560- or ATmega32U4-based Arduinos. You can probably safely ignore it for
 * ATmega328-based Arduinos, too.
 */

void enableInterrupt(uint8_t interruptDesignator, void (*userFunction)(void), uint8_t mode);
void disableInterrupt(uint8_t interruptDesignator);
void bogusFunctionPlaceholder(void);
#ifdef NEEDFORSPEED
#undef enableInterruptFast
// enableInterruptFast(uint8_t interruptDesignator, uint8_t mode);
#define enableInterruptFast(x, y) enableInterrupt(x, bogusFunctionPlaceholder, y)
#endif


// *************************************************************************************
// End Function Prototypes *************************************************************
// *************************************************************************************

#undef PINCHANGEINTERRUPT
#define PINCHANGEINTERRUPT 0x80

#undef attachPinChangeInterrupt
#undef detachPinChangeInterrupt
#define detachPinChangeInterrupt(pin)                   disableInterrupt(pin)
#define attachPinChangeInterrupt(pin,userFunc,mode)     enableInterrupt(pin, userFunc, mode)

#ifndef LIBCALL_ENABLEINTERRUPT // LIBCALL_ENABLEINTERRUPT ****************************************
#ifdef NEEDFORSPEED
void bogusFunctionPlaceholder(void) {
}
#include "utility/ei_pindefs_speed.h"
#endif

// Example: EI_printPSTR("This is a nice long string that takes no static ram");
#define EI_printPSTR(x) EI_SerialPrint_P(PSTR(x))
void EI_SerialPrint_P(const char *str) {
  for (uint8_t c; (c = pgm_read_byte(str)); str++) Serial.write(c);
} 

/* Arduino pin to ATmega port translaton is found doing digital_pin_to_port_PGM[] */
/* Arduino pin to PCMSKx bitmask is found by doing digital_pin_to_bit_mask_PGM[] */
/* ...except for PortJ, which is shifted left 1 bit in PCI1 */
volatile uint8_t *pcmsk;

// Arduino.h has these, but the block is surrounded by #ifdef ARDUINO_MAIN
#define PA 1
#define PB 2
#define PC 3
#define PD 4
#define PE 5
#define PF 6
#define PG 7
#define PH 8
#define PJ 10
#define PK 11
#define PL 12

typedef void (*interruptFunctionType)(void);

// ===========================================================================================
// CHIP SPECIFIC DATA STRUCTURES =============================================================
// ===========================================================================================

/* UNO SERIES *************************************************************************/
/* UNO SERIES *************************************************************************/
/* UNO SERIES *************************************************************************/
#if defined __AVR_ATmega168__ || defined __AVR_ATmega168A__ || defined __AVR_ATmega168P__ || \
  __AVR_ATmega168PA__ || \
  __AVR_ATmega328__ || __AVR_ATmega328P__

#define ARDUINO_328
#if defined EI_NOTPINCHANGE
#ifndef EI_NOTPORTB
#define EI_NOTPORTB
#endif
#ifndef EI_NOTPORTC
#define EI_NOTPORTC
#endif
#ifndef EI_NOTPORTD
#define EI_NOTPORTD
#endif
#endif // defined EI_NOTPINCHANGE

#ifndef NEEDFORSPEED
#define ARDUINO_PIN_B0 8
#define ARDUINO_PIN_B1 9
#define ARDUINO_PIN_B2 10
#define ARDUINO_PIN_B3 11
#define ARDUINO_PIN_B4 12
#define ARDUINO_PIN_B5 13
#define ARDUINO_PIN_C0 14
#define ARDUINO_PIN_C1 15
#define ARDUINO_PIN_C2 16
#define ARDUINO_PIN_C3 17
#define ARDUINO_PIN_C4 18
#define ARDUINO_PIN_C5 19
#define ARDUINO_PIN_D0 0
#define ARDUINO_PIN_D1 1
#define ARDUINO_PIN_D2 2
#define ARDUINO_PIN_D3 3
#define ARDUINO_PIN_D4 4
#define ARDUINO_PIN_D5 5
#define ARDUINO_PIN_D6 6
#define ARDUINO_PIN_D7 7

const uint8_t PROGMEM digital_pin_to_port_bit_number_PGM[] = {
  0, // 0 == port D, 0
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  0, // 8 == port B, 0
  1,
  2,
  3,
  4,
  5,
  0, // 14 == port C, 0
  1,
  2,
  3,
  4,
  5,
};

#if ! defined(EI_NOTEXTERNAL) && ! defined(EI_NOTINT0) && ! defined (EI_NOTINT1)
interruptFunctionType functionPointerArrayEXTERNAL[2];
#endif

#ifndef EI_NOTPORTB
// 2 of the interrupts are unsupported on Arduino UNO.
struct functionPointersPortB {
  interruptFunctionType pinZero;
  interruptFunctionType pinOne;
  interruptFunctionType pinTwo;
  interruptFunctionType pinThree;
  interruptFunctionType pinFour;
  interruptFunctionType pinFive;
};
typedef struct functionPointersPortB functionPointersPortB;

functionPointersPortB portBFunctions = { NULL, NULL, NULL, NULL, NULL, NULL };

// For Pin Change Interrupts; since we're duplicating FALLING and RISING in software,
// we have to know how the ports were defined.
volatile uint8_t risingPinsPORTB=0;
volatile uint8_t fallingPinsPORTB=0;

// for the saved state of the ports
static volatile uint8_t portSnapshotB;
#endif // EI_NOTPORTB

#ifndef EI_NOTPORTC
// 1 of the interrupts are used as RESET on Arduino UNO.
struct functionPointersPortC {
  interruptFunctionType pinZero;
  interruptFunctionType pinOne;
  interruptFunctionType pinTwo;
  interruptFunctionType pinThree;
  interruptFunctionType pinFour;
  interruptFunctionType pinFive;
};
typedef struct functionPointersPortC functionPointersPortC;

functionPointersPortC portCFunctions = { NULL, NULL, NULL, NULL, NULL, NULL };

// For Pin Change Interrupts; since we're duplicating FALLING and RISING in software,
// we have to know how the ports were defined.
volatile uint8_t risingPinsPORTC=0;
volatile uint8_t fallingPinsPORTC=0;

// for the saved state of the ports
static volatile uint8_t portSnapshotC;
#endif // EI_NOTPORTC

#ifndef EI_NOTPORTD
// 1 of the interrupts are used as RESET on Arduino UNO.
struct functionPointersPortD {
  interruptFunctionType pinZero;
  interruptFunctionType pinOne;
  interruptFunctionType pinTwo;
  interruptFunctionType pinThree;
  interruptFunctionType pinFour;
  interruptFunctionType pinFive;
  interruptFunctionType pinSix;
  interruptFunctionType pinSeven;
};
typedef struct functionPointersPortD functionPointersPortD;

functionPointersPortD portDFunctions = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

// For Pin Change Interrupts; since we're duplicating FALLING and RISING in software,
// we have to know how the ports were defined.
volatile uint8_t risingPinsPORTD=0;
volatile uint8_t fallingPinsPORTD=0;

// for the saved state of the ports
static volatile uint8_t portSnapshotD;
#endif // EI_NOTPORTD
#endif // NEEDFORSPEED

// the PCINT?_vect's are defined in the avr.h files, like iom328p.h
#define PORTB_VECT PCINT0_vect
#define PORTC_VECT PCINT1_vect
#define PORTD_VECT PCINT2_vect

/* MEGA SERIES ************************************************************************/
/* MEGA SERIES ************************************************************************/
/* MEGA SERIES ************************************************************************/
#elif defined __AVR_ATmega640__ || defined __AVR_ATmega2560__ || defined __AVR_ATmega1280__ || \
  defined __AVR_ATmega1281__ || defined __AVR_ATmega2561__
#define ARDUINO_MEGA
#if defined EI_NOTPINCHANGE
#ifndef EI_NOTPORTB
#define EI_NOTPORTB
#endif
#ifndef EI_NOTPORTJ
#define EI_NOTPORTJ
#endif
#ifndef EI_NOTPORTK
#define EI_NOTPORTK
#endif
#endif

volatile uint8_t portJPCMSK=0; // This is a shifted version of PCMSK for PortJ, so I
			                         //	don't have to perform a shift in the IRQ.

#ifndef NEEDFORSPEED
// Pin change interrupts
#define ARDUINO_PIN_B0 53
#define ARDUINO_PIN_B1 52
#define ARDUINO_PIN_B2 51
#define ARDUINO_PIN_B3 50
#define ARDUINO_PIN_B4 10
#define ARDUINO_PIN_B5 11
#define ARDUINO_PIN_B6 12
#define ARDUINO_PIN_B7 13
#define ARDUINO_PIN_J0 15
#define ARDUINO_PIN_J1 14
// "fake" pins
#define ARDUINO_PIN_J2 70
#define ARDUINO_PIN_J3 71
#define ARDUINO_PIN_J4 72
#define ARDUINO_PIN_J5 73
#define ARDUINO_PIN_J6 74

#define ARDUINO_PIN_K0 62
#define ARDUINO_PIN_K1 63
#define ARDUINO_PIN_K2 64
#define ARDUINO_PIN_K3 65
#define ARDUINO_PIN_K4 66
#define ARDUINO_PIN_K5 67
#define ARDUINO_PIN_K6 68
#define ARDUINO_PIN_K7 69

#define ARDUINO_PIN_D0 21
#define ARDUINO_PIN_D1 20
#define ARDUINO_PIN_D2 19
#define ARDUINO_PIN_D3 18
#define ARDUINO_PIN_E4 2
#define ARDUINO_PIN_E5 3
#define ARDUINO_PIN_E6 75
#define ARDUINO_PIN_E7 76

const uint8_t PROGMEM digital_pin_to_port_bit_number_PGM[] = {
  0, // PE0  pin: 0
  1, // PE1  pin: 1
  4, // PE4  pin: 2
  5, // PE5  pin: 3
  5, // PG5  pin: 4
  3, // PE3  pin: 5
  3, // PH3  pin: 6
  4, // PH4  pin: 7
  5, // PH5  pin: 8
  6, // PH6  pin: 9
  4, // PB4  pin: 10
  5, // PB5  pin: 11
  6, // PB6  pin: 12
  7, // PB7  pin: 13
  1, // PJ1  pin: 14
  0, // PJ0  pin: 15
  1, // PH1  pin: 16
  0, // PH0  pin: 17
  3, // PD3  pin: 18
  2, // PD2  pin: 19
  1, // PD1  pin: 20
  0, // PD0  pin: 21
  0, // PA0  pin: 22
  1, // PA1  pin: 23
  2, // PA2  pin: 24
  3, // PA3  pin: 25
  4, // PA4  pin: 26
  5, // PA5  pin: 27
  6, // PA6  pin: 28
  7, // PA7  pin: 29
  7, // PC7  pin: 30
  6, // PC6  pin: 31
  5, // PC5  pin: 32
  4, // PC4  pin: 33
  3, // PC3  pin: 34
  2, // PC2  pin: 35
  1, // PC1  pin: 36
  0, // PC0  pin: 37
  7, // PD7  pin: 38
  2, // PG2  pin: 39
  1, // PG1  pin: 40
  0, // PG0  pin: 41
  7, // PL7  pin: 42
  6, // PL6  pin: 43
  5, // PL5  pin: 44
  4, // PL4  pin: 45
  3, // PL3  pin: 46
  2, // PL2  pin: 47
  1, // PL1  pin: 48
  0, // PL0  pin: 49
  3, // PB3  pin: 50
  2, // PB2  pin: 51
  1, // PB1  pin: 52
  0, // PB0  pin: 53
  0, // PF0  pin: 54
  1, // PF1  pin: 55
  2, // PF2  pin: 56
  3, // PF3  pin: 57
  4, // PF4  pin: 58
  5, // PF5  pin: 59
  6, // PF6  pin: 60
  7, // PF7  pin: 61
  0, // PK0  pin: 62
  1, // PK1  pin: 63
  2, // PK2  pin: 64
  3, // PK3  pin: 65
  4, // PK4  pin: 66
  5, // PK5  pin: 67
  6, // PK6  pin: 68
  7, // PK7  pin: 69
  2, // PJ2  pin: fake70, trick to allow software interrupts on Port J. PJ2
  3, // PJ3  pin: fake71 PJ3
  4, // PJ4  pin: fake72 PJ4
  5, // PJ5  pin: fake73 PJ5
  6, // PJ6  pin: fake74 PJ6
  6, // PE6  pin: fake75 PE6
  7, // PE7  pin: fake76 PE7
};

#if ! defined(EI_NOTEXTERNAL) && ! defined(EI_NOTINT0) && ! defined(EI_NOTINT1) && ! defined(EI_NOTINT2) && ! defined(EI_NOTINT3) && ! defined(EI_NOTINT4) && ! defined(EI_NOTINT5) && ! defined(EI_NOTINT6) && ! defined(EI_NOTINT7)
interruptFunctionType functionPointerArrayEXTERNAL[8];
#endif

#ifndef EI_NOTPORTB
struct functionPointersPortB {
  interruptFunctionType pinZero;
  interruptFunctionType pinOne;
  interruptFunctionType pinTwo;
  interruptFunctionType pinThree;
  interruptFunctionType pinFour;
  interruptFunctionType pinFive;
  interruptFunctionType pinSix;
  interruptFunctionType pinSeven;
};
typedef struct functionPointersPortB functionPointersPortB;

functionPointersPortB portBFunctions = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

volatile uint8_t risingPinsPORTB=0;
volatile uint8_t fallingPinsPORTB=0;

// for the saved state of the ports
static volatile uint8_t portSnapshotB;
#endif

#ifndef EI_NOTPORTJ
// only 7 pins total of port J are supported as interrupts on the ATmega2560,
// and only PJ0 and 1 are supported on the Arduino MEGA.
// For PCI1 the 0th bit is PE0.   PJ2-6 are not exposed on the Arduino pins, but
// we will support them anyway. There are clones that provide them, and users may
// solder in their own connections (...go, Makers!)
struct functionPointersPortJ {
  interruptFunctionType pinZero;
  interruptFunctionType pinOne;
  interruptFunctionType pinTwo;
  interruptFunctionType pinThree;
  interruptFunctionType pinFour;
  interruptFunctionType pinFive;
  interruptFunctionType pinSix;
};
typedef struct functionPointersPortJ functionPointersPortJ;

functionPointersPortJ portJFunctions = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };

// For Pin Change Interrupts; since we're duplicating FALLING and RISING in software,
// we have to know how we were defined.
volatile uint8_t risingPinsPORTJ=0;
volatile uint8_t fallingPinsPORTJ=0;

// for the saved state of the ports
static volatile uint8_t portSnapshotJ;
#endif

#ifndef EI_NOTPORTK
struct functionPointersPortK {
  interruptFunctionType pinZero;
  interruptFunctionType pinOne;
  interruptFunctionType pinTwo;
  interruptFunctionType pinThree;
  interruptFunctionType pinFour;
  interruptFunctionType pinFive;
  interruptFunctionType pinSix;
  interruptFunctionType pinSeven;
};
typedef struct functionPointersPortK functionPointersPortK;

functionPointersPortK portKFunctions = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

volatile uint8_t risingPinsPORTK=0;
volatile uint8_t fallingPinsPORTK=0;

// for the saved state of the ports
static volatile uint8_t portSnapshotK;
#endif
#endif // NEEDFORSPEED

#define PORTB_VECT PCINT0_vect
#define PORTJ_VECT PCINT1_vect
#define PORTK_VECT PCINT2_vect

/* LEONARDO ***************************************************************************/
/* LEONARDO ***************************************************************************/
/* LEONARDO ***************************************************************************/
#elif defined __AVR_ATmega32U4__ || defined __AVR_ATmega16U4__
#define ARDUINO_LEONARDO
#if defined EI_NOTPINCHANGE
#ifndef EI_NOTPORTB
#define EI_NOTPORTB
#endif
#endif

#ifndef NEEDFORSPEED
#define ARDUINO_PIN_B0 17
#define ARDUINO_PIN_B1 15
#define ARDUINO_PIN_B2 16
#define ARDUINO_PIN_B3 14
#define ARDUINO_PIN_B4 8
#define ARDUINO_PIN_B5 9
#define ARDUINO_PIN_B6 10
#define ARDUINO_PIN_B7 11
#define ARDUINO_PIN_D0 3
#define ARDUINO_PIN_D1 2
#define ARDUINO_PIN_D2 0
#define ARDUINO_PIN_D3 1
#define ARDUINO_PIN_E6 7

/* To derive this list: 
   sed -n -e '1,/digital_pin_to_port_PGM/d' -e '/^}/,$d' -e '/P/p' \
       /usr/share/arduino/hardware/arduino/variants/leonardo/pins_arduino.h | \
       awk '{print "  ", $5 ", // " $5 "  pin: " $3}'
   ...then massage the output as necessary to create the below:
*/

const uint8_t PROGMEM digital_pin_to_port_bit_number_PGM[] = {
  2, // PD2  pin: D0
  3, // PD3  pin: D1
  1, // PD1  pin: D2
  0, // PD0  pin: D3
  4, // PD4  pin: D4
  6, // PC6  pin: D5
  7, // PD7  pin: D6
  6, // PE6  pin: D7
  4, // PB4  pin: D8  // we really only care about Port B, but I don't know that
  5, // PB5  pin: D9  // shortening this array and doing array index arithmetic
  6, // PB6  pin: D10 // would make the code any shorter.
  7, // PB7  pin: D11
  6, // PD6  pin: D12
  7, // PC7  pin: D13
  3, // PB3  pin: D14 MISO
  1, // PB1  pin: D15 SCK
  2, // PB2  pin: D16 MOSI
  0, // PB0  pin: D17 SS (RXLED). Available on non-Leonardo 32u4 boards, at least (exposed on the Leonardo??)
// There are no ports we care about after pin 17.
};


#if ! defined(EI_NOTEXTERNAL) && ! defined(EI_NOTINT0) && ! defined(EI_NOTINT1) && ! defined(EI_NOTINT2) && ! defined(EI_NOTINT3) && ! defined(EI_NOTINT6)
interruptFunctionType functionPointerArrayEXTERNAL[5];
#endif

#ifndef EI_NOTPORTB
struct functionPointersPortB {
  interruptFunctionType pinZero;
  interruptFunctionType pinOne;
  interruptFunctionType pinTwo;
  interruptFunctionType pinThree;
  interruptFunctionType pinFour;
  interruptFunctionType pinFive;
  interruptFunctionType pinSix;
  interruptFunctionType pinSeven;
};
typedef struct functionPointersPortB functionPointersPortB;

functionPointersPortB portBFunctions = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

// For Pin Change Interrupts; since we're duplicating FALLING and RISING in software,
// we have to know how we were defined.
volatile uint8_t risingPinsPORTB=0;
volatile uint8_t fallingPinsPORTB=0;

// for the saved state of the ports
static volatile uint8_t portSnapshotB;
#endif
#endif // NEEDFOR SPEED

#define PORTB_VECT PCINT0_vect

/* 644/1284 ***************************************************************************/
/* 644/1284 ***************************************************************************/
/* 644/1284 ***************************************************************************/
#elif defined __AVR_ATmega1284P__ || defined __AVR_ATmega1284__ || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)
#define MIGHTY1284
#if defined EI_NOTPINCHANGE
#ifndef EI_NOTPORTA
#define EI_NOTPORTA
#endif
#ifndef EI_NOTPORTB
#define EI_NOTPORTB
#endif
#ifndef EI_NOTPORTC
#define EI_NOTPORTC
#endif
#ifndef EI_NOTPORTD
#define EI_NOTPORTD
#endif
#endif

#ifndef INPUT_PULLUP
#define INPUT_PULLUP 0x2
#endif

#ifndef NEEDFORSPEED
#define ARDUINO_PIN_A0 24
#define ARDUINO_PIN_A1 25
#define ARDUINO_PIN_A2 26
#define ARDUINO_PIN_A3 27
#define ARDUINO_PIN_A4 28
#define ARDUINO_PIN_A5 29
#define ARDUINO_PIN_A6 30
#define ARDUINO_PIN_A7 31
#define ARDUINO_PIN_B0 0
#define ARDUINO_PIN_B1 1
#define ARDUINO_PIN_B2 2
#define ARDUINO_PIN_B3 3
#define ARDUINO_PIN_B4 4
#define ARDUINO_PIN_B5 5
#define ARDUINO_PIN_B6 6
#define ARDUINO_PIN_B7 7
#define ARDUINO_PIN_C0 16
#define ARDUINO_PIN_C1 17
#define ARDUINO_PIN_C2 18
#define ARDUINO_PIN_C3 19
#define ARDUINO_PIN_C4 20
#define ARDUINO_PIN_C5 21
#define ARDUINO_PIN_C6 22
#define ARDUINO_PIN_C7 23
#define ARDUINO_PIN_D0 8
#define ARDUINO_PIN_D1 9
#define ARDUINO_PIN_D2 10
#define ARDUINO_PIN_D3 11
#define ARDUINO_PIN_D4 12
#define ARDUINO_PIN_D5 13
#define ARDUINO_PIN_D6 14
#define ARDUINO_PIN_D7 15

const uint8_t PROGMEM digital_pin_to_port_bit_number_PGM[] = {
  0, // 0 == port B, 0
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  0, // 8 == port D, 0
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  0, // 16 == port C, 0
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  0, // 24 == port A, 0
  1,
  2,
  3,
  4,
  5,
  6,
  7,
};


#if ! defined(EI_NOTEXTERNAL) && ! defined(EI_NOTINT0) && ! defined(EI_NOTINT1) && ! defined(EI_NOTINT2)
interruptFunctionType functionPointerArrayEXTERNAL[3];
#endif

struct functionPointers {
  interruptFunctionType pinZero;
  interruptFunctionType pinOne;
  interruptFunctionType pinTwo;
  interruptFunctionType pinThree;
  interruptFunctionType pinFour;
  interruptFunctionType pinFive;
  interruptFunctionType pinSix;
  interruptFunctionType pinSeven;
};

#ifndef EI_NOTPORTA
typedef struct functionPointers functionPointersPortA;
functionPointers portAFunctions = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
// For Pin Change Interrupts; since we're duplicating FALLING and RISING in software,
// we have to know how the ports were defined.
volatile uint8_t risingPinsPORTA=0;
volatile uint8_t fallingPinsPORTA=0;
// for the saved state of the ports
static volatile uint8_t portSnapshotA;
#endif

#ifndef EI_NOTPORTB
typedef struct functionPointers functionPointersPortB;
functionPointersPortB portBFunctions = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
volatile uint8_t risingPinsPORTB=0;
volatile uint8_t fallingPinsPORTB=0;
static volatile uint8_t portSnapshotB;
#endif

#ifndef EI_NOTPORTC
typedef struct functionPointers functionPointersPortC;
functionPointersPortC portCFunctions = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
volatile uint8_t risingPinsPORTC=0;
volatile uint8_t fallingPinsPORTC=0;
static volatile uint8_t portSnapshotC;
#endif

#ifndef EI_NOTPORTD
typedef struct functionPointers functionPointersPortD;
functionPointersPortD portDFunctions = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
volatile uint8_t risingPinsPORTD=0;
volatile uint8_t fallingPinsPORTD=0;
static volatile uint8_t portSnapshotD;
#endif
#endif // NEEDFORSPEED



// the vectors (eg, "PCINT0_vect") are defined in the avr.h files, like iom1284p.h
#define PORTA_VECT PCINT0_vect
#define PORTB_VECT PCINT1_vect
#define PORTC_VECT PCINT2_vect
#define PORTD_VECT PCINT3_vect

#endif // #if defined __AVR_ATmega168__ || defined __AVR_ATmega168A__ ... (etc.) ...

// ===========================================================================================
// END END END DATA STRUCTURES ===============================================================
// ===========================================================================================

// From /usr/share/arduino/hardware/arduino/cores/robot/Arduino.h
// #define CHANGE 1
// #define FALLING 2
// #define RISING 3

// ===========================================================================================
// ===========================================================================================
// enableInterrupt(interrupDesignator, userFunction, mode); ==================================
// ===========================================================================================
// ===========================================================================================

// "interruptDesignator" is simply the Arduino pin optionally OR'ed with
// PINCHANGEINTERRUPT (== 0x80)
void enableInterrupt(uint8_t interruptDesignator, interruptFunctionType userFunction, uint8_t mode) {
  uint8_t arduinoPin;
  uint8_t portNumber=0;
  uint8_t portMask=0;
#ifndef NEEDFORSPEED
  uint8_t portBitNumber; // when an interrupted pin is found, this will be used to choose the function.
  interruptFunctionType *calculatedPointer;
#endif

  arduinoPin=interruptDesignator & ~PINCHANGEINTERRUPT;

  // *************************************************************************************
  // *************************************************************************************
  // Pin Change Interrupts
  // *************************************************************************************
  // *************************************************************************************
#if defined ARDUINO_328
  if ( (interruptDesignator & PINCHANGEINTERRUPT) || (arduinoPin != 2 && arduinoPin != 3) ) {
#elif defined MIGHTY1284
  if ( (interruptDesignator & PINCHANGEINTERRUPT) || (arduinoPin != 2 && arduinoPin != 10 &&
                                                      arduinoPin != 11) ) {
#elif defined ARDUINO_LEONARDO
  if ( (arduinoPin > 3) && (arduinoPin != 7) ) {
#endif
#if defined ARDUINO_328 || defined MIGHTY1284 || defined ARDUINO_LEONARDO
    portMask=pgm_read_byte(&digital_pin_to_bit_mask_PGM[arduinoPin]);
    portNumber=pgm_read_byte(&digital_pin_to_port_PGM[arduinoPin]);
#elif defined ARDUINO_MEGA
  // NOTE: PJ2-6 and PE6 & 7 are not exposed on the Arduino, but they are supported here
  // for software interrupts and support of non-Arduino platforms which expose more pins.
  // PJ2-6 are called pins 70-74, PE6 is pin 75, PE7 is pin 76.
  if ( (arduinoPin != 2 && arduinoPin != 3 && arduinoPin != 75 && arduinoPin != 76
                                           && (arduinoPin < 18 || arduinoPin > 21))
     ) {
    if (arduinoPin > 69) { // Dastardly tricks to support PortJ 2-7
      portMask=pgm_read_byte(&digital_pin_to_bit_mask_PGM[arduinoPin-6]); // Steal from PK
      portNumber=PJ;
    } else {
      portMask=pgm_read_byte(&digital_pin_to_bit_mask_PGM[arduinoPin]);
      portNumber=pgm_read_byte(&digital_pin_to_port_PGM[arduinoPin]);
    }
#else
#error Unsupported Arduino platform
#endif

    // *************************************************************************************
    // Store the rising/falling pins
    // *************************************************************************************

    if ((mode == RISING) || (mode == CHANGE)) {
#define EI_SECTION_RISING

#if defined MIGHTY1284
#include "utility/ei_PinChange1284.h"
#elif defined ARDUINO_328
#include "utility/ei_PinChange328.h"
#elif defined ARDUINO_MEGA
#include "utility/ei_PinChange2560.h"
#elif defined ARDUINO_LEONARDO
#include "utility/ei_PinChangeLeonardo.h"
#endif
#undef EI_SECTION_RISING
    }

    if ((mode == FALLING) || (mode == CHANGE)) {
#define EI_SECTION_FALLING
#if defined MIGHTY1284
#include "utility/ei_PinChange1284.h"
#elif defined ARDUINO_328
#include "utility/ei_PinChange328.h"
#elif defined ARDUINO_MEGA
#include "utility/ei_PinChange2560.h"
#elif defined ARDUINO_LEONARDO
#include "utility/ei_PinChangeLeonardo.h"
#endif
#undef EI_SECTION_FALLING
    }

#ifndef NEEDFORSPEED
    portBitNumber=pgm_read_byte(&digital_pin_to_port_bit_number_PGM[arduinoPin]);
#endif

    // *************************************************************************************
    // assign the function to be run in the ISR
    // save the initial value of the port
    // initialize interrupt registers PCMSKx and PCICR
    // *************************************************************************************

#define EI_SECTION_ASSIGNFUNCTIONSREGISTERS
#if defined MIGHTY1284
#include "utility/ei_PinChange1284.h"
#elif defined ARDUINO_328
#include "utility/ei_PinChange328.h"
#elif defined ARDUINO_MEGA
#include "utility/ei_PinChange2560.h"
#elif defined ARDUINO_LEONARDO
#include "utility/ei_PinChangeLeonardo.h"
#endif
#undef EI_SECTION_ASSIGNFUNCTIONSREGISTERS

    *pcmsk |= portMask;  // appropriate PCMSKn bit is set, e.g. this could be PCMSK1 |= portMask;

    // With the exception of the Global Interrupt Enable bit in SREG, interrupts on the arduinoPin
    // are now ready. GIE may have already been set on a previous enable, so it's important
    // to take note of the order in which things were done, above.

  // *************************************************************************************
  // *************************************************************************************
  // External Interrupts
  // *************************************************************************************
  // *************************************************************************************
  } else {
#ifndef EI_NOTEXTERNAL
    uint8_t origSREG; // to save for interrupts
    origSREG = SREG;
    cli(); // no interrupts while we're setting up an interrupt.

#define EI_SECTION_ENABLEEXTERNAL
#if defined MIGHTY1284
#include "utility/ei_External1284.h"
#elif defined ARDUINO_328
#include "utility/ei_External328.h"
#elif defined ARDUINO_MEGA
#include "utility/ei_External2560.h"
#elif defined ARDUINO_LEONARDO
#include "utility/ei_ExternalLeonardo.h"
#endif
#undef EI_SECTION_ENABLEEXTERNAL

    SREG=origSREG;
#endif
  }
  SREG |= (1 << SREG_I); // GIE bit in SREG. From /usr/avr/include/avr/common.h
}

// ===========================================================================================
// ===========================================================================================
// disableInterrupt(interrupDesignator); =====================================================
// ===========================================================================================
// ===========================================================================================
void disableInterrupt (uint8_t interruptDesignator) {

  uint8_t origSREG; // to save for interrupts
  uint8_t arduinoPin;
  uint8_t portNumber=0;
  uint8_t portMask=0;

  origSREG = SREG;
  cli();
  arduinoPin=interruptDesignator & ~PINCHANGEINTERRUPT;
#if defined ARDUINO_328
  if ( (interruptDesignator & PINCHANGEINTERRUPT) || (arduinoPin != 2 && arduinoPin != 3) ) {
#elif defined MIGHTY1284
  if ( (interruptDesignator & PINCHANGEINTERRUPT) || (arduinoPin != 2 && arduinoPin != 10 &&
                                                      arduinoPin != 11) ) {
#elif defined ARDUINO_LEONARDO
  if ( (arduinoPin > 3) && (arduinoPin != 7) ) {
#endif
#if defined ARDUINO_328 || defined MIGHTY1284 || defined ARDUINO_LEONARDO
    portMask=pgm_read_byte(&digital_pin_to_bit_mask_PGM[arduinoPin]);
    portNumber=pgm_read_byte(&digital_pin_to_port_PGM[arduinoPin]);
#elif defined ARDUINO_MEGA
  // NOTE: PJ2-6 and PE6 & 7 are not exposed on the Arduino, but they are supported here
  // for software interrupts and support of non-Arduino platforms which expose more pins.
  // PJ2-6 are called pins 70-74, PE6 is pin 75, PE7 is pin 76.
  if ( (arduinoPin != 2 && arduinoPin != 3 && arduinoPin != 75 && arduinoPin != 76
                                           && (arduinoPin < 18 || arduinoPin > 21))
     ) {
    if (arduinoPin > 69) { // Dastardly tricks to support PortJ 2-7
      portMask=pgm_read_byte(&digital_pin_to_bit_mask_PGM[arduinoPin-6]); // Steal from PK
      portNumber=PJ;
    } else {
      portMask=pgm_read_byte(&digital_pin_to_bit_mask_PGM[arduinoPin]);
      portNumber=pgm_read_byte(&digital_pin_to_port_PGM[arduinoPin]);
    }
#else
#error Unsupported Arduino platform
#endif

#define EI_SECTION_DISABLEPINCHANGE
#if defined MIGHTY1284
#include "utility/ei_PinChange1284.h"
#elif defined ARDUINO_328
#include "utility/ei_PinChange328.h"
#elif defined ARDUINO_MEGA
#include "utility/ei_PinChange2560.h"
#elif defined LEONARDO
#include "utility/ei_PinChangeLeonardo.h"
#endif
#undef EI_SECTION_DISABLEPINCHANGE
  } else {
#ifndef EI_NOTEXTERNAL
#define EI_SECTION_DISABLEEXTERNAL
#if defined MIGHTY1284
#include "utility/ei_External1284.h"
#elif defined ARDUINO_328
#include "utility/ei_External328.h"
#elif defined ARDUINO_MEGA
#include "utility/ei_External2560.h"
#elif defined ARDUINO_LEONARDO
#include "utility/ei_ExternalLeonardo.h"
#endif
#undef EI_SECTION_DISABLEEXTERNAL
#endif
  }
  SREG = origSREG;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////// ISRs /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef EI_NOTEXTERNAL
#ifndef EI_NOTINT0
ISR(INT0_vect) {
#ifndef NEEDFORSPEED
#ifdef EI_ARDUINO_INTERRUPTED_PIN
#if defined MIGHTY1284
  arduinoInterruptedPin=ARDUINO_PIN_D2;
#elif defined ARDUINO_MEGA
  arduinoInterruptedPin=ARDUINO_PIN_D0;
#elif defined ARDUINO_LEONARDO
  arduinoInterruptedPin=ARDUINO_PIN_D0;
#elif defined ARDUINO_328
  arduinoInterruptedPin=ARDUINO_PIN_D2;
#endif
#endif // EI_ARDUINO_INTERRUPTED_PIN
  (*functionPointerArrayEXTERNAL[0])();
#else
#if defined MIGHTY1284
  INTERRUPT_FLAG_PIN10++ 
#endif
#if defined ARDUINO_MEGA
#ifdef INTERRUPT_FLAG_PIN21
  INTERRUPT_FLAG_PIN21++;
#endif
#endif
#if defined ARDUINO_LEONARDO
#ifdef INTERRUPT_FLAG_PIN3
  INTERRUPT_FLAG_PIN3++;
#endif
#endif
#if defined ARDUINO_328
#ifdef INTERRUPT_FLAG_PIN2
  INTERRUPT_FLAG_PIN2++;
#endif
#endif
#endif // NEEDFORSPEED
}
#endif // EI_NOTINT0

#ifndef EI_NOTINT1
ISR(INT1_vect) {
#ifndef NEEDFORSPEED
#ifdef EI_ARDUINO_INTERRUPTED_PIN
#if defined MIGHTY1284
  arduinoInterruptedPin=ARDUINO_PIN_D3;
#elif defined ARDUINO_MEGA
  arduinoInterruptedPin=ARDUINO_PIN_D1;
#elif defined ARDUINO_LEONARDO
  arduinoInterruptedPin=ARDUINO_PIN_D1;
#elif defined ARDUINO_328
  arduinoInterruptedPin=ARDUINO_PIN_D3;
#endif
#endif // EI_ARDUINO_INTERRUPTED_PIN
  (*functionPointerArrayEXTERNAL[1])();
#else
#if defined MIGHTY1284
  INTERRUPT_FLAG_PIN11++ 
#endif
#if defined ARDUINO_MEGA
#ifdef INTERRUPT_FLAG_PIN20
  INTERRUPT_FLAG_PIN20++;
#endif
#endif
#if defined ARDUINO_LEONARDO
#ifdef INTERRUPT_FLAG_PIN2
  INTERRUPT_FLAG_PIN2++;
#endif
#endif
#if defined ARDUINO_328
#ifdef INTERRUPT_FLAG_PIN3
  INTERRUPT_FLAG_PIN3++;
#endif
#endif
#endif // NEEDFORSPEED
}
#endif // EI_NOTINT1

#if defined ARDUINO_MEGA || defined ARDUINO_LEONARDO || defined MIGHTY1284
#ifndef EI_NOTINT2
ISR(INT2_vect) {
#ifndef NEEDFORSPEED
#ifdef EI_ARDUINO_INTERRUPTED_PIN
#if defined MIGHTY1284
  arduinoInterruptedPin=ARDUINO_PIN_B2;
#elif defined ARDUINO_MEGA
  arduinoInterruptedPin=ARDUINO_PIN_D2;
#elif defined ARDUINO_LEONARDO
  arduinoInterruptedPin=ARDUINO_PIN_D2;
#endif
#endif // EI_ARDUINO_INTERRUPTED_PIN
  (*functionPointerArrayEXTERNAL[2])();
#else
#if defined MIGHTY1284
  INTERRUPT_FLAG_PIN2++ 
#endif
#if defined ARDUINO_MEGA
#ifdef INTERRUPT_FLAG_PIN19
  INTERRUPT_FLAG_PIN19++;
#endif
#else
#ifdef INTERRUPT_FLAG_PIN0
  INTERRUPT_FLAG_PIN0++;
#endif
#endif
#endif // NEEDFORSPEED
}
#endif // EI_NOTINT2
#endif // ARDUINO_MEGA || ARDUINO_LEONARDO || MIGHTY1284

#if defined ARDUINO_MEGA || defined ARDUINO_LEONARDO
#ifndef EI_NOTINT3
ISR(INT3_vect) {
#ifndef NEEDFORSPEED
#ifdef EI_ARDUINO_INTERRUPTED_PIN
#if defined ARDUINO_MEGA
  arduinoInterruptedPin=ARDUINO_PIN_D3;
#elif defined ARDUINO_LEONARDO
  arduinoInterruptedPin=ARDUINO_PIN_D3;
#endif
#endif // EI_ARDUINO_INTERRUPTED_PIN
  (*functionPointerArrayEXTERNAL[3])();
#else
#if defined ARDUINO_MEGA
#ifdef INTERRUPT_FLAG_PIN18
  INTERRUPT_FLAG_PIN18++;
#endif
#else
#ifdef INTERRUPT_FLAG_PIN1
  INTERRUPT_FLAG_PIN1++;
#endif
#endif
#endif // NEEDFORSPEED
}
#endif // EI_NOTINT3
#endif // ARDUINO_MEGA || ARDUINO_LEONARDO

#if defined ARDUINO_MEGA
#ifndef EI_NOTINT4
ISR(INT4_vect) {
#ifndef NEEDFORSPEED
#ifdef EI_ARDUINO_INTERRUPTED_PIN
  arduinoInterruptedPin=ARDUINO_PIN_E4;
#endif // EI_ARDUINO_INTERRUPTED_PIN
  (*functionPointerArrayEXTERNAL[4])();
#else
#ifdef INTERRUPT_FLAG_PIN2
  INTERRUPT_FLAG_PIN2++;
#endif
#endif // NEEDFORSPEED
}
#endif // EI_NOTINT4

#ifndef EI_NOTINT5
ISR(INT5_vect) {
#ifndef NEEDFORSPEED
#ifdef EI_ARDUINO_INTERRUPTED_PIN
  arduinoInterruptedPin=ARDUINO_PIN_E5;
#endif // EI_ARDUINO_INTERRUPTED_PIN
  (*functionPointerArrayEXTERNAL[5])();
#else
#ifdef INTERRUPT_FLAG_PIN3
  INTERRUPT_FLAG_PIN3++;
#endif
#endif // NEEDFORSPEED
}
#endif // EI_NOTINT5

#ifndef EI_NOTINT6
ISR(INT6_vect) {
#ifndef NEEDFORSPEED
#ifdef EI_ARDUINO_INTERRUPTED_PIN
  arduinoInterruptedPin=ARDUINO_PIN_E6;
#endif // EI_ARDUINO_INTERRUPTED_PIN
  (*functionPointerArrayEXTERNAL[6])();
#else
#ifdef INTERRUPT_FLAG_PIN75
  INTERRUPT_FLAG_PIN75++;
#endif
#endif // NEEDFORSPEED
}
#endif // EI_NOTINT6

#ifndef EI_NOTINT7
ISR(INT7_vect) {
#ifndef NEEDFORSPEED
#ifdef EI_ARDUINO_INTERRUPTED_PIN
  arduinoInterruptedPin=ARDUINO_PIN_E7;
#endif // EI_ARDUINO_INTERRUPTED_PIN
  (*functionPointerArrayEXTERNAL[7])();
#else
#ifdef INTERRUPT_FLAG_PIN76
  INTERRUPT_FLAG_PIN76++;
#endif
#endif // NEEDFORSPEED
}
#endif // EI_NOTINT7
#endif // defined ARDUINO_MEGA

#if defined ARDUINO_LEONARDO
#ifndef EI_NOTINT6
ISR(INT6_vect) {
#ifndef NEEDFORSPEED
#ifdef EI_ARDUINO_INTERRUPTED_PIN
  arduinoInterruptedPin=ARDUINO_PIN_E6;
#endif // EI_ARDUINO_INTERRUPTED_PIN
  (*functionPointerArrayEXTERNAL[4])();
#else
#ifdef INTERRUPT_FLAG_PIN7
  INTERRUPT_FLAG_PIN7++;
#endif
#endif // NEEDFORSPEED
}
#endif // EI_NOTINT6
#endif // defined ARDUINO_LEONARDO
#endif // EI_NOTEXTERNAL

#if defined MIGHTY1284
#ifndef EI_NOTPORTA
ISR(PORTA_VECT) {
  uint8_t current;
  uint8_t interruptMask;
  uint8_t changedPins;
  uint8_t tmp;

  current=PINA;
// If we trust the compiler to do this, it will use an extra register...
//  changedPins=(portSnapshotA ^ current) &
//                                       ((risingPinsPORTA & current) | (fallingPinsPORTA & ~current));
// ...so we do it ourselves:
  changedPins   = portSnapshotA ^ current;
  tmp           = risingPinsPORTA & current;
  interruptMask = fallingPinsPORTA & ~current;
  interruptMask = interruptMask | tmp;
  interruptMask = changedPins & interruptMask;
  interruptMask = PCMSK0 & interruptMask;


  portSnapshotA = current;
#ifdef NEEDFORSPEED
#include "utility/ei_porta_speed.h"
#else
  if (interruptMask == 0) goto exitPORTAISR; // get out quickly if not interested.
#ifdef EI_ARDUINO_INTERRUPTED_PIN
  if (interruptMask & _BV(0)) { arduinoInterruptedPin=ARDUINO_PIN_A0; portAFunctions.pinZero(); }
  if (interruptMask & _BV(1)) { arduinoInterruptedPin=ARDUINO_PIN_A1; portAFunctions.pinOne(); }
  if (interruptMask & _BV(2)) { arduinoInterruptedPin=ARDUINO_PIN_A2; portAFunctions.pinTwo(); }
  if (interruptMask & _BV(3)) { arduinoInterruptedPin=ARDUINO_PIN_A3; portAFunctions.pinThree(); }
  if (interruptMask & _BV(4)) { arduinoInterruptedPin=ARDUINO_PIN_A4; portAFunctions.pinFour(); }
  if (interruptMask & _BV(5)) { arduinoInterruptedPin=ARDUINO_PIN_A5; portAFunctions.pinFive(); }
  if (interruptMask & _BV(6)) { arduinoInterruptedPin=ARDUINO_PIN_A6; portAFunctions.pinSix(); }
  if (interruptMask & _BV(7)) { arduinoInterruptedPin=ARDUINO_PIN_A7; portAFunctions.pinSeven(); }
#else
  if (interruptMask & _BV(0)) portAFunctions.pinZero();
  if (interruptMask & _BV(1)) portAFunctions.pinOne();
  if (interruptMask & _BV(2)) portAFunctions.pinTwo();
  if (interruptMask & _BV(3)) portAFunctions.pinThree();
  if (interruptMask & _BV(4)) portAFunctions.pinFour();
  if (interruptMask & _BV(5)) portAFunctions.pinFive();
  if (interruptMask & _BV(6)) portAFunctions.pinSix();
  if (interruptMask & _BV(7)) portAFunctions.pinSeven();
#endif // EI_ARDUINO_INTERRUPTED_PIN
  exitPORTAISR: return;
#endif // NEEDFORSPEED
  // FOR MEASUREMENT ONLY
  // exitPORTBISR: PORTC &= ~(1 << PC5); // SIGNAL THAT WE ARE LEAVING THE INTERRUPT
}
#endif // EI_NOTPORTA
#endif // MIGHTY1284

#ifndef EI_NOTPORTB
ISR(PORTB_VECT) {
  uint8_t current;
  uint8_t interruptMask;
  uint8_t changedPins;
  uint8_t tmp;

  current=PINB;
// If we trust the compiler to do this, it will use an extra register...
//  changedPins=(portSnapshotB ^ current) &
//                                       ((risingPinsPORTB & current) | (fallingPinsPORTB & ~current));
// ...so we do it ourselves:
  changedPins   = portSnapshotB ^ current;
  tmp           = risingPinsPORTB & current;
  interruptMask = fallingPinsPORTB & ~current;
  interruptMask = interruptMask | tmp;
  interruptMask = changedPins & interruptMask;
#ifdef MIGHTY1284
  interruptMask = PCMSK1 & interruptMask;
#else
  interruptMask = PCMSK0 & interruptMask;
#endif

  portSnapshotB = current;
#ifdef NEEDFORSPEED
#include "utility/ei_portb_speed.h"
#else
  if (interruptMask == 0) goto exitPORTBISR; // get out quickly if not interested.
#ifdef EI_ARDUINO_INTERRUPTED_PIN
  if (interruptMask & _BV(0)) { arduinoInterruptedPin=ARDUINO_PIN_B0; portBFunctions.pinZero(); }
  if (interruptMask & _BV(1)) { arduinoInterruptedPin=ARDUINO_PIN_B1; portBFunctions.pinOne(); }
  if (interruptMask & _BV(2)) { arduinoInterruptedPin=ARDUINO_PIN_B2; portBFunctions.pinTwo(); }
  if (interruptMask & _BV(3)) { arduinoInterruptedPin=ARDUINO_PIN_B3; portBFunctions.pinThree(); }
  if (interruptMask & _BV(4)) { arduinoInterruptedPin=ARDUINO_PIN_B4; portBFunctions.pinFour(); }
  if (interruptMask & _BV(5)) { arduinoInterruptedPin=ARDUINO_PIN_B5; portBFunctions.pinFive(); }
#ifndef ARDUINO_328
  if (interruptMask & _BV(6)) { arduinoInterruptedPin=ARDUINO_PIN_B6; portBFunctions.pinSix(); }
  if (interruptMask & _BV(7)) { arduinoInterruptedPin=ARDUINO_PIN_B7; portBFunctions.pinSeven(); }
#endif
#else // EI_ARDUINO_INTERRUPTED_PIN
  if (interruptMask & _BV(0)) portBFunctions.pinZero();
  if (interruptMask & _BV(1)) portBFunctions.pinOne();
  if (interruptMask & _BV(2)) portBFunctions.pinTwo();
  if (interruptMask & _BV(3)) portBFunctions.pinThree();
  if (interruptMask & _BV(4)) portBFunctions.pinFour();
  if (interruptMask & _BV(5)) portBFunctions.pinFive();
#ifndef ARDUINO_328
  if (interruptMask & _BV(6)) portBFunctions.pinSix();
  if (interruptMask & _BV(7)) portBFunctions.pinSeven();
#endif
#endif // EI_ARDUINO_INTERRUPTED_PIN
  exitPORTBISR: return;
  // FOR MEASUREMENT ONLY
  // exitPORTBISR: PORTC &= ~(1 << PC5); // SIGNAL THAT WE ARE LEAVING THE INTERRUPT
#endif // NEEDFORSPEED
}
#endif // EI_NOTPORTB

#if defined ARDUINO_328 || defined MIGHTY1284
#ifndef EI_NOTPORTC
ISR(PORTC_VECT) {
  uint8_t current;
  uint8_t interruptMask;
  uint8_t changedPins;
  uint8_t tmp;

  current=PINC;
// If we trust the compiler to do this, it will use an extra register...
//  changedPins=(portSnapshotB ^ current) &
//                                       ((risingPinsPORTB & current) | (fallingPinsPORTB & ~current));
// ...so we do it ourselves:
  changedPins   = portSnapshotC ^ current;
  tmp           = risingPinsPORTC & current;
  interruptMask = fallingPinsPORTC & ~current;
  interruptMask = interruptMask | tmp;
  interruptMask = changedPins & interruptMask;
  interruptMask = PCMSK1 & interruptMask;

  portSnapshotC = current;
#ifdef NEEDFORSPEED
#include "utility/ei_portc_speed.h"
#else
  if (interruptMask == 0) goto exitPORTCISR; // get out quickly if not interested.
#ifdef EI_ARDUINO_INTERRUPTED_PIN
  if (interruptMask & _BV(0)) { arduinoInterruptedPin=ARDUINO_PIN_C0; portCFunctions.pinZero(); }
  if (interruptMask & _BV(1)) { arduinoInterruptedPin=ARDUINO_PIN_C1; portCFunctions.pinOne(); }
  if (interruptMask & _BV(2)) { arduinoInterruptedPin=ARDUINO_PIN_C2; portCFunctions.pinTwo(); }
  if (interruptMask & _BV(3)) { arduinoInterruptedPin=ARDUINO_PIN_C3; portCFunctions.pinThree(); }
  if (interruptMask & _BV(4)) { arduinoInterruptedPin=ARDUINO_PIN_C4; portCFunctions.pinFour(); }
  if (interruptMask & _BV(5)) { arduinoInterruptedPin=ARDUINO_PIN_C5; portCFunctions.pinFive(); }
#ifdef MIGHTY1284
  if (interruptMask & _BV(6)) { arduinoInterruptedPin=ARDUINO_PIN_C6; portCFunctions.pinSix(); }
  if (interruptMask & _BV(7)) { arduinoInterruptedPin=ARDUINO_PIN_C7; portCFunctions.pinSeven(); }
#endif
#else
  if (interruptMask & _BV(0)) portCFunctions.pinZero();
  if (interruptMask & _BV(1)) portCFunctions.pinOne();
  if (interruptMask & _BV(2)) portCFunctions.pinTwo();
  if (interruptMask & _BV(3)) portCFunctions.pinThree();
  if (interruptMask & _BV(4)) portCFunctions.pinFour();
  if (interruptMask & _BV(5)) portCFunctions.pinFive();
#ifdef MIGHTY1284
  if (interruptMask & _BV(6)) portCFunctions.pinSix();
  if (interruptMask & _BV(7)) portCFunctions.pinSeven();
#endif
#endif // EI_ARDUINO_INTERRUPTED_PIN
  exitPORTCISR: return;
#endif // NEEDFORSPEED
}
#endif // EI_NOTPORTC

#ifndef EI_NOTPORTD
ISR(PORTD_VECT) {
  uint8_t current;
  uint8_t interruptMask;
  uint8_t changedPins;
  uint8_t tmp;

  current=PIND;
// If we trust the compiler to do this, it will use an extra register...
//  changedPins=(portSnapshotB ^ current) &
//                                       ((risingPinsPORTB & current) | (fallingPinsPORTB & ~current));
// ...so we do it ourselves:
  changedPins   = portSnapshotD ^ current;
  tmp           = risingPinsPORTD & current;
  interruptMask = fallingPinsPORTD & ~current;
  interruptMask = interruptMask | tmp;
  interruptMask = changedPins & interruptMask;
  interruptMask = PCMSK2 & interruptMask;


  portSnapshotD = current;
#ifdef NEEDFORSPEED
#include "utility/ei_portd_speed.h"
#else
  if (interruptMask == 0) goto exitPORTDISR; // get out quickly if not interested.
#ifdef EI_ARDUINO_INTERRUPTED_PIN
  if (interruptMask & _BV(0)) { arduinoInterruptedPin=ARDUINO_PIN_D0; portDFunctions.pinZero(); }
  if (interruptMask & _BV(1)) { arduinoInterruptedPin=ARDUINO_PIN_D1; portDFunctions.pinOne(); }
  if (interruptMask & _BV(2)) { arduinoInterruptedPin=ARDUINO_PIN_D2; portDFunctions.pinTwo(); }
  if (interruptMask & _BV(3)) { arduinoInterruptedPin=ARDUINO_PIN_D3; portDFunctions.pinThree(); }
  if (interruptMask & _BV(4)) { arduinoInterruptedPin=ARDUINO_PIN_D4; portDFunctions.pinFour(); }
  if (interruptMask & _BV(5)) { arduinoInterruptedPin=ARDUINO_PIN_D5; portDFunctions.pinFive(); }
  if (interruptMask & _BV(6)) { arduinoInterruptedPin=ARDUINO_PIN_D6; portDFunctions.pinSix(); }
  if (interruptMask & _BV(7)) { arduinoInterruptedPin=ARDUINO_PIN_D7; portDFunctions.pinSeven(); }
#else
  if (interruptMask & _BV(0)) portDFunctions.pinZero();
  if (interruptMask & _BV(1)) portDFunctions.pinOne();
  if (interruptMask & _BV(2)) portDFunctions.pinTwo();
  if (interruptMask & _BV(3)) portDFunctions.pinThree();
  if (interruptMask & _BV(4)) portDFunctions.pinFour();
  if (interruptMask & _BV(5)) portDFunctions.pinFive();
  if (interruptMask & _BV(6)) portDFunctions.pinSix();
  if (interruptMask & _BV(7)) portDFunctions.pinSeven();
#endif // EI_ARDUINO_INTERRUPTED_PIN
  exitPORTDISR: return;
#endif // NEEDFORSPEED
}
#endif // EI_NOTPORTD

#elif defined ARDUINO_MEGA
#ifndef EI_NOTPORTJ
ISR(PORTJ_VECT) {
  uint8_t current;
  uint8_t interruptMask;
  uint8_t changedPins;
  uint8_t tmp;

  current=PINJ;
// If we trust the compiler to do this, it will use an extra register...
//  changedPins=(portSnapshotB ^ current) &
//                                       ((risingPinsPORTB & current) | (fallingPinsPORTB & ~current));
// ...so we do it ourselves:
  changedPins   = portSnapshotJ ^ current;
  tmp           = risingPinsPORTJ & current;
  interruptMask = fallingPinsPORTJ & ~current;
  interruptMask = interruptMask | tmp;
  interruptMask = changedPins & interruptMask;
  interruptMask = portJPCMSK & interruptMask; // because PCMSK1 is shifted wrt. PortJ.

  portSnapshotJ = current;
#ifdef NEEDFORSPEED
#include "utility/ei_portj_speed.h"
#else
  if (interruptMask == 0) goto exitPORTJISR; // get out quickly if not interested.
#ifdef EI_ARDUINO_INTERRUPTED_PIN
  if (interruptMask & _BV(0)) { arduinoInterruptedPin=ARDUINO_PIN_J0; portJFunctions.pinZero(); }
  if (interruptMask & _BV(1)) { arduinoInterruptedPin=ARDUINO_PIN_J1; portJFunctions.pinOne(); }
  if (interruptMask & _BV(2)) { arduinoInterruptedPin=ARDUINO_PIN_J2; portJFunctions.pinTwo(); }
  if (interruptMask & _BV(3)) { arduinoInterruptedPin=ARDUINO_PIN_J3; portJFunctions.pinThree(); }
  if (interruptMask & _BV(4)) { arduinoInterruptedPin=ARDUINO_PIN_J4; portJFunctions.pinFour(); }
  if (interruptMask & _BV(5)) { arduinoInterruptedPin=ARDUINO_PIN_J5; portJFunctions.pinFive(); }
  if (interruptMask & _BV(6)) { arduinoInterruptedPin=ARDUINO_PIN_J6; portJFunctions.pinSix(); }
#else
  if (interruptMask & _BV(0)) portJFunctions.pinZero();
  if (interruptMask & _BV(1)) portJFunctions.pinOne();
  if (interruptMask & _BV(2)) portJFunctions.pinTwo();
  if (interruptMask & _BV(3)) portJFunctions.pinThree();
  if (interruptMask & _BV(4)) portJFunctions.pinFour();
  if (interruptMask & _BV(5)) portJFunctions.pinFive();
  if (interruptMask & _BV(6)) portJFunctions.pinSix();
#endif // EI_ARDUINO_INTERRUPTED_PIN
  exitPORTJISR: return;
#endif // NEEDFORSPEED
}
#endif // EI_NOTPORTJ

#ifndef EI_NOTPORTK
ISR(PORTK_VECT) {
  uint8_t current;
  uint8_t interruptMask;
  uint8_t changedPins;
  uint8_t tmp;

  current=PINK;
// If we trust the compiler to do this, it will use an extra register...
//  changedPins=(portSnapshotB ^ current) &
//                                       ((risingPinsPORTB & current) | (fallingPinsPORTB & ~current));
// ...so we do it ourselves:
  changedPins   = portSnapshotK ^ current;
  tmp           = risingPinsPORTK & current;
  interruptMask = fallingPinsPORTK & ~current;
  interruptMask = interruptMask | tmp;
  interruptMask = changedPins & interruptMask;
  interruptMask = PCMSK2 & interruptMask;

  portSnapshotK = current;
#ifdef NEEDFORSPEED
#include "utility/ei_portk_speed.h"
#else
  if (interruptMask == 0) goto exitPORTKISR; // get out quickly if not interested.
#ifdef EI_ARDUINO_INTERRUPTED_PIN
  if (interruptMask & _BV(0)) { arduinoInterruptedPin=ARDUINO_PIN_K0; portKFunctions.pinZero(); }
  if (interruptMask & _BV(1)) { arduinoInterruptedPin=ARDUINO_PIN_K1; portKFunctions.pinOne(); }
  if (interruptMask & _BV(2)) { arduinoInterruptedPin=ARDUINO_PIN_K2; portKFunctions.pinTwo(); }
  if (interruptMask & _BV(3)) { arduinoInterruptedPin=ARDUINO_PIN_K3; portKFunctions.pinThree(); }
  if (interruptMask & _BV(4)) { arduinoInterruptedPin=ARDUINO_PIN_K4; portKFunctions.pinFour(); }
  if (interruptMask & _BV(5)) { arduinoInterruptedPin=ARDUINO_PIN_K5; portKFunctions.pinFive(); }
  if (interruptMask & _BV(6)) { arduinoInterruptedPin=ARDUINO_PIN_K6; portKFunctions.pinSix(); }
  if (interruptMask & _BV(7)) { arduinoInterruptedPin=ARDUINO_PIN_K7; portKFunctions.pinSeven(); }
#else
  if (interruptMask & _BV(0)) portKFunctions.pinZero();
  if (interruptMask & _BV(1)) portKFunctions.pinOne();
  if (interruptMask & _BV(2)) portKFunctions.pinTwo();
  if (interruptMask & _BV(3)) portKFunctions.pinThree();
  if (interruptMask & _BV(4)) portKFunctions.pinFour();
  if (interruptMask & _BV(5)) portKFunctions.pinFive();
  if (interruptMask & _BV(6)) portKFunctions.pinSix();
  if (interruptMask & _BV(7)) portKFunctions.pinSeven();
#endif // EI_ARDUINO_INTERRUPTED_PIN
  exitPORTKISR: return;
#endif // NEEDFORSPEED
}
#endif // EI_NOTPORTK
#elif defined ARDUINO_LEONARDO
  // No other Pin Change Interrupt ports than B on Leonardo
#endif // defined ARDUINO_328

#endif // #ifndef LIBCALL_ENABLEINTERRUPT *********************************************************
#endif // #if defined __SAM3U4E__ || defined __SAM3X8E__ || defined __SAM3X8H__
#endif // #ifndef EnableInterrupt_h ***************************************************************
