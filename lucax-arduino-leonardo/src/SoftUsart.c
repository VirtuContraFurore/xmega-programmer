/*
 * SoftUsart.c
 *
 *  Created on: Jan 28, 2017
 *      Author: ceragioliluca
 */

#include "SoftUsart.h"

void static print_nibble(unsigned char n);

/*Init port operation*/
void SoftUsart_setup(void){
	//set idle condition and wait
	SOFT_USART_DDR |= 1<<SOFT_USART_PIN;
	SOFT_USART_HIGH();
	_delay_ms(100);
}

/*Send a char*/
void SoftUsart_send_char(unsigned char data){
	//start bit
	SOFT_USART_LOW();
	SOFT_USART_BIT_DELAY();
	//data LSB
	for(char bit = 1; bit != 0; bit<<=1){
		if(bit & data)
			SOFT_USART_HIGH();
		else
			SOFT_USART_LOW();
		SOFT_USART_BIT_DELAY();
	}
	//stop bits
	SOFT_USART_HIGH();
	SOFT_USART_BIT_DELAY();
#if SOFT_USART_STOP_BITS == 2
	SOFT_USART_BIT_DELAY();
#endif
}

/*Send a null terminated string*/
void SoftUsart_send_str(char *s){
	while(*s)
		SoftUsart_send_char(*(s++));
}

/*Print a null terminated string followed by \n \t*/
void SoftUsart_print(char *c){
	SoftUsart_send_str(c);
	SoftUsart_send_char('\n');
	SoftUsart_send_char('\r');
}

/*Print a number in hex format*/
void SoftUsart_print_hex(unsigned int h){
	unsigned char c = h>>8;
	if(c!=0){
		print_nibble(c>>4);
		print_nibble(c);
	}
	c = h&0xff;
	print_nibble(c>>4);
	print_nibble(c);
}

void static print_nibble(unsigned char n){
	unsigned char d;
	n&=0x0F;
	switch(n){
	case 0:
		d = '0';
		break;
	case 1:
		d = '1';
		break;
	case 2:
		d = '2';
		break;
	case 3:
		d = '3';
		break;
	case 4:
		d = '4';
		break;
	case 5:
		d = '5';
		break;
	case 6:
		d = '6';
		break;
	case 7:
		d = '7';
		break;
	case 8:
		d = '8';
		break;
	case 9:
		d = '9';
		break;
	case 10:
		d = 'A';
		break;
	case 11:
		d = 'B';
		break;
	case 12:
		d = 'C';
		break;
	case 13:
		d = 'D';
		break;
	case 14:
		d = 'E';
		break;
	case 15:
		d = 'F';
		break;
	}
	SoftUsart_send_char(d);
}
