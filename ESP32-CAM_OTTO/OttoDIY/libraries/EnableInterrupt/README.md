# EnableInterrupt
New Arduino interrupt library, designed for all versions of the Arduino.
NEW: 644/1284 support, using the Mighty1284 as the basis. See https://github.com/maniacbug/mighty-1284p.
Functions:

```
enableInterrupt- Enables interrupt on a selected Arduino pin.
disableInterrupt - Disables interrupt on the selected Arduino pin.
```
*_What's New?_ Tue Jun 23 06:47:10 CDT 2015 Version 0.8.1 of the library has
been released. This release includes a compiler directive that allows you
to query a variable and know which pin triggered the interrupt
(sorry, this capability is not available on the Due). See https://github.com/GreyGnome/EnableInterrupt/wiki/Usage#Determine_the_Pin_That_Was_Interrupted

(update: I forgot to include some fixes for an issue with the 1284 chip. This has now been included.)

The EnableInterrupt library is a new Arduino interrupt library, designed for
all versions of the Arduino- at this writing, the Uno (and other ATmega328p-based
boards, like the mini), Due, Leonardo (and other ATmega32u4-based boards, like the
Micro), the Mega2560 (and other ATmega2560-based boards, like the MegaADK),
and for non-Arduino 644/1284p-based projects (using the Mighty1284 for support files
and pin numbering).  It enables you to assign an interrupt to pins on your chip
that support them, and presents a common interface to all supported chips. This
means that on the Arduino Uno and Mega you don't give it an interrupt number, as per
http://arduino.cc/en/Reference/attachInterrupt. Rather, your first argument is a
pin number of a pin that's supported on that chip (see
https://github.com/GreyGnome/EnableInterrupt/wiki/Usage#pin--port-bestiary ).

## Download
See the https://github.com/GreyGnome/EnableInterrupt/wiki/Download page to
download the library.

## More Information
See the Wiki at https://github.com/GreyGnome/EnableInterrupt/wiki/Home .
For detailed usage information see https://github.com/GreyGnome/EnableInterrupt/wiki/Usage .

See the examples subdirectory in the distribution or in this Git site for code examples.

See the extras subdirectory in the distribution or in this Git site for License and Release Notes.

For a tutorial on interrupts, see
http://www.engblaze.com/we-interrupt-this-program-to-bring-you-a-tutorial-on-arduino-interrupts/
The posting gets into low-level details on interrupts.

**NOTICE** Many of the interrupt pins on the ATmega processor used in the Uno,
Leonardo, ATmega2560, and ATmega644/1284p are "Pin Change Interrupt pins". This means
that under the sheets, the pins *only* trigger on CHANGE, and a number of pins share a
single interrupt subroutine. The library enables these interrupt types to appear
normal, so that each pin can support RISING, FALLING, or CHANGE, and each pin
can support its own user-defined interrupt subroutine. But there is a *significant*
time between when the interrupt triggers and when the pins are read to determine
what actually happened (rising or falling) and which pin changed. Therefore, these
pins are *not* suitable for fast changing signals, and under the right conditions
such events as a bouncing switch may actually be missed. Caveat Programmer.
If you're concerned about this, continue to read the following information and
make sure to read the wiki pages; especially see https://github.com/GreyGnome/EnableInterrupt/wiki/Usage#atmega-processor-interrupt-types .

## ATmega Processor Interrupt Types
Note that the ATmega processor at the heart of the Arduino Uno/Mega2560/Leonardo/ATmega1284
has two different kinds of interrupts: “external”, and “pin change”.
For the list of available interrupt pins and their interrupt types, see the
PORT / PIN BESTIARY, below.

