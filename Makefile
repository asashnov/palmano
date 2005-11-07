CC = m68k-palmos-gcc
CFLAGS = -g -DDEBUG -O0 -Wall
LDFLAGS = -g

SOURCES = *.c palmano.rcp
HEADERS = *.h
DIST_COMMON = README AUTHORS COPYING ChangeLog INSTALL Makefile NEWS TODO
EXTRA_DIST = doc

DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

all: palmano.prc tags

palmano.prc: palmano bin.stamp
	build-prc palmano.prc "Palmano" pmnS palmano *.bin

bin.stamp: palmano.rcp resource.h
	pilrc palmano.rcp
	touch bin.stamp

palmano: palmano.o mkeys.o main_form.o editor_form.o option_form.o utils.o smfutils.o notelist.o
palmano.o:     *.h palmano.c
main_form.o:   *.h main_form.c
editor_form.o: *.h editor_form.c
option_form.o: *.h option_form.c
mkeys.o:       *.h mkeys.c
notelist.o:    *.h notelist.c
utils.o:       *.h utils.c
smfutils.o:    *.h smfutils.c

clean:
	-rm -f *.o palmano *.bin *.stamp *.[pg]rc *~

install:
	pilot-xfer -i palmano.prc

dist:
	rm -f palmano*.tgz
	tar -czf palmano-`date '+%Y%m%d'`.tgz --exclude *~  $(DISTFILES)

tags:
	etags *.c *.h

