/* $Id: mpl3115a2.c $
 * Functions for talking to the MPL3115A2
 */

#include <avr/io.h>
#include "twi.h"
#include "mpl3115a2.h"

#define MP13115A2_I2C_ADDR 0x60

void mpl3115a2_init(void)
{
  twi_open(MP13115A2_I2C_ADDR | I2C_WRITE);
  /* Register 0x26 and autoincrement from there */
  twi_write(0x26 | I2C_AUTOINCREGADDR);
  /* 0x26 = CTRL_REG1: Enable sampling with maximum oversampling, barometer mode */
  twi_write(0x39);
  /* 0x27 = CTRL_REG2: Autoacquisition every second, no other nonsense. */
  twi_write(0x00);
  twi_close();
}

uint32_t mpl3115a2_getpressure(void)
{
  uint32_t res = 0;
  twi_open(MP13115A2_I2C_ADDR | I2C_WRITE);
  /* Start with the register at 0x01 */
  twi_write(0x01 | I2C_AUTOINCREGADDR);
  twi_close();
  twi_open(MP13115A2_I2C_ADDR | I2C_READ);
  res |= ((uint32_t)twi_read(1)) << 16;
  res |= ((uint32_t)twi_read(1)) <<  8;
  res |= ((uint32_t)twi_read(0)) <<  0;
  twi_close();
  return res;
}
