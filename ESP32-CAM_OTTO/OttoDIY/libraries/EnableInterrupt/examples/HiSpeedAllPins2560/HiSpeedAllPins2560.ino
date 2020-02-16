// EnableInterrupt example sketch: Hi Speed Interrupts, ATmega2560/compatible, all pins
// See the Wiki at https://github.com/GreyGnome/EnableInterrupt/wiki

#define NEEDFORSPEED

#define INTERRUPT_FLAG_PINSS   iflag_pinSS
#define INTERRUPT_FLAG_PINSCK  iflag_pinSCK
#define INTERRUPT_FLAG_PINMOSI iflag_pinMOSI
#define INTERRUPT_FLAG_PINMISO iflag_pinMISO
#define INTERRUPT_FLAG_PIN10   iflag_pin10
#define INTERRUPT_FLAG_PIN11   iflag_pin11
#define INTERRUPT_FLAG_PIN12   iflag_pin12
#define INTERRUPT_FLAG_PIN13   iflag_pin13
#define INTERRUPT_FLAG_PIN14   iflag_pin14
#define INTERRUPT_FLAG_PIN15   iflag_pin15
#define INTERRUPT_FLAG_PINA8   iflag_pinA8
#define INTERRUPT_FLAG_PINA9   iflag_pinA9
#define INTERRUPT_FLAG_PINA10  iflag_pinA10
#define INTERRUPT_FLAG_PINA11  iflag_pinA11
#define INTERRUPT_FLAG_PINA12  iflag_pinA12
#define INTERRUPT_FLAG_PINA13  iflag_pinA13
#define INTERRUPT_FLAG_PINA14  iflag_pinA14
#define INTERRUPT_FLAG_PINA15  iflag_pinA15
#define INTERRUPT_FLAG_PIN70   iflag_pin70
#define INTERRUPT_FLAG_PIN71   iflag_pin71
#define INTERRUPT_FLAG_PIN72   iflag_pin72
#define INTERRUPT_FLAG_PIN73   iflag_pin73
#define INTERRUPT_FLAG_PIN74   iflag_pin74
#define INTERRUPT_FLAG_PIN21   iflag_pin21
#define INTERRUPT_FLAG_PIN20   iflag_pin20
#define INTERRUPT_FLAG_PIN19   iflag_pin19
#define INTERRUPT_FLAG_PIN18   iflag_pin18
#define INTERRUPT_FLAG_PIN2    iflag_pin2
#define INTERRUPT_FLAG_PIN3    iflag_pin3
#define INTERRUPT_FLAG_PIN75   iflag_pin75
#define INTERRUPT_FLAG_PIN76   iflag_pin76

#define INTERRUPT_FLAG_PIN18   iflag_pin18
#define INTERRUPT_FLAG_PIN19   iflag_pin19
#define INTERRUPT_FLAG_PIN20   iflag_pin20
#define INTERRUPT_FLAG_PIN21   iflag_pin21

#include <EnableInterrupt.h>

volatile uint8_t externalInterruptCounter=0;
volatile uint8_t anyInterruptCounter=0;

#ifdef ARDUINO_MEGA
#define PINCOUNT(x) iflag_pin ##x

#define setupInterrupt(x) \
  EI_printPSTR("Add pin: "); \
  EI_printPSTR(#x); \
  EI_printPSTR("\r\n"); \
  pinMode( x, INPUT_PULLUP); \
  enableInterruptFast(x, CHANGE)

#define updateOn(x) \
  if (PINCOUNT(x) != 0) { \
    printIt((char *) #x, PINCOUNT(x)); \
    PINCOUNT(x)=0; \
  }

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
  enableInterruptFast(70, CHANGE);
  enableInterruptFast(71, CHANGE);
  enableInterruptFast(72, CHANGE);
  enableInterruptFast(73, CHANGE);
  enableInterruptFast(74, CHANGE);
  // External Interrupts
  setupInterrupt(21);
  setupInterrupt(20);
  setupInterrupt(19);
  setupInterrupt(18);
  setupInterrupt(2);
  setupInterrupt(3);
  ////
  DDRE |=0b11000000; // Non-Arduino Port E pins all become output.
  PORTE|=0b11000000; // Turn them all high.
  enableInterruptFast(75, CHANGE);
  enableInterruptFast(76, CHANGE);
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
      setupInterrupt(20);
      enableInterruptFast(71, CHANGE);
      enableInterruptFast(75, CHANGE);
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
