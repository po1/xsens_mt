xsens_mt linux 3.2 driver
========================

This is based on the original driver by xsens, with added sugar to make it work on linux 3.2

Install instructions
--------------------

```
make
sudo modprobe usbserial
sudo insmod ./xsens_mt.ko
```

Device file modes
-----------------

You will not be able to use a device without the proper rights.
By default, only root can access it.

This is easily fixed with the following command:

```
sudo chmod 777 /dev/ttyUSB0
```
