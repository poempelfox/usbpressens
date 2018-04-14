/* $Id: $
 * Functions for a serial console.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/version.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdio.h>
#include "console.h"

/* PD0 is RXD, PD1 is TXD, but we don't need to address them manually */

/* Formula for calculating the value of UBRR from baudrate and cpufreq */
#define UBRRCALC ((CPUFREQ / (16 * BAUDRATE)) - 1)

#define INPUTBUFSIZE 30
static uint8_t inputbuf[INPUTBUFSIZE];
static uint8_t inputpos = 0;
#define OUTPUTBUFSIZE 400
static uint8_t outputbuf[OUTPUTBUFSIZE];
static uint16_t outputhead = 0; /* WARNING cannot be modified atomically */
static uint16_t outputtail = 0;
static uint8_t opinprog = 0;
static uint8_t escstatus = 0;
static const uint8_t CRLF[] PROGMEM = "\r\n";
static const uint8_t WELCOMEMSG[] PROGMEM = "\r\n"\
				   "\r\n *****************************"\
				   "\r\n * Foxis USB pressure sensor *"\
				   "\r\n *****************************"\
				   "\r\n"\
				   "\r\nAVR-libc: " __AVR_LIBC_VERSION_STRING__ " (" __AVR_LIBC_DATE_STRING__ ")"\
				   "\r\nSoftware Version 0.1, Compiled " __DATE__ " " __TIME__;
static const uint8_t PROMPT[] PROGMEM = "\r\n# ";

/* external variables */
extern uint32_t barpress;
extern int16_t temp;

/* Handler for TXC (TX Complete) IRQ */
ISR(USART_TX_vect) {
  if (outputhead == outputtail) { /* Nothing more to send! */
    opinprog = 0;
  } else {
    UDR0 = outputbuf[outputhead];
    outputhead++;
    if (outputhead >= OUTPUTBUFSIZE) {
      outputhead = 0;
    }
  }
}

#if defined __GNUC__
/* Unfortunately, gcc is very very dumb, and despite -Os tries to inline this
 * function even though it is called about a thousand times, when only
 * NETCONSOLE is defined, costing us more than 2.5 KiloBytes of Flash.
 * Way to go, gcc "optimization". */
static void appendchar(uint8_t what) __attribute__((noinline));
#endif /* __GNUC__ */
/* This can only be called safely with interrupts disabled - remember that! */
static void appendchar(uint8_t what) {
  uint16_t newpos;
  newpos = (outputtail + 1);
  if (newpos >= OUTPUTBUFSIZE) {
    newpos = 0;
  }
  if (newpos != outputhead) {
    outputbuf[outputtail] = what;
    outputtail = newpos;
  }
  if (!opinprog) {
    /* Send the byte */
    UDR0 = what;
    outputhead++;
    if (outputhead >= OUTPUTBUFSIZE) {
      outputhead = 0;
    }
    opinprog = 1;
  }
}

void console_printchar_noirq(uint8_t what) {
  appendchar(what);
}

/* This can only be called safely with interrupts disabled - remember that! */
void console_printhex8_noirq(uint8_t what) {
  uint8_t buf;
  uint8_t i;
  for (i=0; i<2; i++) {
    buf = (uint8_t)(what & (uint8_t)0xF0) >> 4;
    if (buf <= 9) {
      buf += '0';
    } else {
      buf += 'A' - 10;
    }
    appendchar(buf);
    what <<= 4;
  }
}

/* This can only be called safely with interrupts disabled - remember that! */
void console_printdec_noirq(uint8_t what) {
  uint8_t buf;
  buf = what / 100;
  appendchar(buf + '0');
  what %= 100;
  buf = what / 10;
  appendchar(buf + '0');
  buf = what % 10;
  appendchar(buf + '0');
}

/* This can only be called safely with interrupts disabled - remember that! */
/* This is the same as printec, but only prints 2 digits (e.g. for times/dates) */
void console_printdec2_noirq(uint8_t what) {
  if (what > 99) { what = 99; }
  appendchar((what / 10) + '0');
  appendchar((what % 10) + '0');
}

/* This can only be called safely with interrupts disabled - remember that! */
void console_printbin8_noirq(uint8_t what) {
  uint8_t i;
  for (i = 0; i < 8; i++) {
    if (what & 0x80) {
      appendchar('1');
    } else {
      appendchar('0');
    }
    what <<= 1;
  }
}