### External Interrupts
There are a varying number of external interrupt pins on the different
processors. The Uno supports only 2 and they are mapped to Arduino pins 2 and 3.
The 2560 supports 6 usable, the Leonardo supports 5, and the ATmega1284p supports 3.
These interrupts can be set to trigger on RISING, or FALLING, or both ("CHANGE")
signal levels, or on LOW level. The triggers are interpreted by hardware, so by
the time your user function is running, you know exactly which pin interrupted at
the time of the event, and how it changed. On the other hand, as mentioned there
are a limited number of these pins.

### Pin Change Interrupts
On the Arduino Uno (and again, all 328p-based boards) and 644/1284-based boards,
the pin change interrupts can be enabled on any or all of the pins. The two
pins 2 and 3 on 328p-based boards, or three pins (2, 10, and 11) on the
1284-based boards support *either* pin change or external interrupts. On 2560-based
Arduinos, there are 18 pin change interrupt pins in addition to the 6 external
interrupt pins. On the Leonardo there are 7 pin change interrupt pins in addition
to the 5 external interrupt pins. See PIN BESTIARY below for the pin numbers and
other details.

Pin Change interrupts trigger on all RISING and FALLING signal edges.
Furthermore, the processor's pins, and pin change interrupts, are grouped into
“port”s, so for example on the Arduino Uno there are three ports and therefore
only 3 interrupt vectors (subroutines) available for the entire body of 20 pin
change interrupt pins.

### The Library and Pin Change Interrupts
When an event triggers an interrupt on any interrupt-enabled pin on a port, a
subroutine attached to that pin's port is triggered. It is up to the interrupt
library to set the proper port to receive interrupts for a pin, to determine
what happened when an interrupt is triggered (which pin? ...did the signal rise,
or fall?), to handle it properly (Did we care if the signal fell? Did we care
if it rose?), then to call the programmer's chosen subroutine. This makes the
job of resolving the action on a single pin somewhat complicated. There is a
modest slowdown in the interrupt routine because of this complication. Perhaps
more importantly, there is some latency between the interrupt and the system
determining exactly which pin and what change caused it. So the signal could
have changed by the time the pin's status is read, returning a false reading
back to your sketch. For a review of this issue see
https://github.com/GreyGnome/EnableInterrupt/blob/master/Interrupt%20Timing.pdf

# USAGE:
## Basic Usage
*enableInterrupt*- Enables interrupt on a selected Arduino pin.
```C
enableInterrupt(uint8_t pinNumber, void (*userFunction)(void), uint8_t mode);
or
enableInterrupt(uint8_t interruptDesignator, void (*userFunction)(void), uint8_t mode);

The arguments are:
* pinNumber - The number of the Arduino pin, such as 3, or A0, or SCK. Note that
these are *not* strings, so when you use A0 for example, do not use quotes.
* interruptDesignator- very much like a pin. See below.
* userFunction - The name of the function you want the interrupt to run. Do not
use a pointer here, just give it the name of your function. See the example code
in the Examples directory.
* mode - What you want the interrupt to interrupt on. For Pin Change Interrupt
pins, the modes supported are RISING, FALLING, or CHANGE.
** RISING - The signal went from "0", or zero volts, to "1", or 5 volts.
** FALLING - The signal went from "1" to "0".
** CHANGE - The signal either rose or fell.

For External Interrupts, the same modes are supported plus the additional mode
of LOW signal level.
** LOW - The signal is at a low level for some time.

Each pin supports only 1 function and 1 mode at a time.
```

*disableInterrupt*- Disables interrupt on a selected Arduino pin.  

```C
disableInterrupt(uint8_t pinNumber);
or
disableInterrupt(uint8_t interruptDesignator);
```

* interruptDesignator: Essentially this is an Arduino pin, and if that's all you want to give
the function, it will work just fine. Why is it called an "interruptDesignator", then? Because
there's a twist: You can perform a bitwise "and" with the pin number and PINCHANGEINTERRUPT
to specify that you want to use a Pin Change Interrupt type of interrupt on those pins that
support both Pin Change and External Interrupts. Otherwise, the library will choose whatever
interrupt type (External, or Pin Change) normally applies to that pin,
with priority to External Interrupt. 

