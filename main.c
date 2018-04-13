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
#include "timers.h"
#include "twi.h"

/* We need to disable the watchdog very early, because it stays active
 * after a reset with a timeout of only 15 ms. */
void dwdtonreset(void) __attribute__((naked)) __attribute__((section(".init3")));
void dwdtonreset(void) {
  MCUSR = 0;
  wdt_disable();
}

int main(void) {
  /* Initialize stuff */
  console_init();
  timers_init();
  twi_init();
  
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
    wdt_reset();
    sleep_cpu(); /* Go to sleep until the next IRQ arrives */
  }
}
