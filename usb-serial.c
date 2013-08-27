/*
 * USB Serial Converter driver
 *
 * Copyright (C) 2009 - 2013 Johan Hovold (jhovold@gmail.com)
 * Copyright (C) 1999 - 2012 Greg Kroah-Hartman (greg@kroah.com)
 * Copyright (C) 2000 Peter Berger (pberger@brimson.com)
 * Copyright (C) 2000 Al Borchers (borchers@steinerpoint.com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License version
 *  2 as published by the Free Software Foundation.
 *
 * This driver was originally based on the ACM driver by Armin Fuerst (which was
 * based on a driver by Brad Keryan)
 *
 * See Documentation/usb/usb-serial.txt for more information on using this
 * driver
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/serial.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

#define DRIVER_AUTHOR "Greg Kroah-Hartman <gregkh@linuxfoundation.org>"
#define DRIVER_DESC "USB Serial Driver core"

/**
 * usb_serial_register_drivers - register drivers for a usb-serial module
 * @serial_drivers: NULL-terminated array of pointers to drivers to be registered
 * @name: name of the usb_driver for this set of @serial_drivers
 * @id_table: list of all devices this @serial_drivers set binds to
 *
 * Registers all the drivers in the @serial_drivers array, and dynamically
 * creates a struct usb_driver with the name @name and id_table of @id_table.
 */
int usb_serial_register_drivers(struct usb_serial_driver *const serial_drivers[],
        const char *name,
        const struct usb_device_id *id_table)
{
  int rc;
  struct usb_driver *udriver;
  struct usb_serial_driver * const *sd;

  /*
   * udriver must be registered before any of the serial drivers,
   * because the store_new_id() routine for the serial drivers (in
   * bus.c) probes udriver.
   *
   * Performance hack: We don't want udriver to be probed until
   * the serial drivers are registered, because the probe would
   * simply fail for lack of a matching serial driver.
   * So we leave udriver's id_table set to NULL until we are all set.
   *
   * Suspend/resume support is implemented in the usb-serial core,
   * so fill in the PM-related fields in udriver.
   */
  udriver = kzalloc(sizeof(*udriver), GFP_KERNEL);
  if (!udriver)
    return -ENOMEM;

  udriver->name = name;
  udriver->no_dynamic_id = 1;
  udriver->supports_autosuspend = 1;
  udriver->suspend = usb_serial_suspend;
  udriver->resume = usb_serial_resume;
  udriver->probe = usb_serial_probe;
  udriver->disconnect = usb_serial_disconnect;

  rc = usb_register(udriver);
  if (rc)
    return rc;

  for (sd = serial_drivers; *sd; ++sd) {
    (*sd)->usb_driver = udriver;
    rc = usb_serial_register(*sd);
    if (rc)
      goto failed;
  }

  /* Now set udriver's id_table and look for matches */
  udriver->id_table = id_table;
  rc = driver_attach(&udriver->drvwrap.driver);
  return 0;

 failed:
  while (sd-- > serial_drivers)
    usb_serial_deregister(*sd);
  usb_deregister(udriver);
  return rc;
}
EXPORT_SYMBOL_GPL(usb_serial_register_drivers);

/**
 * usb_serial_deregister_drivers - deregister drivers for a usb-serial module
 * @serial_drivers: NULL-terminated array of pointers to drivers to be deregistered
 *
 * Deregisters all the drivers in the @serial_drivers array and deregisters and
 * frees the struct usb_driver that was created by the call to
 * usb_serial_register_drivers().
 */
void usb_serial_deregister_drivers(struct usb_serial_driver *const serial_drivers[])
{
  struct usb_driver *udriver = (*serial_drivers)->usb_driver;

  for (; *serial_drivers; ++serial_drivers)
    usb_serial_deregister(*serial_drivers);
  usb_deregister(udriver);
  kfree(udriver);
}
EXPORT_SYMBOL_GPL(usb_serial_deregister_drivers);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
