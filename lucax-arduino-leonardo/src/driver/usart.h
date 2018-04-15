/*
 * usart.h
 *
 *  Created on: Jan 15, 2017
 *      Author: ceragioliluca
 */

#ifndef SRC_DRIVER_USART_H_
#define SRC_DRIVER_USART_H_

//static void disable_usart(); //really?
void usart_tx_mode(void);
void usart_rx_mode(void);
void usart_disconnect(void);
void usart_setup(unsigned int ubrr); //8 bit, even parity, 2 stop bits, sampling on clock's rising edge - synchronous operation

void usart_wait_tx(void);

void send_byte(unsigned char data);
unsigned char read_byte(unsigned char *data);


#endif /* SRC_DRIVER_USART_H_ */
