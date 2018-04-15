/*
 * pdi_const.h
 *
 *  Created on: Jan 15, 2017
 *      Author: ceragioliluca
 */

#ifndef SRC_DRIVER_PDI_CONST_H_
#define SRC_DRIVER_PDI_CONST_H_

//INSTRUCTIONS
#define PDI_LOAD_DIRECT			0x00
#define PDI_STORE_DIRECT		0x40
#define PDI_LOAD_INDIRECT		0x20
#define PDI_STORE_INDIRECT		0x60
#define PDI_LOAD_CONTROL_REG	0x80
#define PDI_STORE_CONTROL_REG	0xC0
#define PDI_REPEAT				0xA0
#define PDI_KEY				0xE0

//CONTROL AND STATUS REGISTER ADDRESSES
#define PDI_STATUS_REG	0x00
#define NVMEN 1
#define PDI_RESET_REG	0x01
#define PDI_CTRL_REG	0x02

#define PDI_RESET_SIGNATURE 0x59

//DATA SIZE
#define LONG	0x03
#define WORD	0x01
#define BYTE	0x00

#define PTR				0x02
#define PTR_DATA		0x00
#define PTR_DATA_INC	0x01


#endif /* SRC_DRIVER_PDI_CONST_H_ */
