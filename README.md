xsens_mt linux 3.2 driver
========================

This is based on the original driver by xsens, with added sugar to make it work on linux 3.2

Install instructions
--------------------

```
$ make
$ sudo modprobe usbserial
$ sudo insmod ./xsens_mt.ko
```
