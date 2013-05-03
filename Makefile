include config.mk

all: build-src

build-src:
	@${MAKE} -C src

install: install-bin

install-bin:
	@echo installing executables to ${bin_dir}
	@mkdir -p ${bin_dir}
	@cp src/ds1m12-log ${bin_dir}
	@chmod 755 ${bin_dir}/ds1m12-log


uninstall:
	rm -f ${bin_dir}/ds1m12-log

dist:
	mkdir /tmp/ds1m12-${VERSION}
	git --work-tree=/tmp/ds1m12-${VERSION} checkout -f
	cp src/deps.mk /tmp/ds1m12-${VERSION}/src/deps.mk
	sed -i 's/^VERSION ?= .*$$/VERSION ?= ${VERSION}/' \
		/tmp/ds1m12-${VERSION}/config.mk
	tar -C /tmp -cjf ../ds1m12-${VERSION}.tar.bz2 ds1m12-${VERSION}
	rm -r /tmp/ds1m12-${VERSION}

clean:
	@${MAKE} -C src clean

.PHONY: all install uninstall clean install-bin dist
