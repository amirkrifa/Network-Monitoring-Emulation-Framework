# $Id: Makefile,v 1.2 2009/01/28 14:42:54 akrifa Exp $

PROGVER=0.9.1

prefix=/usr/local
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
sbindir=${exec_prefix}/sbin
libdir=${exec_prefix}/lib
libexecdir=${exec_prefix}/libexec
datadir=${prefix}/share
mandir=${prefix}/share/man
sysconfdir=${prefix}/etc
srcdir=.
top_srcdir=.
localstatedir=${prefix}/var
includedir=${prefix}/include

PIDFILEDIR=/usr/local/var/run

PATHFLAGS=-DSYSCONFDIR="\"$(sysconfdir)\"" -DPIDFILEDIR="\"$(PIDFILEDIR)\""
PROGFLAGS=-DPROGVER="\"$(PROGVER)\""


CC=gcc
LDFLAGS=
CFLAGS=-g -fPIC -D_GNU_SOURCE
CPPFLAGS=-I$(srcdir)  $(PATHFLAGS) $(PROGFLAGS) -DHAVE_CONFIG_H
LIBS=-lpthread -lm
EXEEXT=
INSTALL=/usr/bin/install -c
RANLIB=ranlib

HEADER_DIR=$(includedir)/flowd

# Bison doesn't work
YACC=/usr/bin/byacc

TARGETS=flowd flowd-reader 
#flowd-controller

all: $(TARGETS)

#FLOWD_CONTROLLER_OBJS=	ClientController.o 

LIBFLOWD_OBJS=		atomicio.o addr.o store.o store-v2.o crc32.o \
			strlcpy.o strlcat.o
LIBFLOWD_HEADERS=	flowd-config.h flowd-common.h addr.h crc32.h \
			store.h store-v2.h flowd-pytypes.h GlobalEstimation.h ClientController.h
FLOWD_OBJS=		flowd.o privsep_fdpass.o privsep.o filter.o \
			GlobalEstimation.o ClientController.o\
			parse.o log.o daemon.o peer.o \
			closefrom.o setproctitle.o
FLOWD_READER_OBJS=	flowd-reader.o parse.o log.o filter.o

libflowd.a: $(LIBFLOWD_HEADERS) $(LIBFLOWD_OBJS)
	$(AR) rv $@ $(LIBFLOWD_OBJS)
	$(RANLIB) $@

parse.c: $(top_srcdir)/parse.y
	$(YACC) $(top_srcdir)/parse.y
	mv y.tab.c parse.c
	rm -f y.tab.h

#flowd-controller: $(FLOWD_CONTROLLER_OBJS)
#	$(CC) -o $@ $(FLOWD_CONTROLLER_OBJS)

flowd: $(LIBFLOWD_HEADERS) $(FLOWD_OBJS) libflowd.a
	$(CC) $(LDFLAGS) -L. -g -o $@ $(FLOWD_OBJS) -lflowd $(LIBS)

flowd-reader: $(LIBFLOWD_HEADERS) $(FLOWD_READER_OBJS) libflowd.a
	$(CC) $(LDFLAGS) -L. -o $@ $(FLOWD_READER_OBJS) libflowd.a $(LIBS)

clean:
	rm -f $(TARGETS) *.o core *.core y.tab.* parse.c libflowd.a

realclean: clean
	-(cd Flowd-perl && test -f Makefile && make distclean)
	rm -rf autom4te.cache Makefile config.log config.status
	rm -f flowd.8 flowd-reader.8 flowd.conf.5
	rm -f *.pyc *.pyo
	rm -rf build

distclean: realclean
	rm -f flowd-config.h* flowd-pytypes.h configure

strip:
	strip $(TARGETS)

install-man:
	$(srcdir)/mkinstalldirs $(DESTDIR)$(mandir)/man5
	$(srcdir)/mkinstalldirs $(DESTDIR)$(mandir)/man8
	$(INSTALL) -m 0644 flowd.8 $(DESTDIR)$(mandir)/man8/flowd.8
	$(INSTALL) -m 0644 flowd.conf.5 $(DESTDIR)$(mandir)/man5/flowd.conf.5
	$(INSTALL) -m 0644 flowd-reader.8 $(DESTDIR)$(mandir)/man8/flowd-reader.8

install-bin: $(TARGETS)
	$(srcdir)/mkinstalldirs $(DESTDIR)$(sbindir)
	$(srcdir)/mkinstalldirs $(DESTDIR)$(bindir)
	$(INSTALL) -m 0755 -s flowd $(DESTDIR)$(sbindir)/flowd
	$(INSTALL) -m 0755 -s flowd-reader $(DESTDIR)$(bindir)/flowd-reader

install-conf: flowd.conf
	$(srcdir)/mkinstalldirs $(DESTDIR)$(sysconfdir)
	test -f $(DESTDIR)$(sysconfdir)/flowd.conf || \
	    $(INSTALL) -m 0600 flowd.conf $(DESTDIR)$(sysconfdir)

install-lib: libflowd.a
	$(srcdir)/mkinstalldirs $(DESTDIR)$(libdir)
	$(INSTALL) -m 0644 libflowd.a $(DESTDIR)$(libdir)

install-includes: $(LIBFLOWD_HEADERS)
	$(srcdir)/mkinstalldirs $(DESTDIR)$(HEADER_DIR)
	$(INSTALL) -m 0644 addr.h $(DESTDIR)$(HEADER_DIR)
	$(INSTALL) -m 0644 store.h $(DESTDIR)$(HEADER_DIR)
	$(INSTALL) -m 0644 store-v2.h $(DESTDIR)$(HEADER_DIR)
	$(INSTALL) -m 0644 crc32.h $(DESTDIR)$(HEADER_DIR)
	$(INSTALL) -m 0644 flowd-pytypes.h $(DESTDIR)$(HEADER_DIR)
	$(INSTALL) -m 0644 flowd-config.h $(DESTDIR)$(HEADER_DIR)
	$(INSTALL) -m 0644 flowd-common.h $(DESTDIR)$(HEADER_DIR)

install: install-bin install-man install-conf install-lib install-includes
