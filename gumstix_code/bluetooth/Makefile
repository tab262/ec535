default: rfcomm_listen

rfcomm_listen: rfcomm_listen.c
	arm-linux-gcc -Wall -o rfcomm_listen rfcomm_listen.c

.PHONY: clean

clean:
	rm rfcomm_listen *.o
