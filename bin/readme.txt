How to use:

rember to pass the configuration file

./avrdude -C ./avrdude.conf -c lucax -m x128a3u

Driver: ./atm32u_driver/Xluca.hex

flash it:

./avrdude -C ./avrdude.conf -c usbasp-clone -m m32u [memory write ./atm32u_driver/Xluca.hex]

