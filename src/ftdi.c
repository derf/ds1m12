#include <ftdi.h>
#include <stdlib.h>
#include <stdio.h>
#include <usb.h>


int main(int count, char *argv[])
{
	unsigned char inbuf[64];
	int ret;

	struct ftdi_context *ftdi;

	unsigned char echobuf = '\xaa';

	if ((ftdi = ftdi_new()) == 0) {
		puts("ftdi_new failed");
		exit(1);
	}

	if (ftdi_usb_open(ftdi, 0x0403, 0xfac2) < 0) {
		printf("error opening\n");
		exit(1);
	}

	printf("Typ: %d\n", ftdi->type);
	printf("usb_read_timeout: %d\n", ftdi->usb_read_timeout);
	printf("usb_write_timeout: %d\n", 
		ftdi->usb_write_timeout);
	printf("baudrate: %d\n",  ftdi->baudrate);
	printf("bitbang_enabled: %x\n", ftdi->bitbang_enabled);
	printf("bitbang_mode: %x\n", ftdi->bitbang_mode);

	if (ftdi_set_latency_timer(ftdi, 0x0a) < 0) {
		puts("set latency: ");
		puts(ftdi_get_error_string(ftdi));
	}

	if (ftdi_set_baudrate(ftdi, 1000000) < 0) {
		puts("set baudrate: ");
		puts(ftdi_get_error_string(ftdi));
	}

	if ((ret = ftdi_set_bitmode(ftdi, 0, 2)) < 0) {
		puts("set bitmode: ");
		puts(ftdi_get_error_string(ftdi));
	}

	printf("Typ: %d\n", ftdi->type);
	printf("usb_read_timeout: %d\n", ftdi->usb_read_timeout);
	printf("usb_write_timeout: %d\n", 
		ftdi->usb_write_timeout);
	printf("baudrate: %d\n",  ftdi->baudrate);
	printf("bitbang_enabled: %x\n", ftdi->bitbang_enabled);
	printf("bitbang_mode: %x\n", ftdi->bitbang_mode);

	if ((ret = ftdi_write_data(ftdi, &echobuf, 1)) < 0) {
		puts("write data: ");
		puts(ftdi_get_error_string(ftdi));
	}

	if ((ret = ftdi_write_data(ftdi, &echobuf, 1)) < 0) {
		puts("write data: ");
		puts(ftdi_get_error_string(ftdi));
	}

	if ((ret = ftdi_read_data(ftdi, inbuf, 2)) != 2) {
		puts("read data: ");
		puts(ftdi_get_error_string(ftdi));
	}

	printf("< %02x%02x\n", inbuf[0], inbuf[1]);

	if ((ret = ftdi_usb_close(ftdi)) < 0) {
		fprintf(stderr, "usb_close: %d (%s)\n", ret,
				ftdi_get_error_string(ftdi));
	}

	ftdi_free(ftdi);

	exit(0);
}

