/*
 * Lucax.c
 *
 *  Created on: Jan 24, 2017
 *      Author: ceragioliluca
 */

#define  INCLUDE_FROM_BULKVENDOR_C
#include "Lucax.h"

unsigned char buffer[MAX_BUFFER];
unsigned char last_op;//used for returning error of operations

int main(void)
{
//	SoftUsart_setup();
//	SoftUsart_print("Hay LUFA is here");
	USB_Init();
	GlobalInterruptEnable();

	for (;;)
	{
		USB_USBTask();
	}
}


void EVENT_USB_Device_ControlRequest(void)
{
//	SoftUsart_send_str("Control request 0x");
//	SoftUsart_print_hex(USB_ControlRequest.bRequest);
//	SoftUsart_send_str("\n\r");

	switch(USB_ControlRequest.bRequest){

	case LUCAX_GET_DESCRIPTOR:
		if(USB_ControlRequest.bmRequestType & REQDIR_DEVICETOHOST)
			lucax_get_descriptor();
		break;
	case LUCAX_REPORT_ERROR:
		if(USB_ControlRequest.bmRequestType & REQDIR_DEVICETOHOST)
			lucax_report_error();
		break;
	case LUCAX_RELEASE_INTERFACE:
		if((USB_ControlRequest.bmRequestType & (1<<7)) == 0)
			lucax_release_interface();
		break;

	case LUCAX_PDI_INIT:
		if(USB_ControlRequest.bmRequestType & REQDIR_DEVICETOHOST)
			lucax_pdi_init();
		break;
	case LUCAX_PDI_CHIP_ERASE:
		if(USB_ControlRequest.bmRequestType & REQDIR_DEVICETOHOST)
			lucax_pdi_chip_erase();
		break;
	case LUCAX_PDI_NVM_READ:
		if((USB_ControlRequest.bmRequestType & (1<<7)) == 0)
			lucax_pdi_nvm_read();
		break;
	case LUCAX_PDI_NVM_WRITE_PAGE:
		if((USB_ControlRequest.bmRequestType & (1<<7)) == 0)
			lucax_pdi_nvm_write_page();
		break;
	case LUCAX_PDI_NVM_WRITE_BYTE:
		if((USB_ControlRequest.bmRequestType & (1<<7)) == 0)
			lucax_pdi_nvm_write_byte();
		break;
	case LUCAX_PDI_RAW_READ:
		if(USB_ControlRequest.bmRequestType & REQDIR_DEVICETOHOST)
			lucax_pdi_raw_read();
		break;
	case LUCAX_PDI_RAW_WRITE:
		if((USB_ControlRequest.bmRequestType & (1<<7)) == 0)
			lucax_pdi_raw_write();
		break;
	}

}

void lucax_get_descriptor(void){
//	SoftUsart_print("* * Get Descriptor");
	Endpoint_ClearSETUP();
	Endpoint_Write_8(0x01); //bit 0: support PDI
	Endpoint_ClearIN();
	Endpoint_ClearStatusStage();
}

void lucax_release_interface(void){
//	SoftUsart_print("* * Release Interface");
	Endpoint_ClearSETUP();
	Endpoint_ClearIN();
	pdi_exit();
	last_op = 0;
}

void lucax_pdi_init(void){
//	SoftUsart_print("* * Pdi init");
	last_op = pdi_init(USB_ControlRequest.wValue);
	lucax_report_error();
}

void lucax_pdi_chip_erase(void){
//	SoftUsart_print("* * Chip erase");
	last_op = nvm_chip_erase();
	lucax_report_error();
}

void lucax_pdi_nvm_read(void){
//	SoftUsart_print("* * NVM read");
	unsigned int count = USB_ControlRequest.wIndex;
	unsigned long address = 0;

	Endpoint_ClearSETUP();

	while(Endpoint_IsOUTReceived() == false);
	address = Endpoint_Read_32_LE();
	Endpoint_ClearOUT();

	Endpoint_ClearStatusStage();

	Endpoint_SelectEndpoint(VENDOR_IN_EPADDR);

	if(count == 1){
		unsigned char data = 0;
		last_op = nvm_read_byte(address, &data);
		Endpoint_Write_8(data);
		Endpoint_ClearIN();
//		SoftUsart_print("short read");
		return;
	}


	if(count > MAX_BUFFER) count = MAX_BUFFER;
	last_op = nvm_read_bytes(address, buffer, count);


	int done = 0;
	while(count > 0){

//		SoftUsart_print("Read - w");
		int packet = (count>VENDOR_IO_EPSIZE_IN) ? VENDOR_IO_EPSIZE_IN : count;

		int time_out=0;
		while(Endpoint_IsINReady()==false){
			if(time_out++ == 30000)
				break;
		}
		Endpoint_Write_Stream_LE(buffer+done, packet, NULL);
		Endpoint_ClearIN();

		count -= packet;
		done += packet;
	}

//	SoftUsart_send_str("Read 0x");
//	SoftUsart_print_hex(done);
//	SoftUsart_send_str(" bytes at 0x");
//	SoftUsart_print_hex(address >> 16);
//	SoftUsart_print_hex(address & 0xFFFF);
//	SoftUsart_print(" absolute");
}