* The complexity is because of pins 2 and 3 on the ATmega328-based Arduinos, and pins 2, 10,
and 11 on 1284-based boards. Those are the only pins on the processors supported by this
library that can share External or Pin Change Interrupt types. Otherwise, each pin only supports
a single type of interrupt and the PINCHANGEINTERRUPT scheme changes nothing. This means you can
ignore this whole discussion for ATmega2560, ATmega32U4, or SAM3X8E (Due)-based Arduinos.

It is possible to change the user function assigned to an interrupt after enabling it (if you
want). Later in your code simply disable the interrupt and enable it with a different function.

## Determine the Pin That Was Interrupted
There is a facility in the library to identify the most recent pin that triggered an interrupt. Set the following definition '''before''' including the EnableInterrupt.h file in your sketch:
```
 #define EI_ARDUINO_INTERRUPTED_PIN
```
Then, the ATmega chip will set a variable with every interrupt, and you can query it to find which pin interrupted your sketch. The variable is arduinoInterruptedPin and it is of type uint8_t.

See the https://github.com/GreyGnome/EnableInterrupt/wiki/Usage wiki page for more information.

# PIN / PORT BESTIARY
Theoretically pins 0 and 1 (RX and TX) are supported but as these pins have
a special purpose on the Arduino, their use in this library has not been tested.

## Summary
### Arduino Uno/Duemilanove/etc.
Interrupt Type | Pins
-------------- | --------------
External       | 2 3
Pin Change     | 2-13 and A0-A5
### Arduino Mega2560
Interrupt Type | Pins
-------------- | --------------
External       | 2 3 and 18-21
Pin Change     | 10-15 and A8-A15 and SS, SCK, MOSI, MISO
### Arduino Leonardo
Interrupt Type | Pins
-------------- | --------------
External       | 0-3 and 7
Pin Change     | 8-11 and SCK, MOSI, MISO
### Mighty 1284
Interrupt Type | Pins
-------------- | --------------
External       | 2 10 11
Pin Change     | 0-31 (aka: 0-23 and A0-A7)

## Details
### Arduino Uno

<pre>
Interrupt Pins:
Arduino	External                Arduino Pin Change      Arduino Pin Change
Pin     Interrupt               Pin     Interrupt       Pin     Interrupt
                Port                           Port                     Port
2       INT0    PD2             2       PCINT18 PD2     A0      PCINT8  PC0
3       INT1    PD3             3       PCINT19 PD3     A1      PCINT9  PC1
                                4       PCINT20 PD4     A2      PCINT10 PC2
                                5       PCINT21 PD5     A3      PCINT11 PC3
                                6       PCINT22 PD6     A4      PCINT12 PC4
                                7       PCINT23 PD7     A5      PCINT13 PC5
                                8       PCINT0  PB0
                                9       PCINT1  PB1
                                10      PCINT2  PB2
                                11      PCINT3  PB3
                                12      PCINT4  PB4
                                13      PCINT5  PB5
</pre>

### Leonardo Pins LEONARDO

<pre>
Interrupt pins:
Arduino                         Arduino
Pin     External                Pin     Pin Change
        Interrupt                       Interrupt
               Port                               Port
 3      INT0   PD0              8       PCINT4    PB4
 2      INT1   PD1              9       PCINT5    PB5
 0      INT2   PD2              10      PCINT6    PB6
 1      INT3   PD3              11      PCINT7    PB7
 7      INT6   PE6              SCK/15  PCINT1    PB1
                                MOSI/16 PCINT2    PB2
                                MISO/14 PCINT3    PB3

                                on ICSP:
                                SCK/15:  PCINT1 (PB1)
                                MOSI/16: PCINT2 (PB2)
                                MISO/14: PCINT3 (PB3)

// Map SPI port to 'new' pins D14..D17
static const uint8_t SS   = 17;
static const uint8_t MOSI = 16;
static const uint8_t MISO = 14;
static const uint8_t SCK  = 15;
// A0 starts at 18

