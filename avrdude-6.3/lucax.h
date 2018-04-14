#ifndef lucax_h
#define lucax_h
/*
 * Luca X mega test programmer
 * */

#ifdef __cplusplus
extern "C" {
#endif

//usb pid and vid
#define LUCAX_VID 0x03EB
#define LUCAX_PID 0x206C

#define USB_HOSTTODEVICE 0
#define USB_DEVICETOHOST (1<<7)

#define USB_TIME_OUT_MS 1200

#define LUCAX_IN 	130
#define LUCAX_OUT	1

#define LUCAX_PDI_FLASH	 0
#define LUCAX_PDI_EEPROM 1

#define LUCAX_PDI_FUSE	0
#define LUCAX_PDI_LOCK	1

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

extern const char lucax_desc[];
void lucax_initpgm(PROGRAMMER * pgm);

#ifdef __cplusplus
}
#endif

#endif /* lucax_h */
