#if defined MIGHTY1284
#if defined INTERRUPT_FLAG_PIN8
  if (interruptMask & _BV(0)) INTERRUPT_FLAG_PIN8++;
#endif
#if defined INTERRUPT_FLAG_PIN9
  if (interruptMask & _BV(1)) INTERRUPT_FLAG_PIN9++;
#endif
#if defined INTERRUPT_FLAG_PIN10
  if (interruptMask & _BV(2)) INTERRUPT_FLAG_PIN10++;
#endif
#if defined INTERRUPT_FLAG_PIN11
  if (interruptMask & _BV(3)) INTERRUPT_FLAG_PIN11++;
#endif
#if defined INTERRUPT_FLAG_PIN12
  if (interruptMask & _BV(4)) INTERRUPT_FLAG_PIN12++;
#endif
#if defined INTERRUPT_FLAG_PIN13
  if (interruptMask & _BV(5)) INTERRUPT_FLAG_PIN13++;
#endif
#if defined INTERRUPT_FLAG_PIN14
  if (interruptMask & _BV(6)) INTERRUPT_FLAG_PIN14++;
#endif
#if defined INTERRUPT_FLAG_PIN15
  if (interruptMask & _BV(7)) INTERRUPT_FLAG_PIN15++;
#endif
#endif

#if defined ARDUINO_328
#if defined INTERRUPT_FLAG_PIN0
  if (interruptMask & _BV(0)) INTERRUPT_FLAG_PIN0++;
#endif
#if defined INTERRUPT_FLAG_PIN1
  if (interruptMask & _BV(1)) INTERRUPT_FLAG_PIN1++;
#endif
#if defined INTERRUPT_FLAG_PIN2
  if (interruptMask & _BV(2)) INTERRUPT_FLAG_PIN2++;
#endif
#if defined INTERRUPT_FLAG_PIN3
  if (interruptMask & _BV(3)) INTERRUPT_FLAG_PIN3++;
#endif
#if defined INTERRUPT_FLAG_PIN4
  if (interruptMask & _BV(4)) INTERRUPT_FLAG_PIN4++;
#endif
#if defined INTERRUPT_FLAG_PIN5
  if (interruptMask & _BV(5)) INTERRUPT_FLAG_PIN5++;
#endif
#if defined INTERRUPT_FLAG_PIN6
  if (interruptMask & _BV(6)) INTERRUPT_FLAG_PIN6++;
#endif
#if defined INTERRUPT_FLAG_PIN7
  if (interruptMask & _BV(7)) INTERRUPT_FLAG_PIN7++;
#endif
#endif
