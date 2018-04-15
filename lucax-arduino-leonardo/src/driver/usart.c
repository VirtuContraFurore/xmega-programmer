/*
 * usart.c
 *
 *  Created on: Jan 15, 2017
 *      Author: ceragioliluca
 */
#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "pdi_physical.h"
#include "usart.h"

typedef enum {UTX = 0, URX = 1} UsartState;

UsartState state;
char pending;

void usart_setup(unsigned int ubrr){
	UBRR1 = (ubrr&0xFFFF)-1; //baud
	UCSR1C = (1<<UMSEL10) | (1<<UPM11) | (1<<USBS1) | (1<<UCSZ11) | (1<<UCSZ10) | (1<<UCPOL1);
}

void usart_tx_mode(void){
	UCSR1B = 1<<TXEN1;
	state = UTX;
	pending = 0;
}

void usart_rx_mode(void){
	UCSR1B = 1<<RXEN1;
	state = URX;
}

void usart_disconnect(void){
	UCSR1B = 0;
}


void send_byte(unsigned char data){
	if(state == URX)
		usart_tx_mode();
	while(!(UCSR1A & (1<<UDRE1))); //wait until the transmitter buffer is ready to accept new data
	UDR1 = data;
	UCSR1A |= 1<<TXC1; //clear transmit completed flag
	pending = 1;
}

//TODO: add time out control
unsigned char read_byte(unsigned char *data){
	if(state == UTX){
		while (!(UCSR1A & (1<<TXC1)));//check if transmission is completed before disabling the transmitter
		usart_rx_mode();
		pending = 0;
	}
	unsigned char retry = 250;
	while (!(UCSR1A & (1<<RXC1))){
		retry--;
		_delay_us(15);
		if(retry == 0) return 0x01;
	}
	*data = UDR1;
	return 0;
}

void usart_wait_tx(void){
	if((state == UTX) && (pending == 1)){
		while (!(UCSR1A & (1<<TXC1)));
		pending = 0;
	}
}
