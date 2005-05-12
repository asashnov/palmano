CC = m68k-palmos-gcc
CFLAGS = -g -DDEBUG -O0 -Wall

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
palmano.o: palmano.c resource.h
main_form.o: main_form.c main_form.h
editor_form.o: editor_form.c editor_form.h
option_form.o: option_form.c option_form.h
mkeys.o: mkeys.c mkeys.h
notelist.o: notelist.h notelist.c
utils.o: utils.c utils.h
smfutils.o: smfutils.c smfutils.h

clean:
	-rm -f *.o palmano *.bin *.stamp *.[pg]rc *~

install:
	pilot-xfer -i palmano.prc

dist:
	rm -f palmano*.tgz
	tar -czf palmano-`date '+%Y%m%d'`.tgz --exclude *~  $(DISTFILES)

tags:
	etags *.c *.h

