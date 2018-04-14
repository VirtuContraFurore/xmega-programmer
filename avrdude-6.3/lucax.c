/*
 * Luca X mega test programmer
 * */

/*Check for libusb-1.0 since lucax use this version*/
#include "ac_cfg.h"
#ifndef HAVE_LIBUSB_1_0
#error Error: libusb-1.0 must be installed
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>

#include <libusb-1.0/libusb.h>

#include "avrdude.h"
#include "libavrdude.h"

#include "lucax.h"

const char lucax_desc[] = "Custom Luca xmega test programmer";

#define LUCAX_IO_UNSUPPORTED 	0x00
#define LUCAX_IO_PDI			0x01

#define LUCAX_HAS_PDI			0x01

struct pdata {
	libusb_context * ctx;
	libusb_device_handle * dev;

	unsigned char lucax_info; //lucax information descriptor
	int io_frequency; //general input/output frequency
	int lucax_io_mode; //current state of the programmer
};

#define PDATA(pgm) ((struct pdata *)(pgm->cookie))
#define IMPORT_PDATA(pgm) struct pdata *pdata = PDATA(pgm)

//interface 
static void lucax_setup(PROGRAMMER * pgm);
static void lucax_teardown(PROGRAMMER * pgm);

//programmer
static int lucax_open(PROGRAMMER * pgm, char * port);
static int lucax_init(PROGRAMMER * pgm, AVRPART * p);
static void lucax_close(PROGRAMMER * pgm);

static int lucax_chip_erase(PROGRAMMER * pgm, AVRPART * p);
static int lucax_set_sck_period(PROGRAMMER * pgm, double v);
static int lucax_read_byte(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m, unsigned long addr, unsigned char * value);
static int lucax_paged_load(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m, unsigned int page_size, unsigned int baseaddr, unsigned int n_bytes);
static int lucax_paged_write(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m, unsigned int page_size, unsigned int addr, unsigned int n_bytes);
static int lucax_write_byte(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m, unsigned long addr, unsigned char value);

int lucax_get_error(PROGRAMMER * pgm);

void lucax_dummy_read(PROGRAMMER * pgm);

void lucax_initpgm(PROGRAMMER * pgm){
	strcpy(pgm->type, "lucax");
	avrdude_message(MSG_DEBUG, "\n*** LucaX initpgm called\n");
	pgm->open = lucax_open;
	pgm->initialize = lucax_init;
	pgm->setup = lucax_setup;
	pgm->teardown = lucax_teardown;
	pgm->close = lucax_close;
	pgm->read_byte = lucax_read_byte;
	pgm->paged_load = lucax_paged_load;
	pgm->paged_write = lucax_paged_write;
	pgm->write_byte = lucax_write_byte;

}

static int lucax_open(PROGRAMMER * pgm, char * port){
	avrdude_message(MSG_DEBUG, "\n*** LucaX open called\n", port);
	IMPORT_PDATA(pgm);
	pdata->ctx = NULL; //a libusb session
	if((libusb_init(&pdata->ctx)) < 0) {
			avrdude_message(MSG_INFO, "\nError calling libusb_init()\n");
			return -1;
	}

	//open lucax device
	pdata->dev = libusb_open_device_with_vid_pid(pdata->ctx, LUCAX_VID, LUCAX_PID);
	if(pdata->dev==NULL){
		avrdude_message(MSG_INFO, "\nError opening lucax programmer \ncan't find a device with vid 0x%04x and pid 0x%04x", LUCAX_VID, LUCAX_PID);
		return -1;
	} else
		avrdude_message(MSG_INFO, "\nLucax programmer's USB port opened\n");

	return 0;
}

