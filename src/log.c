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

static void c_channel(enum ftdi_interface n) {
	int ret = ftdi_set_interface(ftdi, n);
	check_ret("set interface", ret);
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

	c_channel(INTERFACE_A);

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

	ret = ftdi_set_baudrate(ftdi, 3000000);
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

	ftdi->eeprom_size *= 2;

	printf("btw, reading eeprom (%d bytes)\n", ftdi->eeprom_size);
	unsigned char *watrom = malloc(ftdi->eeprom_size);
	ftdi_read_eeprom(ftdi, watrom);
	struct ftdi_eeprom *watdec = malloc(sizeof(struct ftdi_eeprom));
	ftdi_eeprom_decode(watdec, watrom, ftdi->eeprom_size);

	for (int i = 0; i < ftdi->eeprom_size / 2; i++) {
		printf("[%02x] %02x%02x %c%c\n", i, watrom[2*i+1], watrom[2*i], isalpha(watrom[2*i+1]) ? watrom[2*i+1] : '.', isalpha(watrom[2*i]) ? watrom[2*i] : '.');
	}

	printf("ven %04x pro %04x manuf \"%s\" prod \"%s\"\n", watdec->vendor_id, watdec->product_id, watdec->manufacturer, watdec->product);

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
	int kfd = open("DDL1M12.rbf", O_RDONLY);
	if (!kfd) {
		perror("open firmware file");
	}
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
	
	c_channel(INTERFACE_B);

	c_ftdi_write_data(ftdi, UCHAR("\x00\x1c\x1c"), 3);

	ret = ftdi_usb_reset(ftdi); check_ret("reset", ret);

	int interval_us = 3;
	int ch_a = 0;
	int ch_b = 1;

	unsigned char watbuff[6] = { 0x02, 0, 0, 0x08, 0, 0 };
	interval_us--;
	watbuff[1] = interval_us & 0xff;
	watbuff[2] = (interval_us >> 8) & 0xff;
	watbuff[4] = (interval_us >> 16) & 0xff;
	watbuff[5] = (interval_us >> 24) & 0xff;
	c_ftdi_write_data(ftdi, watbuff, 6); /* 2182ff */

	c_ftdi_write_data(ftdi, UCHAR("\x03\x0a\x0a"), 3);
	c_ftdi_write_data(ftdi, UCHAR("\x00\x03\x03"), 3); /* coupling */
		/* the second byte controls the leds. the third byte controls what data is actually logged */
	if (ch_a && ch_b) {
		c_ftdi_write_data(ftdi, UCHAR("\x00\x04\x7c"), 3);
	} else if (ch_a && !ch_b) {
		c_ftdi_write_data(ftdi, UCHAR("\x00\x0c\x3c"), 3);
	} else if (!ch_a && ch_b) {
		c_ftdi_write_data(ftdi, UCHAR("\x00\x14\x5c"), 3);
	} else {
		printf("wat\n");
		exit(1);
	}


	if (ch_a && ch_b) {
		int i = 0;
		int d1 = 0, d2 = 0;
		while (1) {
			while (ftdi_read_data(ftdi, inbuf, 4) > 0) {
				// fprintf(stderr, "%02x%02x %02x%02x\n", inbuf[1], inbuf[0],  inbuf[3], inbuf[2]);
				if (inbuf[1] & 0x40) {
					if (1 || !(d1 & 0xff)) {
						fprintf(stderr, "DROP A %d (%d, %f%%)\n", i, d1, d1 * 100. / i);
					}
					d1++;
				}
				if (inbuf[3] & 0x40) {
					if (1 || !(d2 & 0xff)) {
						fprintf(stderr, "DROP B %d (%d, %f%%)\n", i, d2, d2 * 100. / i);
					}
					d2++;
				}
				i++;
			}
		}
	} else {
		int i = 0;
		int d = 0;
		while (1) {
			while (ftdi_read_data(ftdi, inbuf, 2) > 0) {
				// fprintf(stderr, "%02x%02x\n", inbuf[1], inbuf[0]);
				if (inbuf[1] & 0x40) {
					if (1 || !(d & 0xff)) {
						fprintf(stderr, "DROP %d (%d, %f%%)\n", i, d, d * 100. / i);
					}
					d++;
				}
				i++;
			}
		}
	}

	ret = ftdi_usb_close(ftdi);
	check_ret("close", ret);

	ftdi_free(ftdi);

	exit(0);
}

