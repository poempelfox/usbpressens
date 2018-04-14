/* $Id: mpl3115a2.h $
 * Functions for talking to the MPL3115A2 pressure sensor.
 */

#ifndef _MPL3115A2_H_
#define _MPL3115A2_H_

/* Initialize the MPL3115A2 */
void mpl3115a2_init(void);

uint32_t mpl3115a2_getpressure(void);
int16_t mpl3115a2_gettemp(void);

#endif /* _MPL3115A2_H_ */