static int lucax_init(PROGRAMMER * pgm, AVRPART * part){
	avrdude_message(MSG_DEBUG, "\n*** LucaX init part called\n");
	IMPORT_PDATA(pgm);
	//get descriptor
	if(libusb_control_transfer(pdata->dev, USB_DEVICETOHOST, LUCAX_GET_DESCRIPTOR, 0, 0, &pdata->lucax_info, 1, USB_TIME_OUT_MS) < 0){
		avrdude_message(MSG_INFO, "\nLucaX error getting lucax descriptor\n");
		return -1;
	}

	pdata->lucax_io_mode = ((part->flags & AVRPART_HAS_PDI) && (pdata->lucax_info & LUCAX_HAS_PDI)) ? LUCAX_IO_PDI : LUCAX_IO_UNSUPPORTED;

	if(pdata->lucax_io_mode == LUCAX_IO_PDI){
		//INIT PDI
		if(pdata->io_frequency<15 || pdata->io_frequency>1000) pdata->io_frequency = 250;
			avrdude_message(MSG_INFO, "Setting PDI frequency to %i", pdata->io_frequency, " KHz\n");

		unsigned char status;
		if(libusb_control_transfer(pdata->dev, USB_DEVICETOHOST, LUCAX_PDI_INIT, pdata->io_frequency&0xFFFF, 0, &status, 1, USB_TIME_OUT_MS) < 0){
			avrdude_message(MSG_INFO, "\nLucaX usb error\n");
			return -1;
		}

		if(status != 0){
			avrdude_message(MSG_INFO, "\nLucaX error initiating pdi module\n");
			return -1;
		} else
			avrdude_message(MSG_INFO, "\nLucax PDI online\n");
		//SET FUNC POINTER
		pgm->chip_erase = lucax_chip_erase;
		pgm->set_sck_period = lucax_set_sck_period;


	} else {
		avrdude_message(MSG_INFO, "\nError: LucaX programmer supports PDI only devices (xmega serie)\n");
		return -1;
	}
	return  0;
}

static void lucax_setup(PROGRAMMER * pgm){
	avrdude_message(MSG_DEBUG, "\n*** LucaX setup called\n");
	if ((pgm->cookie = malloc(sizeof(struct pdata))) == 0) {
	   avrdude_message(MSG_INFO, "%s: lucax_setup(): Out of memory allocating private data\n", progname);
	    exit(1);
	}
	memset(pgm->cookie, 0, sizeof(struct pdata));
}

static void lucax_close(PROGRAMMER * pgm){
	avrdude_message(MSG_DEBUG, "\n*** LucaX close called\n");
	IMPORT_PDATA(pgm);

	libusb_control_transfer(pdata->dev, 0, LUCAX_RELEASE_INTERFACE, 0, 0, NULL, 0, USB_TIME_OUT_MS);

	//free device and close the session opened
	libusb_close(pdata->dev);
	libusb_exit(pdata->ctx);
}

static void lucax_teardown(PROGRAMMER * pgm){
	avrdude_message(MSG_DEBUG, "\n*** LucaX teardown called\n");
	free(pgm->cookie);
}

static int lucax_chip_erase(PROGRAMMER * pgm, AVRPART * p){
	unsigned char ret;
	IMPORT_PDATA(pgm);

	libusb_control_transfer(pdata->dev, USB_DEVICETOHOST, LUCAX_PDI_CHIP_ERASE, 0, 0, &ret, 1, USB_TIME_OUT_MS);

	if(ret != 0)
		return -1;
	return 0;
}

static int lucax_set_sck_period(PROGRAMMER * pgm, double v){
	IMPORT_PDATA(pgm);
	pdata->io_frequency = 1/v/1000; //freq in khz
	return 0;
}

static int lucax_read_byte(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m, unsigned long addr, unsigned char * value){
	IMPORT_PDATA(pgm);
	int address = m->offset + addr;

	if(address >= 0x01000000){
		if(libusb_control_transfer(pdata->dev, USB_DEVICETOHOST, LUCAX_PDI_RAW_READ, address&0xFFFF, (address>>16), value, 1, USB_TIME_OUT_MS)<0){
			avrdude_message(MSG_INFO, "USB error");
			return -1;
		}
	} else {
		unsigned char adr[4];
		adr[0] = address&0xFF;
		adr[1] = (address>>8)&0xFF;
		adr[2] = (address>>16)&0xFF;
		adr[3] = (address>>24)&0xFF;

		if(libusb_control_transfer(pdata->dev, USB_HOSTTODEVICE, LUCAX_PDI_NVM_READ, 0, 1, adr, 4, USB_TIME_OUT_MS)<0){
			avrdude_message(MSG_INFO, "USB error - 1");
			return -1;
		}

		int actual = 0;
		if(libusb_bulk_transfer(pdata->dev, LUCAX_IN, value, 1, &actual, USB_TIME_OUT_MS)<0){
			avrdude_message(MSG_INFO, "USB error - 2");
			return -1;
		}
		if(actual != 1){
			avrdude_message(MSG_INFO, "USB error - transferred %i bytes instead of 1", actual);
			return -1;
		}

	}

	return 0;
}

