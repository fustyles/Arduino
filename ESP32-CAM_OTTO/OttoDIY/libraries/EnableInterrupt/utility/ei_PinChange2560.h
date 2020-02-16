#ifdef EI_SECTION_RISING
#ifndef EI_NOTPORTB
if (portNumber==PB) {
  risingPinsPORTB |= portMask;
}
#endif
#ifndef EI_NOTPORTJ
if (portNumber==PJ) {
  risingPinsPORTJ |= portMask;
}
#endif
#ifndef EI_NOTPORTK
if (portNumber==PK) {
  risingPinsPORTK |= portMask;
}
#endif
#endif

#ifdef EI_SECTION_FALLING
#ifndef EI_NOTPORTB
if (portNumber==PB) {
  fallingPinsPORTB |= portMask;
}
#endif
#ifndef EI_NOTPORTJ
if (portNumber==PJ) {
  fallingPinsPORTJ |= portMask;
}
#endif
#ifndef EI_NOTPORTK
if (portNumber==PK) {
  fallingPinsPORTK |= portMask;
}
#endif
#endif // EI_SECTION_FALLING

#ifdef EI_SECTION_ASSIGNFUNCTIONSREGISTERS
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
#ifndef EI_NOTPORTJ
if (portNumber==PJ) {
#ifndef NEEDFORSPEED
  calculatedPointer=&portJFunctions.pinZero + portBitNumber;
  *calculatedPointer = userFunction;
#endif
  portSnapshotJ=*portInputRegister(portNumber);
  pcmsk=&PCMSK1;
  PCICR |= _BV(1);
  // TODO: I think the order of these is flipped. Test. BUG??? -Mike
  portJPCMSK|=portMask; // because PCMSK1 is shifted wrt. PortJ.
  portMask <<= 1; // Handle port J's oddness. PJ0 is actually 1 on PCMSK1.
}
#endif
#ifndef EI_NOTPORTK
if (portNumber==PK) {
#ifndef NEEDFORSPEED
  calculatedPointer=&portKFunctions.pinZero + portBitNumber;
  *calculatedPointer = userFunction;
#endif
  portSnapshotK=*portInputRegister(portNumber);
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
#ifndef EI_NOTPORTJ
if (portNumber == PJ) {
  // Handle port J's oddness. PJ0 is actually 1 on PCMSK1.
  PCMSK1 &= ((~portMask << 1) | 0x01); // or with 1 to not touch PE0.
  if (PCMSK1 == 0) { PCICR &= ~_BV(1); };
  risingPinsPORTJ &= ~portMask;
  fallingPinsPORTJ &= ~portMask;
}
#endif
#ifndef EI_NOTPORTK
if (portNumber == PK) {
  PCMSK2 &= ~portMask;
  if (PCMSK2 == 0) { PCICR &= ~_BV(2); };
  risingPinsPORTK &= ~portMask;
  fallingPinsPORTK &= ~portMask;
}
#endif
#endif // EI_SECTION_DISABLEPINCHANGE

