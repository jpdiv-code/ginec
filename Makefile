CC=gcc

RM=rm -rf
INSTALL=install
MKDIR=mkdir -p

CVER=-ansi
COPT=-O2
CMES=-Wall -Wextra -Wpedantic -Wshadow

CFLAGS=$(CVER) $(COPT) $(CMES)

SRCDIR=./src
BINDIR=./bin
BINNAME=ginec
BINPATH=$(BINDIR)/$(BINNAME)
INSTALLPATH=/usr/local/bin

SOURCES=$(wildcard $(SRCDIR)/*.c)

build:
	$(MKDIR) $(BINDIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(BINPATH)

run:
	$(BINPATH)

clear:
	$(RM) $(BINDIR)

install:
	$(INSTALL) $(BINPATH) $(INSTALLPATH)

uninstall:
	$(RM) $(INSTALLPATH)/$(BINNAME)
