/*
 * pdi_physical.h
 *
 *  Created on: Jan 12, 2017
 *      Author: ceragioliluca
 */

#ifndef SRC_DRIVER_PDI_PHYSICAL_H_
#define SRC_DRIVER_PDI_PHYSICAL_H_

#define PDI_CLOCK 110000 //in Hz - minimum is 10000 Hz

//Pins
#define CLK PD5
#define TX PD3
#define RX PD2
#define PDI_PORT PORTD
#define PDI_DDR DDRD

void pin_default_state(void);
void send_break(void);
void send_idle(void);
void send_idles(char n);

#endif /* SRC_DRIVER_PDI_PHYSICAL_H_ */
