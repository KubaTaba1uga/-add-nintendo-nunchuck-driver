ifneq ($(KERNELRELEASE),)
obj-m := nunchuk.o
else
KDIR := $(HOME)/Github/linux-6-1-lts/

IDIR := /srv/beaglebone-nfs/

all:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

install:
	sudo cp nunchuk.ko $(IDIR)

endif
