# Makefile template for shared library
# gcc -Wall -std=gnu99 -shared -fPIC -o pi2driver.so -I/usr/include/lua5.2 -llua5.2 _Raspberry_Pi_2_Driver.c Raspberry_Pi_2/pi_2_dht_read.c common_dht_read.c Raspberry_Pi_2/pi_2_mmio.c

all:
	gcc -Wall -std=gnu99 -shared -fPIC -o wxGPIO.so -I/usr/include/lua5.2 -I/usr/include/python2.7 -llua5.2 c_gpio.c cpuinfo.c event_gpio.c soft_pwm.c common.c constants.c lua_gpio.c
	
clean: 

	rm -f *.so* *.o 