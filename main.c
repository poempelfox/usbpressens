/* $Id: main.c $
 * main for JeeNode Weather Station, tieing all parts together.
 * (C) Michael "Fox" Meier 2016
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#include "console.h"
#include "mpl3115a2.h"
#include "timers.h"
#include "twi.h"

uint32_t barpress = 0;
int16_t temp = 0;

/* We need to disable the watchdog very early, because it stays active
 * after a reset with a timeout of only 15 ms. */
void dwdtonreset(void) __attribute__((naked)) __attribute__((section(".init3")));
void dwdtonreset(void) {
  MCUSR = 0;
  wdt_disable();
}

int main(void) {
  uint16_t lastts = 0;
  /* Initialize stuff */
  console_init();
  timers_init();
  twi_init();
  _delay_ms(100); /* Give the chip some time to start up */
  mpl3115a2_init();
  
  /* Enable watchdog timer with a timeout of 8 seconds */
  wdt_enable(WDTO_8S); /* Longest possible on ATmega328P */

  /* Prepare sleep mode */
  /* We really have no choice if we're using serial, */
  /* SLEEP_MODE_IDLE is the only sleepmode we can safely use. But we usually */
  /* don't have power problems being connected to USB, so... */
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();

  /* All set up, enable interrupts and go. */
  sei();
  
  while (1) { /* Main loop, we should never exit it. */
    uint16_t curts = getticks();
    if ((curts - lastts) >= TICKSPERSECOND) { /* One second has passed */
      char outbuf[20]; uint8_t bpfrac;
      /* Get pressure */
      cli(); /* So that the serial debug console cannot get a corrupt value inbetween */
      barpress = mpl3115a2_getpressure();
      sei();
      cli(); /* So that the serial debug console cannot get a corrupt value inbetween */
      temp = mpl3115a2_gettemp();
      sei();
      /* Output the value on the serial console */
      console_printpgm_P(PSTR(">Barometric Pressure: "));
      sprintf(outbuf, "%lu", (barpress >> 2));
      console_printtext(outbuf);
      bpfrac = barpress & 0x03;
      switch (bpfrac) {
      case 0x00: console_printpgm_P(PSTR(".00")); break;
      case 0x01: console_printpgm_P(PSTR(".25")); break;
      case 0x02: console_printpgm_P(PSTR(".50")); break;
      case 0x03: console_printpgm_P(PSTR(".75")); break;
      };
      console_printpgm_P(PSTR(" Pa / "));
      sprintf(outbuf, "%lu", (barpress / 400));
      console_printtext(outbuf);
      console_printpgm_P(PSTR(" hPa\r\n"));
      lastts = curts;
    }
    wdt_reset();
    sleep_cpu(); /* Go to sleep until the next IRQ arrives */
  }
}
