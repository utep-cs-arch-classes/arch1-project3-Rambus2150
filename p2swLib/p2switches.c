#include <msp430.h>
#include "p2switches.h"

static unsigned char switch_mask;
static unsigned char switches_last_reported;
static unsigned char switches_current;

static char
switch_update_interrupt_sense()
{
   char p1val = P2IN;

  P2IES |= (p1val & SWITCHES);
  P2IES &= (p1val | ~SWITCHES);
  return p1val;
}

void 
switch_init()
{
  
  P2REN |= SWITCHES;    /* enables resistors for switches */
  P2IE = SWITCHES;      /* enable interrupts from switches */
  P2OUT |= SWITCHES;    /* pull-ups for switches */
  P2DIR &= ~SWITCHES;   /* set switches' bits for input */
  P2IES |= SWITCHES;    

  switch_update_interrupt_sense();
}

/* Returns a word where:
 * the high-order byte is the buttons that have changed,
 * the low-order byte is the current state of the buttons
 */
/*unsigned int 
p2sw_read() {
  unsigned int sw_changed = switches_current ^ switches_last_reported;
  switches_last_reported = switches_current;
  return switches_current | (sw_changed << 8);
}

/* Switch on P2 (S1) 
__interrupt(PORT2_VECTOR) Port_2(){
  if (P2IFG & switch_mask) {  /* did a button cause this interrupt? 
    P2IFG &= ~switch_mask;	/* clear pending sw interrupts 
    switch_update_interrupt_sense();
  }*/
}
