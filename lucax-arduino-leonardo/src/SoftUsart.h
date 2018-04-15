/*
 * SoftUsart.h
 *
 *  Created on: Jan 28, 2017
 *      Author: ceragioliluca
 */

#ifndef SRC_SOFTUSART_H_
#define SRC_SOFTUSART_H_

#include <avr/io.h>
#include <util/delay.h>

/*Configure these two macros to use the pin you want*/
#define SOFT_USART_PORT_CHAR 	D
#define SOFT_USART_PORT_NUMBER	4

#define SOFT_USART_BAUDRATE 	57600
#define SOFT_USART_STOP_BITS	1

//macros used by the library only

/*these macros are used only to merge port names*/
#define sup(A, B) P ## A ## B
#define sup1(A, B) sup(A, B)
#define sud(A) DDR ## A
#define sud1(A) sud(A)
#define suP(A) PORT ## A
#define suP1(A) suP(A)

#define SOFT_USART_PIN 	sup1(SOFT_USART_PORT_CHAR, SOFT_USART_PORT_NUMBER)
#define SOFT_USART_PORT	suP1(SOFT_USART_PORT_CHAR)
#define SOFT_USART_DDR 	sud1(SOFT_USART_PORT_CHAR)

#define SOFT_USART_HIGH() 		SOFT_USART_PORT |= (1<<SOFT_USART_PIN)
#define SOFT_USART_LOW()		SOFT_USART_PORT &= ~(1<<SOFT_USART_PIN)
#define SOFT_USART_BIT_DELAY()	_delay_us(1000000/SOFT_USART_BAUDRATE)

/*Init port operation*/
void SoftUsart_setup(void);

/*Send a char*/
void SoftUsart_send_char(unsigned char c);

/*Send a null terminated string*/
void SoftUsart_send_str(char *c);

/*Print a null terminated string followed by \n \t*/
void SoftUsart_print(char *c);

/*Print a number in hex format*/
void SoftUsart_print_hex(unsigned int h);

#endif /* SRC_SOFTUSART_H_ */