static int lucax_paged_load(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m, unsigned int page_size, unsigned int baseaddr, unsigned int n_bytes){
	IMPORT_PDATA(pgm);
	int total = 0;

	int address = m->offset + baseaddr;
	unsigned char adr[4];
	adr[0] = address&0xFF;
	adr[1] = (address>>8)&0xFF;
	adr[2] = (address>>16)&0xFF;
	adr[3] = (address>>24)&0xFF;

	if(libusb_control_transfer(pdata->dev, USB_HOSTTODEVICE, LUCAX_PDI_NVM_READ, 0, n_bytes, adr, 4, USB_TIME_OUT_MS)<0){
			avrdude_message(MSG_INFO, "USB error");
			return -1;
		}

	while(total < n_bytes){
			int actual=0;
			int ret = libusb_bulk_transfer(pdata->dev, LUCAX_IN, (m->buf+baseaddr)+total, 64, &actual, USB_TIME_OUT_MS);

			total += actual;
			if(ret < 0)
				return -1;
			if(actual == 0)
				return 0;
	}

	return 0;
}

static int lucax_paged_write(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m,
                                  unsigned int page_size,
                                  unsigned int addr, unsigned int n_bytes)
{
	IMPORT_PDATA(pgm);

	int mem = 0;
	if(strcmp(m->desc, "flash")==0){
		mem = LUCAX_PDI_FLASH;
	} else if(strcmp(m->desc, "eeprom")==0){
		mem = LUCAX_PDI_EEPROM;
	} else {
		avrdude_message(MSG_INFO, "Error: the memory region %s is not supported in page writing!", m->desc);
		return -1;
	}

	int address = m->offset + addr;

	unsigned char adr[4];
	adr[0] = address&0xFF;
	adr[1] = (address>>8)&0xFF;
	adr[2] = (address>>16)&0xFF;
	adr[3] = (address>>24)&0xFF;

	if(libusb_control_transfer(pdata->dev, USB_HOSTTODEVICE, LUCAX_PDI_NVM_WRITE_PAGE, mem, n_bytes, adr, 4, USB_TIME_OUT_MS)<0){
		avrdude_message(MSG_INFO, "USB error");
		return -1;
	}

	int total_page = 0;
	while(total_page<n_bytes){//fill the programmer buffer
		int actual = 0;
		int lenght = ((n_bytes-total_page) > 64) ? 64 : (n_bytes-total_page);
		int ret = libusb_bulk_transfer(pdata->dev, LUCAX_OUT, (m->buf+addr)+total_page, lenght, &actual, USB_TIME_OUT_MS);
		if(ret < 0){
			avrdude_message(MSG_INFO, "USB error writing bulk %i - size %i", ret, lenght);
			return -1;
		}
		total_page += actual;
		if(actual == 0) return 0;
	}


	return 0;
}

static int lucax_write_byte(PROGRAMMER * pgm, AVRPART * p, AVRMEM * m, unsigned long addr, unsigned char value){
	IMPORT_PDATA(pgm);

	int mem_type = 0;
	if(strcmp(m->desc, "lock")==0){
		mem_type = LUCAX_PDI_LOCK;
	} else {
		char c[4];
		strncpy(c, m->desc, 4);
		if(strcmp(c, "fuse")==0)
			mem_type = LUCAX_PDI_FUSE;
		else {
			avrdude_message(MSG_INFO, "Error: this memory is not supported in single byte writing!");
			return -1;
		}
	}

	int address = m->offset + addr;
	unsigned char adr[4];
	adr[0] = address&0xFF;
	adr[1] = (address>>8)&0xFF;
	adr[2] = (address>>16)&0xFF;
	adr[3] = (address>>24)&0xFF;
	if(libusb_control_transfer(pdata->dev, USB_HOSTTODEVICE, LUCAX_PDI_NVM_WRITE_BYTE, mem_type, value, adr, 4, USB_TIME_OUT_MS)<0){
		avrdude_message(MSG_INFO, "USB error");
		return -1;
	}

	return 0;
}

int lucax_get_error(PROGRAMMER * pgm){
	IMPORT_PDATA(pgm);
	unsigned char c;
	libusb_control_transfer(pdata->dev, USB_DEVICETOHOST, LUCAX_REPORT_ERROR, 0, 0, &c, 1, 3000);
	return (int) c;
}


void lucax_dummy_read(PROGRAMMER * pgm){
	IMPORT_PDATA(pgm);
	unsigned char adr[] = {0, 1, 2, 4};
	if(libusb_control_transfer(pdata->dev, USB_HOSTTODEVICE, LUCAX_PDI_NVM_READ, 0, 2, adr, 4, USB_TIME_OUT_MS)<0){
		avrdude_message(MSG_INFO, "USB error");
	}
	int actual;
	libusb_bulk_transfer(pdata->dev, LUCAX_IN, adr, 2, &actual, USB_TIME_OUT_MS);
}
