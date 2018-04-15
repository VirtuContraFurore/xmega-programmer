/*
 * nvm.h
 *
 *  Created on: Jan 15, 2017
 *      Author: ceragioliluca
 */

#ifndef SRC_NVM_H_
#define SRC_NVM_H_

#include "driver/pdi.h"

//MEMORY SPACE ADDRESS (32 bit)
#define PDI_FLASH_BASE 			0x00800000
#define PDI_EEPROM_BASE			0x008C0000
#define PDI_SIGNATURE_BASE  	0x008E0200
#define PDI_CALIBRATION_BASE	0x008E0400
#define PDI_FUSES_BASE			0x008F0020
#define PDI_MEMORY_BASE			0x01000000

#define NVM_BASE 	0x01C0
#define NVM_CMD		0x000A
#define NVM_CTRLA	0x000B
#define NVM_STATUS	0x000F

//NVM COMMANDS
#define CHIP_ERASE						0x40
#define READ_NVM						0x43
//Flash - Page Buffer
#define LOAD_FLASH_PAGE_BUFFER			0x23
#define ERASE_FLASH_PAGE_BUFFER 		0x26
//Flash - General
#define ERASE_FLASH_PAGE				0x2B
#define WRITE_FLASH_PAGE				0x2E
#define ERASE_WRITE_FLASH_PAGE			0x2F
//EEPROM - Page Buffer
#define LOAD_EEPROM_PAGE_BUFFER			0x33
#define ERASE_EEPROM_PAGE_BUFFER		0x36
//EEPROM - General
#define ERASE_EEPROM					0x30
#define ERASE_EEPROM_PAGE				0x32
#define WRITE_EEPROM_PAGE				0x34
#define ERASE_WRITE_EEPROM_PAGE			0x35
//FUSES and LOCKBITS
#define WRITE_FUSE						0x4C
#define WRITE_LOCK						0x08

typedef enum {FLASH=0, EEPROM=1} PagedMemory;
typedef enum {FUSE = 0, LOCK = 1} ByteMemory;


/*perform a chip erase*/
PdiResult nvm_chip_erase(void);

/*
 * Reads any non volatile memory (flash, EEPROM, SROM, FUSES)
 * Only addresses between 0x00800000 and 0x01000000 are allowed
 * If you need to read I/O Memory issue a standard pdi_read call
 * */
PdiResult nvm_read_byte(unsigned long address, unsigned char * data);
PdiResult nvm_read_bytes(unsigned long address, unsigned char * data, unsigned int count);

PdiResult nvm_write_byte(unsigned long address, unsigned char data, ByteMemory mem);

/*
 * Writes paged memory memory
 * data and count should fit the page size of the device being programmed
 * */
PdiResult nvm_paged_write(unsigned long address, unsigned char * data, unsigned int count, PagedMemory mem);

void nvm_pointer(unsigned long ptr);
void nvm_cmd(unsigned char c);
void trig_cmdex(void);
PdiResult wait_nvm(void);


#endif /* SRC_NVM_H_ */
