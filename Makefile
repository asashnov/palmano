CC = m68k-palmos-gcc
CFLAGS = -O2 -Wall

SOURCES = *.c palmano.rcp
HEADERS = *.h
DIST_COMMON =  README AUTHORS COPYING ChangeLog INSTALL \
Makefile NEWS TODO
EXTRA_DIST = dis_doc

DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

all: palmano.prc

palmano.prc: palmano bin.stamp
	build-prc palmano.prc "Palmano" pmnS palmano *.bin

bin.stamp: palmano.rcp resource.h
	pilrc palmano.rcp
	touch bin.stamp

palmano: palmano.o midi_keys.o main_form.o editor_form.o option_form.o midi_util.o util.o smf_util.o notes.o
palmano.o: palmano.c resource.h midi_keys.h
midi_keys.o: midi_keys.c midi_keys.h
midi_util.o: midi_util.c midi_util.h
util.o: util.c util.h
main_form.o: main_form.c main_form.h
editor_form.o: editor_form.c editor_form.h
option_form.o: option_form.c option_form.h
smf_util.o: smf_util.c smf_util.h
notes.o: notes.h notes.c

clean:
	-rm -f *.o palmano *.bin *.stamp *.[pg]rc *~

install:
	pilot-xfer -i palmano.prc

dist:
	rm -f palmano*.tgz
	tar -czf palmano-`date '+%Y%m%d'`.tgz --exclude *~  $(DISTFILES)

tags:
	etags *.c *.h

