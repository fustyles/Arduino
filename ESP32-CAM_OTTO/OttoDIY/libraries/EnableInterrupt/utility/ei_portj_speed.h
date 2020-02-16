#if defined ARDUINO_MEGA
#if defined INTERRUPT_FLAG_PIN15
  if (interruptMask & _BV(0)) INTERRUPT_FLAG_PIN15++;
#endif
#if defined INTERRUPT_FLAG_PIN14
  if (interruptMask & _BV(1)) INTERRUPT_FLAG_PIN14++;
#endif
#if defined INTERRUPT_FLAG_PIN70
  if (interruptMask & _BV(2)) INTERRUPT_FLAG_PIN70++;
#endif
#if defined INTERRUPT_FLAG_PIN71
  if (interruptMask & _BV(3)) INTERRUPT_FLAG_PIN71++;
#endif
#if defined INTERRUPT_FLAG_PIN72
  if (interruptMask & _BV(4)) INTERRUPT_FLAG_PIN72++;
#endif
#if defined INTERRUPT_FLAG_PIN73
  if (interruptMask & _BV(5)) INTERRUPT_FLAG_PIN73++;
#endif
#if defined INTERRUPT_FLAG_PIN74
  if (interruptMask & _BV(6)) INTERRUPT_FLAG_PIN74++;
#endif
#endif
// NOTE: 75 and 76 are "fake" External interrupt pins.
