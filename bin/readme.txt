How to use avrdude:

	rember to pass the configuration file

	./avrdude -C ./avrdude.conf -c lucax -m x128a3u


Driver: ./atm32u_driver/Xluca.hex

	flash it:

	./avrdude -C ./avrdude.conf -c usbasp-clone -p m32u4 -U flash:w:./atm32u_driver/Xluca.hex:i
