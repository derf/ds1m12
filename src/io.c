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
			1000
		);
	printf("C => %03d: %05d = %05d (endpoint %d)\n", request, index, value, EP_CONTROL);
	if (r < 0)
		err(1, "control transfer failed: %s", libusb_error_name(r));
}

void ds1m12_bulk_in(libusb_device_handle *handle,
		int expected_length, unsigned char *expected)
{
	unsigned char data[64];
	int rx_data = 0;
	int r;

	r = libusb_bulk_transfer(
			handle,
			EP_BULK_IN | LIBUSB_ENDPOINT_IN,
			data,
			sizeof(data),
			&rx_data,
			1000
		);
	if (r < 0)
		err(1, "bulk receive failed: %s", libusb_error_name(r));
	fputs("B <= ", stdout);
	for (r = 0; r < rx_data; r++)
		printf("%02x", data[r]);
	printf(" (bulk endpoint %d) [expected ", EP_BULK_IN);

	if (expected != NULL) {
		for (r = 0; r < expected_length; r++) {
			printf("%02x", expected[r]);
			if (expected[r] != data[r]){ 
				errx(1, "response != expectation");
			}
		}
		puts("]");
	}
	if (rx_data != expected_length)
		errx(1, "bulk receive: expected %d bytes, got %d", expected_length, rx_data);

}

void ds1m12_bulk_out(libusb_device_handle *handle,
		unsigned char *data, int length)
{
	int tx_data, r;

	r = libusb_bulk_transfer(
			handle,
			EP_BULK_OUT | LIBUSB_ENDPOINT_OUT,
			data,
			length,
			&tx_data,
			1000
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
	unsigned char s_ok[] = {0x31, 0x60};
	unsigned char s_s1[] = {0x31, 0x60,
		0xfa, 0xaa, 0xfa, 0xaa, 0xfa, 0xaa, 0xfa, 0xaa};
	unsigned char s_s2[] = {0x31, 0x60,
		0xfa, 0xaa, 0xfa, 0xaa, 0xfa, 0xab};
	unsigned char s_s5[] = {0x31, 0x60, 0x6e};
	unsigned char s_s15[] = {0x31, 0x60, 0x6c};
	unsigned char o_aa[] = {0xaa};
	unsigned char o_ab[] = {0xab};
	unsigned char o_s3[] = {0x86, 0x00, 0x00, 0x80, 0x06, 0x87};
	unsigned char o_s4[] = {0x80, 0x06, 0x87, 0x80, 0x02, 0x87, 0x80, 0x06, 0x87};
	unsigned char o_s5[] = {0x81, 0x87};
	unsigned char o_s6[] = {0x19, 0xff, 0x01, 0xff, 0xff, 0x62, 0xff, 0x36,
		0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	unsigned char o_s7[] = {0x30, 0x40, 0x06, 0xc8, 0x00, 0x18, 0x20, 0x03,
		0x60, 0x02, 0x4c, 0x80, 0x09, 0x32, 0x00, 0x06,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char o_s8[] = {0x05, 0xd0, 0x01, 0x3a, 0x40, 0x07, 0x5c, 0x00,
		0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0xe8, 0x00, 0x1d, 0xa0, 0x03, 0x54, 0x80};
	unsigned char o_s9[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char o_s10[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x00, 0x00, 0x10, 0x00, 0x20, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0xe8};
	unsigned char o_s11[] = {0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x30, 0x00,
		0x02, 0x50, 0x00, 0x00, 0x28, 0x04, 0x85, 0x00,
		0x20, 0x00, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00,
		0xa6, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x01};
	unsigned char o_s12[] = {0x02, 0x00, 0x01, 0x00, 0x00, 0x20, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
		0x00, 0x8e, 0xa0, 0x41, 0x04, 0x82, 0x20, 0x50,
		0x42, 0x4a, 0x00, 0x00, 0x2a, 0x11, 0x80, 0x24};
	unsigned char o_s13[] = {0x30, 0x00, 0x40, 0x10, 010, 0x00, 0x80, 0x00,
		0x60, 0x80, 0xb7, 0x00, 0x01, 0x00, 0x08, 0x00,
		0x02, 0x00, 0x00, 0x04, 0x00, 0x40, 0x00, 0x00,
		0x00, 0x40, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char o_s14[] = {0x10, 0x00, 0x00};
	unsigned char o_s15[] = {0x81, 0x87};


	ds1m12_bulk_in(handle, 2, s_ok);
	ds1m12_bulk_in(handle, 2, s_ok);
	ds1m12_control_out(handle, 0, 1, 0);
	ds1m12_control_out(handle, 0, 1, 2);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_bulk_in(handle, 2, s_ok);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 0, 1, 1); // 1158, 1159
	ds1m12_bulk_in(handle, 2, s_ok); // 1149, 1160
	ds1m12_control_out(handle, 0, 1, 2);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 0, 1, 1); // 1172, 1173
	ds1m12_bulk_in(handle, 2, s_ok); // 1163, 1174
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 11, 1, 0x0200); // 1180, 1181
	ds1m12_bulk_out(handle, o_aa, 1);
	ds1m12_bulk_out(handle, o_aa, 1);
	ds1m12_bulk_out(handle, o_aa, 1);
	ds1m12_bulk_out(handle, o_aa, 1); //1188, 1189
	ds1m12_bulk_in(handle, 10, s_s1); // 1177, 1190
	ds1m12_bulk_out(handle, o_aa, 1);
	ds1m12_bulk_out(handle, o_aa, 1);
	ds1m12_bulk_out(handle, o_ab, 1);
	ds1m12_bulk_in(handle, 8, s_s2); // 1194, 1198
	ds1m12_bulk_out(handle, o_s3, 6);
	ds1m12_control_out(handle, 0, 1, 0);
	ds1m12_control_out(handle, 0, 1, 2);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_bulk_in(handle, 2, s_ok); // 1200, 1210
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 0, 1, 1);
	ds1m12_control_out(handle, 3, 1, 0);
	ds1m12_bulk_out(handle, o_s4, 9);
	ds1m12_bulk_out(handle, o_s5, 2);
	ds1m12_bulk_in(handle, 3, s_s5); // 1213, 1226
	ds1m12_bulk_out(handle, o_s6, 32);
	ds1m12_bulk_out(handle, o_s7, 32);
	ds1m12_bulk_out(handle, o_s8, 32); // 1232, 1233
	ds1m12_bulk_out(handle, o_s9, 32);
	ds1m12_bulk_out(handle, o_s10, 32);
	ds1m12_bulk_out(handle, o_s11, 32);
	ds1m12_bulk_out(handle, o_s12, 32);
	ds1m12_bulk_out(handle, o_s13, 32);
	ds1m12_bulk_in(handle, 3, s_s15); // 1228, 1248
	ds1m12_bulk_out(handle, o_s14, 3);
	ds1m12_bulk_out(handle, o_s15, 2);
}
