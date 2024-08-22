PWD := $(shell pwd)
KERNEL := /lib/modules/$(shell uname -r)/build

obj-m += charDrvs.o

charDrvs.o := charDrvs.c charDrvs.h

all:
	make -C $(KERNEL) M=$(PWD) modules

clean:
	make -C $(KERNEL) M=$(PWD) clean