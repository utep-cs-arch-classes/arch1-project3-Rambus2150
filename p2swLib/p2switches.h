#ifndef switches_included
#define switches_included

#include "msp430.h"
#define SW1 BIT0
#define SW2 BIT1
#define SW3 BIT2
#define SW4 BIT3

#define SWITCHES (SW1 | SW2 | SW3 | SW4)
void switch_init();
char switch_update_interrupt_sense();



#endif // included
