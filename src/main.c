/*
 * libusbx example program to list devices on the bus
 * Copyright © 2007 Daniel Drake <dsd@gentoo.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <libusb-1.0/libusb.h>

static libusb_device_handle * find_and_open(libusb_device **devs)
{
	libusb_device *dev;
	libusb_device_handle *handle;
	struct libusb_device_descriptor desc;
	int i = 0, r = 0;

	while ((dev = devs[i++]) != NULL) {
		r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			err(1, "Failed to get device descriptor: %s", libusb_error_name(r));
			return NULL;
		}

		if ((desc.idVendor == 0x0403) && (desc.idProduct == 0xfac2)) {
			if ((r = libusb_open(dev, &handle)) < 0) {
				err(1, "Failed to open USB device: %s", libusb_error_name(r));
			}
			return handle;
		}
	}
	errx(1, "Failed to find USB device -- is the DS1M12 connected?"
			" I'm looking for vendor 0403 and product fac2");
	return NULL;
}

int main(void)
{
	libusb_device **devs;
	libusb_device_handle *handle;
	int r;
	ssize_t cnt;

	r = libusb_init(NULL);
	if (r < 0)
		return r;

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0)
		return (int) cnt;

	handle = find_and_open(devs);

	libusb_free_device_list(devs, 1);

	libusb_close(handle);
	libusb_exit(NULL);
	return 0;
}
