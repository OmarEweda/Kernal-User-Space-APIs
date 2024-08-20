PWD := $(shell pwd)
KERNEL := /lib/modules/$(shell uname -r)/build

obj-m += dummyModule.o

EXTRA_CFLAGS := -I$(PWD)

all:
	make -C $(KERNEL) M=$(PWD) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.mod modules.order *.symvers