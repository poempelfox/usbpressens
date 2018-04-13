/* $Id: timers.h $
 * Functions for timing.
 * This mainly allows you to get a timestamp value.
 */

#ifndef _TIMERS_H_
#define _TIMERS_H_

/* timestamps are delivered in 'ticks'.
 * Ticks are overflows of the TIMER0 counter, which is prescaled by 1024.
 * At 8 MHz, there are about 30.5 of these per second.
 * Since Ticks is a 16 bit value, it will overflow every around 2147
 * seconds (35 minutes).
 */

#define TICKSPERSECOND (8000000 / (256UL * 1024UL))
#define TICKSPERSECONDF (8000000.0 / (256.0 * 1024.0))

/* Init timers */
void timers_init(void);

/* get ticks.
 * Warning: Will reenable interrupts if they were disabled. */
uint16_t getticks(void);
/* The same, but won't touch the global interrupt enable bit.
 * MUST BE CALLED WITH INTERRUPTS DISABLED */
uint16_t getticks_noirq(void);

#endif /* _TIMERS_H_ */
