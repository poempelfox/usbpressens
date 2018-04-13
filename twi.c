/* $Id: twi.c $
 * Functions for handling the I2C-compatible TWI bus.
 * We have multiple devices connected there, so this handles the common stuff.
 * Note: Pullups for the bus are on both the TSL45315 and the LPS331AP module,
 * so we do not enable the internal port pullups on the AVR.
 * Note: The plan was originally to keep the TSL45315 and LPS331AP sensors
 * permanently connected to +3V, because the chips power draw when idle is in
 * the uA range, and the modules have rather large capacitors on them. Sadly,
 * that doesn't work, because both modules have level shifters in their data
 * lines that suck about 1,5 mA constantly. So the power for both modules is
 * supplied through the PB0 pin and turned off while they're not measuring.
 */

#include <avr/io.h>
#include "twi.h"

void twi_init(void)
{
  /* Set bitrate to 125kbps
   * - or as fast as is possible. Running at only 1 MHz the maximum is 62 kbps.
   * SCL frequency = CPUFREQ / (16 + (2 * TWBR * TWPS))
   * with TWBR = bitrate  TWPS = prescaler */
  TWBR = 0;
  /* The two TWPS bits are hidden in the status register */
  TWSR = 0; /* prescaler = 1 (that's the poweron default anyways) */
  /* PB0 is used as the power supply for our I2C devices. */
  DDRB |= _BV(PB0);
}

void twi_power(uint8_t p)
{
  if (p) {
    PRR &= (uint8_t)~_BV(PRTWI);
  } else {
    PRR |= _BV(PRTWI);
  }
}

static void waitforcompl(void)
{
  uint8_t abortctr = 0;
  while ((TWCR & _BV(TWINT)) == 0) {
    abortctr++; /* This is a needed workaround, else we'll just get stuck */
    if (abortctr > 200) { /* whenever a slave doesn't feel like ACKing. */
      break;
    }
  }
}

void twi_open(uint8_t addr)
{
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  waitforcompl();
  TWDR = addr;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  waitforcompl();
}

void twi_close(void)
{
  /* send stop condition */
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
  /* no waitforcompl(); after stop! TWINT is not set after STOP has been sent! */
  /* Instead wait for the transmission of the STOP */
  uint8_t abortctr = 0;
  while ((TWCR & _BV(TWSTO))) {
    abortctr++;
    if (abortctr > 200) {
      break;
    }
  }
  TWCR = _BV(TWINT); /* Disable TWI completely, it will be reenabled next open */
}

void twi_write(uint8_t what)
{
  TWDR = what;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  waitforcompl();
}

uint8_t twi_read(uint8_t ack)
{
  /* clear interrupt flag to start receiving. */
  TWCR = _BV(TWINT) | _BV(TWEN) | ((ack) ? _BV(TWEA) : 0x00);
  waitforcompl();
  return TWDR;
}

void twi_devicepower(uint8_t p)
{
  if (p) {
    PORTB |= _BV(PB0);
  } else {
    PORTB &= (uint8_t)~_BV(PB0);
  }
}
