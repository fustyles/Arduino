#if ! defined(EI_NOTINT0) && ! defined(EI_NOTINT1)
#ifdef EI_SECTION_ENABLEEXTERNAL
#ifndef EI_NOTINT0
if (arduinoPin == 2) {
#ifndef NEEDFORSPEED
  functionPointerArrayEXTERNAL[0] = userFunction;
#endif
  EIMSK &= ~_BV(0);
  EICRA &= (~_BV(0) & ~_BV(1));
  EICRA |= mode;
  EIFR  |= _BV(0); // using a clue from the ATmega2560 datasheet.
  EIMSK |= _BV(0);
}
#endif
#ifndef EI_NOTINT1
if (arduinoPin == 3) {
#ifndef NEEDFORSPEED
  functionPointerArrayEXTERNAL[1] = userFunction;
#endif
  EIMSK &= ~_BV(1);
  EICRA &= (~_BV(2) & ~_BV(3));
  EICRA |= mode << 2;
  EIFR  |= _BV(1); // using a clue from the ATmega2560 datasheet.
  EIMSK |= _BV(1);
}
#endif
#endif // EI_SECTION_ENABLEEXTERNAL

#ifdef EI_SECTION_DISABLEEXTERNAL
#ifndef EI_NOTINT0
if (arduinoPin == 2) {
  EIMSK &= ~_BV(0);
  EICRA &= (~_BV(0) & ~_BV(1));
  EIFR  |= _BV(0); // using a clue from the ATmega2560 datasheet.
}
#endif
#ifndef EI_NOTINT1
if (arduinoPin == 3) {
  EIMSK &= ~_BV(1);
  EICRA &= (~_BV(2) & ~_BV(3));
  EIFR  |= _BV(1); // using a clue from the ATmega2560 datasheet.
} 
#endif
#endif // EI_SECTION_DISABLEEXTERNAL
#endif // ! defined(EI_NOTINT0) && ! defined(EI_NOTINT1)
