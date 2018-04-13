/* $Id: timers.c,v 1.2 2010/08/22 07:23:57 simimeie Exp $
 * Functions for timing.
 * This mainly allows you to get a timestamp value - a very exact one
 * (resolution 1 clock cycle) or a less exact one.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timers.h"

/* Internal ticks */
static volatile uint16_t ticks = 0;

/* Executes every 256 * 64 32-kHz-crystal cycles when the timer overflows,
 * so exactly 2 times per second. */
ISR(TIMER0_OVF_vect)
{
  ticks++;
}

uint16_t getticks(void)
{
  cli();
  uint16_t res = ticks;
  sei();
  return res;
}

uint16_t getticks_noirq(void)
{
  return ticks;
}

void timers_init(void)
{
  /* Enable counter0 prescaling
   * (1 increment every 1024 timer oscillator cycles) */
  TCCR0B |= _BV(CS00) | _BV(CS02);
  /* Enable interrupt when timer0 overflows (at 256) */
  TIMSK0 |= _BV(TOIE0);
}

