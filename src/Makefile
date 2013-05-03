include ../config.mk

TARGETS = ${shell echo *.c}
OBJECTS = ${TARGETS:.c=.o}

ds1m12-log: deps.mk ${OBJECTS}
	${CC} ${LDFLAGS} ${CFLAGS} -o $@ -lftdi ${OBJECTS} ${LDLIBS}

include deps.mk

deps.mk: ${TARGETS} ${I_DSTS}
	${CC} ${CPPFLAGS} -MM ${TARGETS} > $@

clean:
	rm -f ds1m12 *.o

.PHONY: clean
