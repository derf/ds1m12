PACKAGE ?= ds1m12
VERSION ?= ${shell git describe --always --dirty}

# Prefix for all installed files
PREFIX ?= /usr/local

# Directories for manuals, executables, docs, data, etc.
main_dir = ${DESTDIR}${PREFIX}
man_dir = ${main_dir}/share/man
bin_dir = ${main_dir}/bin
doc_dir = ${main_dir}/share/doc/feh
image_dir = ${main_dir}/share/feh/images
font_dir = ${main_dir}/share/feh/fonts
example_dir = ${main_dir}/share/doc/feh/examples
desktop_dir = ${main_dir}/share/applications

# default CFLAGS
CFLAGS ?= -ggdb -O2
CFLAGS += -Wall -Wextra -pedantic

CFLAGS += -DPREFIX=\"${PREFIX}\" \
	-DPACKAGE=\"${PACKAGE}\" -DVERSION=\"${VERSION}\"

LDLIBS += -lusb-1.0
