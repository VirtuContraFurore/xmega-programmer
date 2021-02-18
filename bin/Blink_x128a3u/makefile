
avrdude_dir = ../


main.elf: main.c
	avr-gcc main.c -o main.elf -Wall -Os -mmcu=atxmega128a3u


main.hex: main.elf
	avr-objcopy -O ihex main.elf main.hex

all: main.hex

clean:
	rm -f main.elf
	rm -f main.hex

flash: main.hex
	$(avrdude_dir)/avrdude -C $(avrdude_dir)/avrdude.conf -c lucax -p x128a3u -U flash:w:./main.hex:i


.PHONY: main.elf hex all clean