</pre>

### ATmega2560 Support

<pre>
External Interrupts ------------------------------------------------------------
The following External Interrupts are available on the Arduino:
Arduino           
  Pin  PORT INT  ATmega2560 pin
  21     PD0  0     43
  20     PD1  1     44
  19     PD2  2     45
  18     PD3  3     46
   2     PE4  4      6
   3     PE5  5      7
 n/c     PE6  6      8  (fake pin 75) **
 n/c     PE7  7      9  (fake pin 76)


Pin Change Interrupts ----------------------------------------------------------

ATMEGA2560 Pin Change Interrupts
Arduino              Arduino              Arduino
  Pin  PORT PCINT     Pin   PORT PCINT     Pin   PORT PCINT
  A8     PK0  16       10     PB4   4       SS     PB0   0
  A9     PK1  17       11     PB5   5       SCK    PB1   1
 A10     PK2  18       12     PB6   6       MOSI   PB2   2
 A11     PK3  19       13     PB7   7       MISO   PB3   3
 A12     PK4  20       14     PJ1  10
 A13     PK5  21       15     PJ0   9
 A14     PK6  22        0     PE0   8 - this one is a little odd. *
 A15     PK7  23
</pre>

The library supports all interrupt pins, even though not all pins to the
ATmega-2560 processor are exposed on the Arduino board. These pins are
supported as "fake pins", and begin with pin 70 (there are 70 pins on the
ATmega 2560 board). The fake pins are as follows:

<pre>
pin: fake70 PJ2 this is Pin Change Interrupt PCINT11
pin: fake71 PJ3 this is Pin Change Interrupt PCINT12
pin: fake72 PJ4 this is Pin Change Interrupt PCINT13
pin: fake73 PJ5 this is Pin Change Interrupt PCINT14
pin: fake74 PJ6 this is Pin Change Interrupt PCINT15
pin: fake75 PE6 this is External Interrupt INT6
pin: fake76 PE7 this is External Interrupt INT7
</pre>

* Note: Arduino Pin 0 is PE0 (PCINT8), which is RX0. Also, it is the only other
pin on another port on PCI1. This would make it very costly to integrate with
the library's code and thus is not supported by this library.  It is the same
pin the Arduino uses to upload sketches, and it is connected to the FT232RL
USB-to-Serial chip (ATmega16U2 on the R3).

### Mighty 1284 Support
The ATmega 1284p shares pinout with the 644; the only difference is in memory
size. We use the "Mighty 1284" platform as our model, because the needed files are
mature and complete.

<pre>
Interrupt Pins:
Mighty  External                Mighty                                 Mighty           
Pin     Interrupt               Pin*  PORT PCINT ATmega644/1284 pin    Pin*  PORT PCINT ATmega644/1284 pin
                Port            0     PB0   8         1                15    PD7  31        21
2       INT2    PB2             1     PB1   9         2                16    PC0  16        22
10      INT1    PD2             2     PB2   2         3                17    PC1  17        23
11      INT0    PD3             3     PB3  11         4                18    PC2  18        24
                                4     PB4  12         5                19    PC3  19        25
                                5     PB5  13         6                20    PC4  20        26
                                6     PB6  14         7                21    PC5  21        27
                                7     PB7  15         8                22    PC6  22        28
                                8     PD0  24        14                23    PC7  23        29
                                9     PD1  25        15             31/A7    PA7   7        33
                               10     PD2  26        16             30/A6    PA6   6        34
                               11     PD3  27        17             29/A5    PA5   5        35
                               12     PD4  28        18             28/A4    PA4   4        36
                               13     PD5  29        19             27/A3    PA3   3        37
                               14     PD6  30        20             26/A2    PA2   2        38
                                                                    25/A1    PA1   1        39
                                                                    24/A0    PA0   0        40
</pre>
