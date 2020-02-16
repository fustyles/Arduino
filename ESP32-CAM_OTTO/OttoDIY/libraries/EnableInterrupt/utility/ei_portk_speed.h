#if defined ARDUINO_MEGA
#if defined INTERRUPT_FLAG_PINA8
  if (interruptMask & _BV(0)) INTERRUPT_FLAG_PINA8++;
#endif
#if defined INTERRUPT_FLAG_PINA9
  if (interruptMask & _BV(1)) INTERRUPT_FLAG_PINA9++;
#endif
#if defined INTERRUPT_FLAG_PINA10
  if (interruptMask & _BV(2)) INTERRUPT_FLAG_PINA10++;
#endif
#if defined INTERRUPT_FLAG_PINA11
  if (interruptMask & _BV(3)) INTERRUPT_FLAG_PINA11++;
#endif
#if defined INTERRUPT_FLAG_PINA12
  if (interruptMask & _BV(4)) INTERRUPT_FLAG_PINA12++;
#endif
#if defined INTERRUPT_FLAG_PINA13
  if (interruptMask & _BV(5)) INTERRUPT_FLAG_PINA13++;
#endif
#if defined INTERRUPT_FLAG_PINA14
  if (interruptMask & _BV(6)) INTERRUPT_FLAG_PINA14++;
#endif
#if defined INTERRUPT_FLAG_PINA15
  if (interruptMask & _BV(7)) INTERRUPT_FLAG_PINA15++;
#endif
#endif
