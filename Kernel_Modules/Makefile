PWD := $(shell pwd)
KERNEL := /lib/modules/$(shell uname -r)/build

obj-m += dummyModule.o

dummyModule.o := dummyModule.c dummyModule.h

all:
	make -C $(KERNEL) M=$(PWD) modules

clean:
	make -C $(KERNEL) M=$(PWD) clean