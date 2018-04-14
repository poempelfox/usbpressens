/* $Id: ledfewi.c $
 * Functions for talking to the Feather Wing with the LEDs (Quad Alphanumeric Diplays).
 * This is essentially an I2C connected HT16K33 display controller.
 */

#include <avr/io.h>
#include <util/delay.h>
#include "twi.h"
#include "ledfewi.h"

/* The address is 0x70, but of course we need to shift that
 * left, to make room for the R/W selection bit. */
#define LEDFEWI_I2C_ADDR (0x70 << 1)

/* The I2C communication is a bit weird on this one.
 * These defines are not really registers usually,
 * but the first 4 bit selects the register, and the second 4 bit is the value.
 * An exception for this is the actual display registers. */
#define HT16K33_CMD_BRIGHTNESS 0xE0
#define HT16K33_CMD_SYSTEMSETUPREG 0x20
#define HT16K33_CMD_DISPLAYSETUPREG 0x80

/* From the Adafruit documentation:
 *        A
 *     -------
 *    |\J>|  /|
 * F> | \ | / | <B
 *    |H>\|/<K|
 *    |---X---| <G1 (left) and G2 (right)
 *    |L>/|\<N|
 * E> | / | \ | <C
 *    |/M>|  \|
 *     -------  O <DP
 *        D
 *
 * This maps into the "value" bits as follows:
 * 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
 *  0 DP  N  M  L  K  J  H G2 G1  F  E  D  C  B  A
 *
 * Ignore the following line, this is just here as a C&P help:
 *  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
 */

/* We provide a sort of default font. */
uint16_t ledfewi_getfontentry(uint8_t c)
{
  switch (c) {
  case '0': return 0x003F;
  case '1': return 0x0006;
  case '2': return 0x088B;
  case '3': return 0x00CF;
  case '4': return 0x00E6;
  case '5': return 0x00ED;
  case '6': return 0x00FD;
  case '7': return 0x0C01;
  case '8': return 0x00FF;
  case '9': return 0x00EF;
  case '.': return 0x4000; /* This is best ORed onto something else */
  };
  return 0x0000;
}

void ledfewi_init(void)
{
  /* Turn on the oscillator */
  twi_open(LEDFEWI_I2C_ADDR | I2C_WRITE);
  twi_write(HT16K33_CMD_SYSTEMSETUPREG | 0x01);
  twi_close();
  _delay_us(1);
  /* Turn on the display */
  twi_open(LEDFEWI_I2C_ADDR | I2C_WRITE);
  twi_write(HT16K33_CMD_DISPLAYSETUPREG | 0x01);
  twi_close();
}

void ledfewi_setbrightness(uint8_t bri)
{
  bri = bri & 0x0f; /* Only 0-15 is valid */
  twi_open(LEDFEWI_I2C_ADDR | I2C_WRITE);
  twi_write(HT16K33_CMD_BRIGHTNESS | bri);
  twi_close();
}

void ledfewi_setraw_and(uint8_t displaynumber, uint16_t value)
{
  twi_open(LEDFEWI_I2C_ADDR | I2C_WRITE);
  twi_write((0x00 + (displaynumber * 2)) /* NOT | I2C_AUTOINCREG */);
  twi_write((value >> 0) & 0xff);
  twi_write((value >> 8) & 0xff);
  twi_close();
}
