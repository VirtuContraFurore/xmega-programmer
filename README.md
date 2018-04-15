# xmega-programmer
AtXmega programmer using an arduino leonardo thanks to https://www.avrfreaks.net/projects/atxmega-programmer-050

## Avrdude 6.3 patch
* the diff file contains the patch to avrdude 6.3
* lucax.c and lucax.h are the usb driver for arduino

## Atmega source code
You need to overwrite the bootloader using an ISP (like a cheap usb-asp)

## Circuit
Is the same shown in the first link, but you have to remove a led from arduino leonardo in order to free the usart clock, then solder to the pcb led track a wire. ATmega32U4 features a clocked usart on PD1 (RX), PD2 (TX) and PD5 (XCK); see atmel's datasheet for details.
The target xmega mcu is connected to arduino's 3.3 voltage reg: be careful.

## Author
Ceragioli Luca -> ceragioli (underscore) luca (at) libero (dot) it

## Acknowledgements
* __avrdude__			https://www.nongnu.org/avrdude/
* __lufa usb library__ 	http://www.fourwalledcubicle.com/LUFA.php
