#include <ftdi.h>
#include <stdlib.h>
#include <stdio.h>
#include <usb.h>
#include <fcntl.h>

#define UCHAR(str) ( (unsigned char *) (str) )

struct ftdi_context *ftdi;

static void check_ret(char *desc, int ret)
{
	if (ret < 0) {
		fprintf(stderr, "%s: %d (%s)\n", desc, ret,
				ftdi_get_error_string(ftdi));
	} else if (ret > 0) {
		fprintf(stdout, "%s ok: %d\n", desc, ret);
	} else {
		fprintf(stdout, "%s 0\n");
	}
}

static void dump_buf(unsigned char *buf, int len)
{
	for (int i = 0; i < len; i++) {
		fprintf(stdout, "%02x", buf[i]);
	}
	putc('\n', stdout);
}

static void c_ftdi_read_data(struct ftdi_context *ftdi, unsigned char *inbuf, int size) {
	int ret = ftdi_read_data(ftdi, inbuf, size);
	fprintf(stderr, "read %d", size);
	if (ret < 0) {
		fprintf(stderr, " error: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
		exit(1);
	} else if (ret == 0) {
		fprintf(stderr, ": no data received\n");

		usleep(1000000);
		ret = ftdi_read_data(ftdi, inbuf, size);
		if (ret == 0) {
			fprintf(stderr, "not even after retry\n");
		} else {
			fprintf(stderr, ", received %d:", ret);
			for (int i = 0; i < ret; i++) {
				fprintf(stderr, " %02x", inbuf[i]);
			}
			fprintf(stderr, "\n");
		}
	} else {
		fprintf(stderr, ", received %d:", ret);
		for (int i = 0; i < ret; i++) {
			fprintf(stderr, " %02x", inbuf[i]);
		}
		fprintf(stderr, "\n");
	}
}

static void c_ftdi_write_data(struct ftdi_context *ftdi, unsigned char *inbuf, int size) {
	fprintf(stderr, "write %d:", size);
	for (int i = 0; i < size; i++) {
		fprintf(stderr, " %02x", inbuf[i]);
	}
	int ret = ftdi_write_data(ftdi, inbuf, size);
	if (ret < 0) {
		fprintf(stderr, ": error %d (%s)\n", ret, ftdi_get_error_string(ftdi));
		exit(1);
	} else if (ret == 0) {
		fprintf(stderr, ": no data sent\n");
	} else {
		fprintf(stderr, ", wrote %d\n", ret);
	}
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
	if (ret != 0) {
		exit(1);
	}

	ret = ftdi_usb_reset(ftdi);
	check_ret("reset", ret);

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

	printf("\n\n");

	for (int i = 0; i < 5; i++) {
		printf("try %d\n", i);
		c_ftdi_write_data(ftdi, UCHAR("\xaa"), 1);
		c_ftdi_read_data(ftdi, inbuf, 2);
	}

	printf("\n");

	c_ftdi_write_data(ftdi, UCHAR("\xab"), 1);

	c_ftdi_read_data(ftdi, inbuf, 2);

	printf("ab foo done.\n\n");

	c_ftdi_write_data(ftdi, UCHAR("\x86\x00\x00\x80\x06\x87"), 6);

	ret = ftdi_usb_reset(ftdi); check_ret("reset", ret);
	ftdi_set_baudrate(ftdi, 0x2dc6c0);

	c_ftdi_write_data(ftdi, UCHAR("\x80\x06\x87\x80\x02\x87\x80\x06\x87"), 9);

	c_ftdi_write_data(ftdi, UCHAR("\x81\x87"), 2);
	c_ftdi_read_data(ftdi, inbuf, 1);

	/* 2097ff, loads some data */
	printf("loading...\n");
	int kfd = open("/tmp/ds1m12-cd/wat/ds1m12/ds1m12a_1k.rbf", O_RDONLY);
	char wat[19895];
	read(kfd, wat, 19895);
	close(kfd);

	int remaining = 19895;
	int pos = 0;
	while (remaining != 0) {
		int now = remaining > 0x200 ? 0x200 : remaining;

		unsigned char nowbuf[0x203];
		nowbuf[0] = 0x19;
		nowbuf[1] = (now-1) & 0xff;
		nowbuf[2] = ((now-1) >> 8) & 0xff;
		memcpy(nowbuf + 3, wat + pos, now);

		printf("writing %d bytes of %d remaining\n", now, remaining);
		ftdi_write_data(ftdi, nowbuf, now + 3);
		pos += now;
		remaining -= now;

		c_ftdi_write_data(ftdi, UCHAR("\x81\x87"), 2);
		c_ftdi_read_data(ftdi, inbuf, 1);
		c_ftdi_write_data(ftdi, UCHAR("\x81\x87"), 2);
		c_ftdi_read_data(ftdi, inbuf, 1);
		c_ftdi_write_data(ftdi, UCHAR("\x81\x87"), 2);
		c_ftdi_read_data(ftdi, inbuf, 1);
	}

	printf("written\n\n");

	c_ftdi_write_data(ftdi, UCHAR("\x81\x87"), 2);
	c_ftdi_read_data(ftdi, inbuf, 1);

	printf("did you see 7e? then it is leuchting!\n");
	printf("enjoy it\n");

	//usleep(3000000);

	c_ftdi_write_data(ftdi, UCHAR("\x19\x01\x00\x06\x06"), 5);

	c_ftdi_write_data(ftdi, UCHAR("\x80\x06\x87\x80\x86\x87\x80\x06\x87"), 9);

	/* mach zu und neu, 3265 .. 3352 */
	

	ret = ftdi_usb_reset(ftdi); check_ret("reset", ret);

	c_ftdi_write_data(ftdi, UCHAR("\x00\x83\x93"), 3); /* 3353 */

	ret = ftdi_usb_reset(ftdi); check_ret("reset", ret);

	c_ftdi_write_data(ftdi, UCHAR("\x00\x8b\x9b"), 3); /* 3435 */

	ret = ftdi_usb_reset(ftdi); check_ret("reset", ret);

	c_ftdi_write_data(ftdi, UCHAR("\x00\x8f\x9f"), 3); /* 3521 */

	ret = ftdi_usb_reset(ftdi); check_ret("reset", ret);

	c_ftdi_write_data(ftdi, UCHAR("\x01\x7e\x7f\x04\x00\x02\x02\x90\x01\x03\x0c\x0f\x00\x8c\x9f\x05\x00\x00"), 0x12); /* 3946 */

	c_ftdi_write_data(ftdi, UCHAR("\x80\x00\x00"), 3);
	while (1) {
		c_ftdi_read_data(ftdi, inbuf, 4);
	}



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

