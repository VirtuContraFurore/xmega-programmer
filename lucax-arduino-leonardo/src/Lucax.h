/*
 * Lucax.h
 *
 *  Created on: Jan 24, 2017
 *      Author: ceragioliluca
 */

#ifndef SRC_LUCAX_H_
#define SRC_LUCAX_H_

/*USB control requests*/

//GENERAL
#define LUCAX_GET_DESCRIPTOR 		33
#define LUCAX_RELEASE_INTERFACE		34
#define LUCAX_REPORT_ERROR			42

//PDI specific
#define LUCAX_PDI_INIT				35
#define LUCAX_PDI_CHIP_ERASE		43
#define LUCAX_PDI_NVM_READ			36 /*Reads any nvm memory*/
#define LUCAX_PDI_NVM_WRITE_BYTE	38 /*write byte memory: FUSE and LOCKBITS*/
#define LUCAX_PDI_NVM_WRITE_PAGE	39 /*write paged memory: FLASH and EEPROM*/

#define LUCAX_PDI_RAW_READ			40
#define LUCAX_PDI_RAW_WRITE			41

#define MAX_BUFFER 1024

#include <avr/io.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include "Descriptors.h"
#include "SoftUsart.h"
#include "driver/pdi.h"
#include "nvm.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/Board/LEDs.h>
#include <LUFA/Platform/Platform.h>

void lucax_get_descriptor(void);
void lucax_release_interface(void);
void lucax_report_error(void);

void lucax_pdi_init(void);
void lucax_pdi_chip_erase(void);
void lucax_pdi_nvm_read(void);
void lucax_pdi_nvm_write_page(void);
void lucax_pdi_nvm_write_byte(void);
void lucax_pdi_raw_read(void);
void lucax_pdi_raw_write(void);

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);

#endif /* SRC_LUCAX_H_ */
