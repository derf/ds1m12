#include <ftdi.h>
#include <stdlib.h>
#include <stdio.h>
#include <usb.h>

#define UCHAR(str) ( (unsigned char *) (str) )

struct ftdi_context *ftdi;

static void check_ret(char *desc, int ret)
{
	if (ret < 0)
		fprintf(stderr, "%s: %d (%s)\n", desc, ret,
				ftdi_get_error_string(ftdi));
	else if (ret > 0)
		fprintf(stdout, "%s ok: %d\n", desc, ret);
}

static void dump_buf(unsigned char *buf, int len)
{
	for (int i = 0; i < len; i++) {
		fprintf(stdout, "%02x", buf[i]);
	}
	putc('\n', stdout);
}

int main(int count, char *argv[])
{
	unsigned char inbuf[64];
	int ret;

	if ((ftdi = ftdi_new()) == 0) {
		puts("ftdi_new failed");
		exit(1);
	}

	ret = ftdi_usb_open(ftdi, 0x0403, 0xfac2);
	check_ret("open", ret);

	printf("Typ: %d\n", ftdi->type);
	printf("usb_read_timeout: %d\n", ftdi->usb_read_timeout);
	printf("usb_write_timeout: %d\n", 
		ftdi->usb_write_timeout);
	printf("baudrate: %d\n",  ftdi->baudrate);
	printf("bitbang_enabled: %x\n", ftdi->bitbang_enabled);
	printf("bitbang_mode: %x\n", ftdi->bitbang_mode);

	ret = ftdi_set_latency_timer(ftdi, 0x0a);
	check_ret("latency", ret);

	ret = ftdi_set_baudrate(ftdi, 1000000);
	check_ret("baudrate", ret);

	ret = ftdi_set_bitmode(ftdi, 0, 2);
	check_ret("bitmode", ret);

	ret = ftdi_usb_reset(ftdi);
	check_ret("reset", ret);

	printf("Typ: %d\n", ftdi->type);
	printf("usb_read_timeout: %d\n", ftdi->usb_read_timeout);
	printf("usb_write_timeout: %d\n", 
		ftdi->usb_write_timeout);
	printf("baudrate: %d\n",  ftdi->baudrate);
	printf("bitbang_enabled: %x\n", ftdi->bitbang_enabled);
	printf("bitbang_mode: %x\n", ftdi->bitbang_mode);

	ret = ftdi_write_data(ftdi, UCHAR("\xaa"), 1);
	check_ret("write 0xaa", ret);

	ret = ftdi_write_data(ftdi, UCHAR("\xaa"), 1);
	check_ret("write 0xaa", ret);

	ret = ftdi_read_data(ftdi, inbuf, 2);
	check_ret("read", ret);
	dump_buf(inbuf, 2);

	ret = ftdi_write_data(ftdi, UCHAR("\xab"), 1);
	check_ret("write 0xab", ret);

	ret = ftdi_read_data(ftdi, inbuf, 4);
	check_ret("read", ret);
	dump_buf(inbuf, 4);

	ret = ftdi_write_data(ftdi, UCHAR("\x81\x87"), 2);
	check_ret("write", ret);

	ret = ftdi_read_data(ftdi, inbuf, 1);
	check_ret("read", ret);
	dump_buf(inbuf, 1);

	for (int i = 0; i < 26; i++) {
		ret = ftdi_write_data(ftdi, UCHAR("\xaa\xaa\xaa"), 3);
		check_ret("write", ret);
	}

	ret = ftdi_read_data(ftdi, inbuf, 0x48);
	check_ret("read", ret);
	dump_buf(inbuf, 0x48);

	ret = ftdi_write_data(ftdi, UCHAR("\xab"), 1);
	check_ret("write 0xab", ret);

	ret = ftdi_read_data(ftdi, inbuf, 7);
	check_ret("read", ret);
	dump_buf(inbuf, 7);

	ret = ftdi_write_data(ftdi, UCHAR("\x00\x8f\x9f"), 3);
	check_ret("write", ret);

	ret = ftdi_set_latency_timer(ftdi, 0x01);
	check_ret("latency", ret);

	ret = ftdi_set_latency_timer(ftdi, 0x0a);
	check_ret("latency", ret);

	ret = ftdi_set_bitmode(ftdi, 0, 2);
	check_ret("bitmode", ret);

	ret = ftdi_usb_reset(ftdi);
	check_ret("reset", ret);

	printf("Typ: %d\n", ftdi->type);
	printf("usb_read_timeout: %d\n", ftdi->usb_read_timeout);
	printf("usb_write_timeout: %d\n", 
		ftdi->usb_write_timeout);
	printf("baudrate: %d\n",  ftdi->baudrate);
	printf("bitbang_enabled: %x\n", ftdi->bitbang_enabled);
	printf("bitbang_mode: %x\n", ftdi->bitbang_mode);

	ret = ftdi_write_data(ftdi, UCHAR("\xaa"), 1);
	check_ret("write 0xaa", ret);

	ret = ftdi_write_data(ftdi, UCHAR("\xaa"), 1);
	check_ret("write 0xaa", ret);

	ret = ftdi_write_data(ftdi, UCHAR("\xaa"), 1);
	check_ret("write 0xaa", ret);

	ret = ftdi_write_data(ftdi, UCHAR("\xaa"), 1);
	check_ret("write 0xaa", ret);

	ret = ftdi_write_data(ftdi, UCHAR("\xaa"), 1);
	check_ret("write 0xaa", ret);

	ret = ftdi_read_data(ftdi, inbuf, 6);
	check_ret("read", ret);
	dump_buf(inbuf, 6);

	ret = ftdi_write_data(ftdi, UCHAR("\xab"), 1);
	check_ret("write 0xab", ret);

	ret = ftdi_read_data(ftdi, inbuf, 6);
	check_ret("read", ret);
	dump_buf(inbuf, 6);

	ret = ftdi_usb_close(ftdi);
	check_ret("close", ret);

	ftdi_free(ftdi);

	exit(0);
}

