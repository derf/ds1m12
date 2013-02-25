#include "ds1m12.h"
#include "options.h"

#define EP_CONTROL (0)
#define EP_BULK_IN (1)
#define EP_BULK_OUT (2)

libusb_device_handle * ds1m12_find_and_open(libusb_device **devs);

/*
void ds1m12_control_in(libusb_device_handle *handle,
		int wIndex, unsigned char expecth, unsigned char expectl);
*/

void ds1m12_control_out(libusb_device_handle *handle,
		uint8_t request, uint16_t index, uint16_t value);

void ds1m12_bulk_in(libusb_device_handle *handle,
		int expected_length, unsigned char *expected);

void ds1m12_bulk_out(libusb_device_handle *handle,
		unsigned char *data, int length);

void ds1m12_setup(libusb_device_handle *handle);
