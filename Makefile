obj-m+=xsens_mt.o
xsens_mt-objs := usb-serial.o xsens_mt_module.o

KDIR:=/lib/modules/$(shell uname -r)/build
PWD:=$(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
