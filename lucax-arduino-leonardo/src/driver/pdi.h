/*
 * pdi.h
 *
 *  Created on: Jan 12, 2017
 *      Author: ceragioliluca
 */
/*PDI generic driver - high level interface for ATxmega microcontrollers*/

#ifndef SRC_DRIVER_PDI_H_
#define SRC_DRIVER_PDI_H_

/*Operation result constants*/
typedef enum {NO_ERROR = 0, PDI_DISABLED = 1, TIME_OUT = 2, INVALID_ADDRESS = 3} PdiResult;

/*Exit PDI mode and let the device execute its own code*/
void pdi_exit(void);

/*Initialize the PDI controller enabling the use of any other PDI function*/
PdiResult pdi_init(unsigned int freq_khz);

/*These functions read and write a single byte to the PDI address space*/
PdiResult pdi_read_byte(unsigned long address, unsigned char *data);

PdiResult pdi_write_byte(unsigned long address, unsigned char data);

/*These functions read and write multiple bytes to the PDI address space*/
PdiResult pdi_read_bytes(unsigned long start_address, unsigned char *data, unsigned int count);

PdiResult pdi_write_bytes(unsigned long start_address, unsigned char *data, unsigned int count);

void pdi_write_raw(unsigned char * data, unsigned int count);


#endif /* SRC_DRIVER_PDI_H_ */
