#include "ds1m12.h"
#include <getopt.h>

static void show_version(void);

void ds1m12_parse_options(int argc, char **argv)
{
	int optch = 0, cmdx = 0;
	static char stropts[] = "v";
	static struct option lopts[] = {
		{"version", 0, 0, 'v'},
		{0,0,0,0},
	};

	while ((optch = getopt_long(argc, argv, stropts, lopts, &cmdx)) != EOF) {
		switch (optch) {
		case 'v':
			show_version();
			break;
		}
	}
}

static void show_version(void)
{
	puts(PACKAGE " version " VERSION);
	exit(0);
}
