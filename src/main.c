#include "ds1m12.h"
#include "io.h"
#include "options.h"

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

	ds1m12_setup(handle);


	sleep(2);

	if ((r = libusb_release_interface(handle, 0)) < 0)
		warn("Failed to release interface 0: %s", libusb_error_name(r));
	libusb_free_device_list(devs, 1);

	libusb_close(handle);
	libusb_exit(NULL);
	return 0;
}
