/*
 * Functions for a serial console.
 */

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <avr/pgmspace.h>

/* Init needs to be called with IRQs still disabled! */
void console_init(void);

/* These need to be called with IRQs disabled! They are usually NOT what
 * you want. */
void console_printchar_noirq(uint8_t c);
void console_printtext_noirq(const uint8_t * what);
void console_printpgm_noirq_P(PGM_P what);
void console_printhex8_noirq(uint8_t what);
void console_printdec_noirq(uint8_t what);

/* These can be called with interrupts enabled (and will reenable them!) */
void console_printchar(uint8_t what);
void console_printtext(const uint8_t * what);
void console_printpgm_P(PGM_P what);
void console_printhex8(uint8_t what);
void console_printdec(uint8_t what);

#endif /* _CONSOLE_H_ */