void lucax_pdi_nvm_write_page(void){
//	SoftUsart_print("* * NVM write page");
	unsigned int count = USB_ControlRequest.wIndex;
	unsigned char memory_type = USB_ControlRequest.wValue;
	unsigned long address = 0;

	Endpoint_ClearSETUP();

	while(Endpoint_IsOUTReceived() == false);
	address = Endpoint_Read_32_LE();
	Endpoint_ClearOUT();

	Endpoint_ClearStatusStage();

	//read data from usb and write to pd
	Endpoint_SelectEndpoint(VENDOR_OUT_EPADDR);

	if(count > MAX_BUFFER) count = MAX_BUFFER;

	int done = 0;
	while(count > 0){

//		SoftUsart_print("Write - r");
		int packet = (count>VENDOR_IO_EPSIZE_OUT) ? VENDOR_IO_EPSIZE_OUT : count;

		unsigned int time_out = 0;
		while(Endpoint_IsOUTReceived()==false){
			if(time_out++ == 0xFFFF){
//				SoftUsart_print("Waiting for out timed out - r");
				return;
			}
		}
		Endpoint_Read_Stream_LE(buffer+done, packet, NULL);
		Endpoint_ClearOUT();

		count -= packet;
		done += packet;
	}

	last_op = nvm_paged_write(address, buffer, done, memory_type);

//	SoftUsart_send_str("Write 0x");
//	SoftUsart_print_hex(done);
//	SoftUsart_send_str(" bytes at 0x");
//	SoftUsart_print_hex(address >> 16);
//	SoftUsart_print_hex(address & 0xFFFF);
//	SoftUsart_print(" absolute");
}

void lucax_pdi_nvm_write_byte(void){
//	SoftUsart_print("* * NVM write byte");
	unsigned char data = USB_ControlRequest.wIndex;
	unsigned char memory_type = USB_ControlRequest.wValue;
	unsigned long address = 0;

	Endpoint_ClearSETUP();
	while(Endpoint_IsOUTReceived() == false);
	address = Endpoint_Read_32_LE();
	Endpoint_ClearOUT();

	while(Endpoint_IsINReady() == false);
	Endpoint_ClearIN();

	last_op = nvm_write_byte(address, data, memory_type);

	Endpoint_SelectEndpoint(1);

//	SoftUsart_send_str("Write  bytes at 0x");
//	SoftUsart_print_hex(address >> 16);
//	SoftUsart_print_hex(address & 0xFFFF);
//	SoftUsart_send_str(" value: 0x");
//	SoftUsart_print_hex(data);
//	SoftUsart_print(" absolute");
//	if(memory_type == FUSE)
//		SoftUsart_print("FUSE");
}

void lucax_report_error(void){
//	SoftUsart_print("* * Report error");
	Endpoint_ClearSETUP();
	Endpoint_Write_8(last_op); //send result
	Endpoint_ClearIN();
	Endpoint_ClearStatusStage();
}


void lucax_pdi_raw_read(void){
//	SoftUsart_print("* * Raw read");
	long address = USB_ControlRequest.wIndex;
	address<<=16;
	address |= USB_ControlRequest.wValue;
	Endpoint_ClearSETUP();
	last_op = pdi_read_bytes(address, buffer, USB_ControlRequest.wLength);

	Endpoint_Write_Control_Stream_LE(buffer, USB_ControlRequest.wLength);

	Endpoint_ClearStatusStage();

}

void lucax_pdi_raw_write(void){
//	SoftUsart_print("* * Raw read");
	long address = USB_ControlRequest.wIndex;
	address<<=16;
	address |= USB_ControlRequest.wValue;
	Endpoint_ClearSETUP();

	Endpoint_Read_Control_Stream_LE(buffer, USB_ControlRequest.wLength);

	Endpoint_ClearStatusStage();

	last_op = pdi_write_bytes(address, buffer, USB_ControlRequest.wLength);
}

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs. */
void EVENT_USB_Device_Connect(void)
{
//	SoftUsart_print("\n\nDevice Connect");

}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  the status LEDs.
 */
void EVENT_USB_Device_Disconnect(void)
{
//	SoftUsart_print("\n\nDevice Disconnect");

}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host set the current configuration
 *  of the USB device after enumeration - the device endpoints are configured.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	/* Setup Vendor Data Endpoints */
	ConfigSuccess &= Endpoint_ConfigureEndpoint(VENDOR_IN_EPADDR,  EP_TYPE_BULK, VENDOR_IO_EPSIZE_IN, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(VENDOR_OUT_EPADDR, EP_TYPE_BULK, VENDOR_IO_EPSIZE_OUT, 1);

//	if(ConfigSuccess == true){
//		SoftUsart_print("CONFIG CHANGED: EndPoints are okay");
//	} else
//		SoftUsart_print("EndPoint are not okay");
}
