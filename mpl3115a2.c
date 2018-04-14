/* $Id: mpl3115a2.c $
 * Functions for talking to the MPL3115A2
 */

#include <avr/io.h>
#include <util/delay.h>
#include "twi.h"
#include "mpl3115a2.h"

/* The address is 0x60, but of course we need to shift that
 * left, to make room for the R/W selection bit. */
#define MP13115A2_I2C_ADDR (0x60 << 1)

void mpl3115a2_init(void)
{
  twi_open(MP13115A2_I2C_ADDR | I2C_WRITE);
  /* Register 0x26 and autoincrement from there */
  twi_write(0x26 | I2C_AUTOINCREGADDR);
  /* 0x26 = CTRL_REG1: Enable sampling with maximum oversampling, barometer mode */
  twi_write(0x0b);
  /* 0x27 = CTRL_REG2: Autoacquisition every second, no other nonsense. */
  twi_write(0x00);
  twi_close();
}

uint32_t mpl3115a2_getpressure(void)
{
  uint32_t res = 0;
  twi_open(MP13115A2_I2C_ADDR | I2C_WRITE);
  /* Start with the OUT_P_MSB register at 0x01 */
  twi_write(0x01 | I2C_AUTOINCREGADDR);
  twi_close();
  _delay_us(1);
  twi_open(MP13115A2_I2C_ADDR | I2C_READ);
  res |= ((uint32_t)twi_read(1)) << 16;
  res |= ((uint32_t)twi_read(1)) <<  8;
  res |= ((uint32_t)twi_read(0)) <<  0;
  twi_close();
  /* we need to shift the return value right because it is 20 bits left aligned */
  return (res >> 4);
}

int16_t mpl3115a2_gettemp(void)
{
  uint16_t res = 0;
  twi_open(MP13115A2_I2C_ADDR | I2C_WRITE);
  /* Start with the OUT_T_MSB register at 0x04 */
  twi_write(0x04 | I2C_AUTOINCREGADDR);
  twi_close();
  _delay_us(1);
  twi_open(MP13115A2_I2C_ADDR | I2C_READ);
  res |= ((uint16_t)twi_read(1)) <<  8;
  res |= ((uint16_t)twi_read(0)) <<  0;
  twi_close();
  /* we need to shift the return value right because it is 12 bits left aligned */
  return (res / 16);
}
