#include "ds1m12.h"
#include "options.h"

static libusb_device_handle * ds1m12_find_and_open(libusb_device **devs)
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
			if ((r = libusb_claim_interface(handle, 0)) < 0) {
				err(1, "Failed to claim interface 0: %s", libusb_error_name(r));
			}
			return handle;
		}
	}
	errx(1, "Failed to find USB device -- is the DS1M12 connected?"
			" I'm looking for vendor 0403 and product fac2");
	return NULL;
}

int main(int argc, char **argv)
{
	libusb_device **devs;
	libusb_device_handle *handle;
	int r;
	ssize_t cnt;

	ds1m12_parse_options(argc, argv);

	r = libusb_init(NULL);
	if (r < 0)
		return r;

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0)
		return (int) cnt;

	handle = ds1m12_find_and_open(devs);

	if ((r = libusb_release_interface(handle, 0)) < 0)
		warn("Failed to release interface 0: %s", libusb_error_name(r));
	libusb_free_device_list(devs, 1);

	libusb_close(handle);
	libusb_exit(NULL);
	return 0;
}
