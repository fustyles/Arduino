#ifdef EI_SECTION_RISING
#ifndef EI_NOTPORTB
if (portNumber==PB) { 
  risingPinsPORTB |= portMask;
}
#endif
#endif // EI_SECTION_RISING

#ifdef EI_SECTION_FALLING
#ifndef EI_NOTPORTB
if (portNumber==PB) { 
  fallingPinsPORTB |= portMask;
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
#endif // EI_SECTION_DISABLEPINCHANGE
