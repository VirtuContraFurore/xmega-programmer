/*
 * nvm.c
 *
 *  Created on: Jan 19, 2017
 *      Author: ceragioliluca
 */

#include "nvm.h"
#include "driver/pdi.h"
#include "driver/pdi_physical.h"
#include "driver/pdi_const.h"

#include <util/delay.h>

void nvm_pointer(unsigned long c){
	unsigned char data[5];
	data[0] = PDI_STORE_INDIRECT | (PTR<<2) | LONG;
	data[1] = c;
	data[2] = c>>8;
	data[3] = c>>16;
	data[4] = c>>24;
	pdi_write_raw(data, 5);
}

void  nvm_cmd(unsigned char c){
	pdi_write_byte(PDI_MEMORY_BASE + NVM_BASE + NVM_CMD, c);
}


void  trig_cmdex(void){
	pdi_write_byte(PDI_MEMORY_BASE + NVM_BASE + NVM_CTRLA, 0x01);
}

PdiResult wait_nvm(void){
	unsigned char wait = 0xff;
	while(wait--){
		unsigned char status;
		unsigned char ret = pdi_read_byte(PDI_MEMORY_BASE + NVM_BASE + NVM_STATUS, &status);
		if(ret != NO_ERROR)
			return ret;
		if((status & (1<<7)) == 0) //Poll NVMBUSY flag
			break;
		_delay_ms(1);
	}
	if(wait == 0)
		return TIME_OUT;
	return NO_ERROR;
}

PdiResult nvm_chip_erase(){
	nvm_cmd(CHIP_ERASE);
	trig_cmdex();
	return wait_nvm();
}

PdiResult nvm_read_byte(unsigned long address, unsigned char * data){
	nvm_cmd(READ_NVM); //read nvm
	return pdi_read_byte(address, data);
}


PdiResult nvm_read_bytes(unsigned long address, unsigned char * data, unsigned int count){
	nvm_cmd(READ_NVM); //read nvm
	return pdi_read_bytes(address, data, count);
}


PdiResult nvm_write_byte(unsigned long address, unsigned char data, ByteMemory mem){
	nvm_cmd((mem == LOCK) ? WRITE_LOCK : WRITE_FUSE);
	pdi_write_byte(address, data);

	if(mem == LOCK)
		trig_cmdex();

	return wait_nvm();
}

PdiResult nvm_paged_write(unsigned long address, unsigned char * data, unsigned int count, PagedMemory mem){
	//erase page buffer
	nvm_pointer(address);
	nvm_cmd((mem == FLASH) ? ERASE_FLASH_PAGE_BUFFER : ERASE_EEPROM_PAGE_BUFFER);
	trig_cmdex();
	wait_nvm();

	//load page buffer
	nvm_cmd((mem == FLASH) ? LOAD_FLASH_PAGE_BUFFER : LOAD_EEPROM_PAGE_BUFFER);
	pdi_write_bytes(address, data, count);

	//write page
	nvm_cmd((mem == FLASH) ? ERASE_WRITE_FLASH_PAGE : ERASE_WRITE_EEPROM_PAGE); //erase and write
	nvm_pointer(address);
	unsigned char dummy[] = {PDI_STORE_INDIRECT | (PTR_DATA_INC<<2), 0x00};	//perform a dummy write to trigger previous command
	pdi_write_raw(dummy, sizeof(dummy));

	return wait_nvm();
}

