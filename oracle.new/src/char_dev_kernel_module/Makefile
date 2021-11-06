KERNELDIR ?= /lib/modules/$(shell uname -r)/build

obj-m := process_cred_oracle.o

PWD := $(shell pwd)

all:

	make -C $(KERNELDIR) M=$(PWD) modules

clean:

	rm *.o *.mod.c *.order *.ko *.markers Module.symvers

