/*
 * pdi_physical.c
 *
 *  Created on: Jan 12, 2017
 *      Author: ceragioliluca
 */

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usart.h"

#include "pdi.h"
#include "pdi_physical.h"
#include "pdi_const.h"

unsigned char pdi_on = 0;
unsigned int char_us;

PdiResult pdi_init(unsigned int freq_khz){
	char_us = 12000/freq_khz; //break and idle character length in us
	//RX input, TX output low, CLK output low
	PDI_PORT &= ~((1<<TX) | (1<<RX) | (1<<CLK));
	PDI_DDR &= ~(1<<RX);
	PDI_DDR |= (1<<CLK) | (1<<TX);

	//Setup USART
	long brr = (F_CPU/2000)/freq_khz;
	usart_setup(brr);

	_delay_us(10); //TODO: check if could be removed
	PDI_PORT |= 1<<TX; //Disable reset and wait at least 95 ns and opt 100us

	usart_tx_mode();
	pin_default_state();
	send_idles(2);

	//set idle bits between dir. change to 16
	send_byte(PDI_STORE_CONTROL_REG | PDI_CTRL_REG);
	send_byte(0x03);
	send_idle();

	//reset device
	send_byte(PDI_STORE_CONTROL_REG | PDI_RESET_REG);
	send_byte(PDI_RESET_SIGNATURE);
	send_idle();

	//provide 64bit programming key - little endian
	send_byte(PDI_KEY);
	send_byte(0xFF);
	send_byte(0x88);
	send_byte(0xD8);
	send_byte(0xCD);
	send_byte(0x45);
	send_byte(0xAB);
	send_byte(0x89);
	send_byte(0x12);
	send_idles(2);

	//poll Non Volatile Memory Enable bit
	unsigned char retry = 200;
	for(; retry > 0; retry--){
		send_byte(PDI_LOAD_CONTROL_REG | PDI_STATUS_REG);
		unsigned char status;
		if(read_byte(&status))
			return TIME_OUT;
		if(status & (1<<NVMEN))break;
		send_idles(2);
	}
	if(retry == 0) return TIME_OUT;

	pdi_on = 1;
	return NO_ERROR;
}


PdiResult pdi_read_byte(unsigned long address, unsigned char *data){
	if(pdi_on == 0) return PDI_DISABLED;
	send_byte(PDI_LOAD_DIRECT | (LONG<<2) | (BYTE<<0));
	for(char c = 0; c<4; c++){
		send_byte(address&0xFF);
		address>>=8;
	}
	if(read_byte(data)) return TIME_OUT;
	//TODO:check for error
	return NO_ERROR;
}

PdiResult pdi_write_byte(unsigned long address, unsigned char data){
	if(pdi_on == 0) return PDI_DISABLED;
	send_byte(PDI_STORE_DIRECT | (LONG<<2) | (BYTE<<0));
	for(char c = 0; c<4; c++){
		send_byte(address);
		address>>=8;
	}
	send_byte(data);
	return NO_ERROR;
}

PdiResult pdi_read_bytes(unsigned long start_address, unsigned char *data, unsigned int count){
	if(pdi_on == 0) return PDI_DISABLED;
	unsigned int count_tmp = count-1;
	//set pointer
	send_byte(PDI_STORE_INDIRECT | (PTR<<2) | LONG);
	for(char c = 0; c<4; c++){
		send_byte(start_address);
		start_address>>=8;
	}
	//set repeat counter
	send_byte(PDI_REPEAT | WORD);
	send_byte(count_tmp);
	count_tmp>>=8;
	send_byte(count_tmp);
	//send first read instruction
	send_byte(PDI_LOAD_INDIRECT | (PTR_DATA_INC<<2) | BYTE);
	for(unsigned int c=0; c<count; c++){
		if(read_byte(data+c))
			return TIME_OUT;
	}

	return NO_ERROR;
}

PdiResult pdi_write_bytes(unsigned long start_address, unsigned char *data, unsigned int count){
	if(pdi_on == 0) return PDI_DISABLED;
	unsigned int count_tmp = count-1;
	//set pointer
	send_byte(PDI_STORE_INDIRECT | (PTR<<2) | LONG);
	for(char c = 0; c<4; c++){
		send_byte(start_address);
		start_address>>=8;
	}
	//set repeat counter
	send_byte(PDI_REPEAT | WORD);
	send_byte(count_tmp);
	count_tmp>>=8;
	send_byte(count_tmp);
	//send first read instruction
	send_byte(PDI_STORE_INDIRECT | (PTR_DATA_INC<<2) | BYTE);
	pdi_write_raw(data, count);
	return NO_ERROR;
}

void pdi_write_raw(unsigned char * data, unsigned int count){
	for(; count>0; count--, data++)
		send_byte(*data);
}

void pdi_exit(void){
	//always synchronize the protocol in case something went bad
	send_break();
	send_break();
	send_idle();
	send_idle();
	//end reset
	send_byte(PDI_STORE_CONTROL_REG | PDI_RESET_REG);
	send_byte(0);

	send_idle();
	send_idle();
	send_idle();
	send_idle();

	usart_disconnect();
	PDI_PORT &= ~((1<<CLK) | (1<<TX) | (1<<RX)); //disable pull ups
	PDI_DDR &= ~((1<<CLK) | (1<<TX) | (1<<RX)); //all input
	pdi_on = 0;
}

void send_break(void){
	PDI_DDR |= (1<<TX); //tx output low (from default state)
	usart_rx_mode();//put in rx mode to manually override data pin
	for(unsigned int c = 0; c<char_us; c+=10)
		_delay_us(9);
	PDI_DDR &= ~(1<<TX); //restore
	usart_tx_mode();
}

void send_idle(void){
	usart_wait_tx();
	usart_tx_mode();
	for(unsigned int c = 0; c<char_us; c+=10)
		_delay_us(9);
}

void send_idles(char n){
	for(char c = 0; c<n; c++)
		send_idle();
}

void pin_default_state(void){
	//RX input, TX input, CLK output
	PDI_PORT &= ~((1<<TX) | (1<<RX) | (1<<CLK));
	PDI_DDR &= ~((1<<RX) | (1<<TX));
	PDI_DDR |= (1<<CLK) ;
}
