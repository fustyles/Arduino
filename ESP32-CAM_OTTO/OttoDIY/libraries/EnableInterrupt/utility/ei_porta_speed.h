#if defined MIGHTY1284
#ifdef INTERRUPT_FLAG_PIN24
  if (interruptMask & _BV(0)) INTERRUPT_FLAG_PIN24++;
#endif
#ifdef INTERRUPT_FLAG_PIN25
  if (interruptMask & _BV(1)) INTERRUPT_FLAG_PIN25++;
#endif
#ifdef INTERRUPT_FLAG_PIN26
  if (interruptMask & _BV(2)) INTERRUPT_FLAG_PIN26++;
#endif
#ifdef INTERRUPT_FLAG_PIN27
  if (interruptMask & _BV(3)) INTERRUPT_FLAG_PIN27++;
#endif
#ifdef INTERRUPT_FLAG_PIN28
  if (interruptMask & _BV(4)) INTERRUPT_FLAG_PIN28++;
#endif
#ifdef INTERRUPT_FLAG_PIN29
  if (interruptMask & _BV(5)) INTERRUPT_FLAG_PIN29++;
#endif
#ifdef INTERRUPT_FLAG_PIN30
  if (interruptMask & _BV(6)) INTERRUPT_FLAG_PIN30++;
#endif
#ifdef INTERRUPT_FLAG_PIN31
  if (interruptMask & _BV(7)) INTERRUPT_FLAG_PIN31++;
#endif
#endif