/* This can only be called safely with interrupts disabled - remember that! */
void console_printtext_noirq(const uint8_t * what) {
  while (*what) {
    appendchar(*what);
    what++;
  }
}

/* This can only be called safely with interrupts disabled - remember that! */
void console_printpgm_noirq_P(PGM_P what) {
  uint8_t t;
  while ((t = pgm_read_byte(what++))) {
    appendchar(t);
  }
}

/* These are wrappers for our internal functions, disabling IRQs before
 * calling them. */
void console_printchar(uint8_t what) {
  cli();
  console_printchar_noirq(what);
  sei();
}

void console_printtext(const uint8_t * what) {
  cli();
  console_printtext_noirq(what);
  sei();
}

void console_printpgm_P(PGM_P what) {
  cli();
  console_printpgm_noirq_P(what);
  sei();
}

void console_printhex8(uint8_t what) {
  cli();
  console_printhex8_noirq(what);
  sei();
}

void console_printdec(uint8_t what) {
  cli();
  console_printdec_noirq(what);
  sei();
}

void console_init(void) {
  /* Set Baud Rate */
  UBRR0H = (uint8_t)(UBRRCALC >> 8);
  UBRR0L = (uint8_t)(UBRRCALC);
  /* Set 8 Bit mode - how braindead is it to default to 5 bit?! */
  UCSR0C = /* _BV(URSEL) | */ _BV(UCSZ00) | _BV(UCSZ01);
  /* Enable Send and Receive and IRQs */
  UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(TXCIE0) | _BV(RXCIE0);
  console_printpgm_noirq_P(WELCOMEMSG);
  console_printpgm_noirq_P(PROMPT);
  return;
}

/* We do all query processing here.
 * This must be called with IRQs disabled.
 * It currently only gets called from the serial RX complete IRQ (i.e. if
 * a byte has been received from the serial console)
 */
