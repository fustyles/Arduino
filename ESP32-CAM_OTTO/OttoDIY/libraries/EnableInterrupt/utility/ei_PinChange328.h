#ifdef EI_SECTION_RISING
#ifndef EI_NOTPORTB
if (portNumber==PB) {
  risingPinsPORTB |= portMask;
}
#endif
#ifndef EI_NOTPORTC
if (portNumber==PC) {
  risingPinsPORTC |= portMask;
}
#endif
#ifndef EI_NOTPORTD
if (portNumber==PD) {
  risingPinsPORTD |= portMask;
}
#endif
#endif // EI_SECTION_RISING

#ifdef EI_SECTION_FALLING
#ifndef EI_NOTPORTB
if (portNumber==PB) {
  fallingPinsPORTB |= portMask;
}
#endif
#ifndef EI_NOTPORTC
if (portNumber==PC) {
  fallingPinsPORTC |= portMask;
}
#endif
#ifndef EI_NOTPORTD
if (portNumber==PD) {
  fallingPinsPORTD |= portMask;
}
#endif
#endif // EI_SECTION_FALLING

#if defined EI_SECTION_ASSIGNFUNCTIONSREGISTERS
#ifndef EI_NOTPORTB
if (portNumber==PB) {
#ifndef NEEDFORSPEED
  calculatedPointer=&portBFunctions.pinZero + portBitNumber;
  *calculatedPointer = userFunction;
#endif
  portSnapshotB=*portInputRegister(portNumber);
  pcmsk=&PCMSK0;
  PCICR |= _BV(0);
}
#endif
#ifndef EI_NOTPORTC
if (portNumber==PC) {
#ifndef NEEDFORSPEED
  calculatedPointer=&portCFunctions.pinZero + portBitNumber;
  *calculatedPointer = userFunction;
#endif
  portSnapshotC=*portInputRegister(portNumber);
  pcmsk=&PCMSK1;
  PCICR |= _BV(1);
}
#endif
#ifndef EI_NOTPORTD
if (portNumber==PD) {
#ifndef NEEDFORSPEED
  calculatedPointer=&portDFunctions.pinZero + portBitNumber;
  *calculatedPointer = userFunction;
#endif
  portSnapshotD=*portInputRegister(portNumber);
  pcmsk=&PCMSK2;
  PCICR |= _BV(2);
}
#endif
#endif // EI_SECTION_ASSIGNFUNCTIONSREGISTERS

#ifdef EI_SECTION_DISABLEPINCHANGE
#ifndef EI_NOTPORTB
if (portNumber == PB) {
  PCMSK0 &= ~portMask;
  if (PCMSK0 == 0) { PCICR &= ~_BV(0); };
  risingPinsPORTB &= ~portMask;
  fallingPinsPORTB &= ~portMask;
}
#endif
#ifndef EI_NOTPORTC
if (portNumber == PC) {
  PCMSK1 &= ~portMask;
  if (PCMSK1 == 0) { PCICR &= ~_BV(1); };
  risingPinsPORTC &= ~portMask;
  fallingPinsPORTC &= ~portMask;
}
#endif
#ifndef EI_NOTPORTD
if (portNumber == PD) {
  PCMSK2 &= ~portMask;
  if (PCMSK2 == 0) { PCICR &= ~_BV(2); };
  risingPinsPORTD &= ~portMask;
  fallingPinsPORTD &= ~portMask;
}
#endif
#endif // EI_SECTION_DISABLEPINCHANGE
