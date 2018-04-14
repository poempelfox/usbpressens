/* $Id: ledfewi.h $
 * Functions for talking to the Feather Wing with the LEDs (Quad Alphanumeric Diplays)
 */

#ifndef _LEDFEWI_H_
#define _LEDFEWI_H__

/* Initialize the LED Feather Wing */
void ledfewi_init(void);

/* Set the display brightness (valid range: 0-15) */
void ledfewi_setbrightness(uint8_t bri);

/* Set the raw values for one alphanumeric display */
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
 * This maps into the "raw value" bits as follows:
 * 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
 *  0 DP  N  M  L  K  J  H G2 G1  F  E  D  C  B  A
 */
void ledfewi_setraw_and(uint8_t displaynumber, uint16_t value);

/* If you need the 'raw value' for the function above, you can also get it
 * through this function, which provides a default font: */
uint16_t ledfewi_getfontentry(uint8_t c);

#endif /* _LEDFEWI_H_ */