void console_inputchar(uint8_t inpb) {
  if (escstatus == 1) {
    if (inpb == '[') {
      escstatus = 2;
    } else {
      escstatus = 0;
      appendchar(7); /* Bell */
    }
    return;
  }
  if (escstatus == 2) {
    switch (inpb) {
    case 'A': /* Up */
            /* Try to restore the last comnmand */
            for (; inputpos > 0; inputpos--) {
              appendchar(8);
            }
            while (inputbuf[inputpos]) {
              appendchar(inputbuf[inputpos++]);
            }
            break;
    case 'B': /* Down */
            /* New empty command line */
            for (; inputpos > 0; inputpos--) {
              appendchar(8);
              appendchar(' ');
              appendchar(8);
            }
            break;
    case 'C': /* Left */
    case 'D': /* Right */
    default:
            appendchar(7); /* Bell */
            break;
    };
    escstatus = 0;
    return;
  }
  /* escstatus == 0, i.e. not in an escape */
  switch (inpb) {
  case 0 ... 7: /* Nonprinting characters. Ignore. */
  case 11 ... 12: /* Nonprinting characters. Ignore. */
  case 14 ... 26: /* Nonprinting characters. Ignore. */
  case 28 ... 31: /* Nonprinting characters. Ignore. */
  case 0x7f ... 0xff: /* Nonprinting characters. Ignore. */
          console_printhex8_noirq(inpb);
          appendchar(7); /* Bell */
          break;
  case 9: /* tab. Should be implemented some day? */
          appendchar(7); /* Bell */
          break;
  case 27: /* Escape */
          escstatus = 1;
          break;
  case 8: /* Backspace */
          if (inputpos > 0) {
            inputpos--;
            appendchar(inpb);
            appendchar(' ');
            appendchar(inpb);
          }
          break;
  case '\r': /* 13 */
  case '\n': /* 10 */
          if (inputpos == 0) {
            console_printpgm_noirq_P(PROMPT);
            break;
          }
          inputbuf[inputpos] = 0; /* Null-terminate the string */
          console_printpgm_noirq_P(CRLF);
          /* now lets see what it is */
          if        (strcmp_P(inputbuf, PSTR("help")) == 0) {
            console_printpgm_noirq_P(PSTR("Available commands:"));
            console_printpgm_noirq_P(PSTR("\r\n motd             repeat welcome message"));
            console_printpgm_noirq_P(PSTR("\r\n showpins [x]     shows the avrs inputpins"));
            console_printpgm_noirq_P(PSTR("\r\n status           show status / counters"));
          } else if (strcmp_P(inputbuf, PSTR("motd")) == 0) {
            console_printpgm_noirq_P(WELCOMEMSG);
          } else if (strncmp_P(inputbuf, PSTR("showpins"), 8) == 0) {
            uint8_t which = 0;
            uint8_t stal = 1; uint8_t endl = 4;
            if (inputpos == 10) {
                    switch (inputbuf[9]) {
                    case 'a':
                    case 'A':
                            which = 1; break;
                    case 'b':
                    case 'B':
                            which = 2; break;
                    case 'c':
                    case 'C':
                            which = 3; break;
                    case 'd':
                    case 'D':
                            which = 4; break;
                    };
            }
            if (which) {
                    stal = which;
                    endl = which;
            }
            for (which = stal; which <= endl; which++) {
              uint8_t pstatus;
              switch (which) {
              /* case 1: pstatus = PINA;
                      console_printpgm_noirq_P(PSTR("PINA: 0x"));
                      break;*/
              case 2: pstatus = PINB;
                      console_printpgm_noirq_P(PSTR("PINB: 0x"));
                      break;
              case 3: pstatus = PINC;
                      console_printpgm_noirq_P(PSTR("PINC: 0x"));
                      break;
              case 4: pstatus = PIND;
                      console_printpgm_noirq_P(PSTR("PIND: 0x"));
                      break;
              default:
                      pstatus = 0;
                      console_printpgm_noirq_P(PSTR("NOPIN: 0x"));
                      break;
              };
              console_printhex8_noirq(pstatus);
              appendchar(' ');
              console_printbin8_noirq(pstatus);
              if (which < endl) {
                appendchar('\r'); appendchar('\n');
              }
            }
          } else if (strcmp_P(inputbuf, PSTR("status")) == 0) {
            char tmpbuf[20]; uint8_t bpfrac;
            console_printpgm_noirq_P(PSTR("Status / last measured values:\r\n"));
            console_printpgm_noirq_P(PSTR("Barometric Pressure: "));
            bpfrac = barpress & 0x03;
            sprintf(tmpbuf, "%lu.%02u", (barpress >> 2), (bpfrac * 25));
            console_printtext_noirq(tmpbuf);
            console_printpgm_noirq_P(PSTR(" Pa\r\n"));
            console_printpgm_noirq_P(PSTR("BPraw: "));
            console_printhex8_noirq((barpress >> 24) & 0xff);
            console_printhex8_noirq((barpress >> 16) & 0xff);
            console_printhex8_noirq((barpress >>  8) & 0xff);
            console_printhex8_noirq((barpress >>  0) & 0xff);
            console_printpgm_noirq_P(PSTR("\r\n"));
            console_printpgm_noirq_P(PSTR("Temperature: "));
            bpfrac = temp & 0x0f;
            sprintf(tmpbuf, "%d.%02u", (temp >> 4), (bpfrac * 25 / 4));
            console_printtext_noirq(tmpbuf);
            console_printpgm_noirq_P(PSTR(" degC\r\n"));
            console_printpgm_noirq_P(PSTR("Traw: "));
            console_printhex8_noirq((temp >>  8) & 0xff);
            console_printhex8_noirq((temp >>  0) & 0xff);
            console_printpgm_noirq_P(PSTR("\r\n"));
          } else {
            console_printpgm_noirq_P(PSTR("Unknown command: "));
            console_printtext_noirq(inputbuf);
          }
          /* show PROMPT and go back to start. */
          console_printpgm_noirq_P(PROMPT);
          inputpos = 0;
          break;
  default:
          if (inputpos < (INPUTBUFSIZE - 1)) { /* -1 for terminating \0 */
                  inputbuf[inputpos++] = inpb;
                  /* Echo the character */
                  appendchar(inpb);
          } else {
                  appendchar(7); /* Bell */
          }
  };
}

/* Handler for RXC (RX Complete) IRQ. */
ISR(USART_RX_vect) {
  uint8_t inpb;
  
  inpb = UDR0;
  console_inputchar(inpb);
}
