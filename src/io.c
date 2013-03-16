#include "ds1m12.h"

#define EP_CONTROL (0)
#define EP_BULK_IN (1)
#define EP_BULK_OUT (2)

libusb_device_handle * ds1m12_find_and_open(libusb_device **devs)
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

/*
static void ds1m12_control_in(libusb_device_handle *handle,
		int wIndex, unsigned char expecth, unsigned char expectl)
{
	unsigned char data[2];
	int r;
	
	r = libusb_control_transfer(
			handle,
			LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR,
			144,
			0,
			wIndex,
			data,
			2,
			10
		);
	printf("=> 0xc0, 144, %d\n<= %02x%02x\n", wIndex, data[1], data[0]);
}
*/

void ds1m12_control_out(libusb_device_handle *handle,
		uint8_t request,
		uint16_t index, uint16_t value)
{
	int r;
	r = libusb_control_transfer(
			handle,
			LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | EP_CONTROL,
			request,
			value,
			index,
			NULL,
			0,
			5000
		);
	printf("C => %03d: %05d = %05d (endpoint %d)\n", request, index, value, EP_CONTROL);
	if (r < 0)
		err(1, "control transfer failed: %s", libusb_error_name(r));
}

void ds1m12_bulk_in(libusb_device_handle *handle,
		unsigned char endpoint,
		int expected_length, unsigned char *expected)
{
	unsigned char data[64];
	int rx_data = 0;
	int r;

	r = libusb_bulk_transfer(
			handle,
			endpoint | LIBUSB_ENDPOINT_IN,
			data,
			sizeof(data),
			&rx_data,
			5000
		);
	if (r < 0)
		err(1, "bulk receive failed: %s", libusb_error_name(r));
	fputs("B <= ", stdout);
	for (r = 0; r < rx_data; r++)
		printf("%02x", data[r]);
	printf(" (bulk endpoint %d)\n", EP_BULK_IN);

	if (expected != NULL) {
		for (r = 0; r < expected_length; r++) {
			printf("%02x", expected[r]);
			if (expected[r] != data[r]){ 
				warnx("response != expectation");
			}
		}
		puts("]");
	}
//	if (rx_data != expected_length)
//		warnx("bulk receive: expected %d bytes, got %d", expected_length, rx_data);

}

void ds1m12_bulk_out(libusb_device_handle *handle,
		unsigned char endpoint,
		unsigned char *data, int length)
{
	int tx_data, r;

	r = libusb_bulk_transfer(
			handle,
			endpoint | LIBUSB_ENDPOINT_OUT,
			data,
			length,
			&tx_data,
			5000
	);
	if (r < 0)
		err(1, "bulk transfer failed: %s", libusb_error_name(r));
	if (tx_data != length)
		errx(1, "bulk transfer: expected %d bytes, sent %d", length, tx_data);
	fputs("B => ", stdout);
	for (r = 0; r < length; r++)
		printf("%02x", data[r]);
	printf(" (bulk endpoint %d)\n", EP_BULK_OUT);
}


void ds1m12_setup(libusb_device_handle *handle)
{
#include "iodump"
}
