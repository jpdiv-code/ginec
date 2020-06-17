CC=gcc

RM=rm -rf
INSTALL=install
MKDIR=mkdir -p

CVER=-std=c99
COPT=-O2
CMES=-Wall -Wextra -Wpedantic -Wshadow

CFLAGS=$(CVER) $(COPT) $(CMES)
DEBUGCFLAGS=-ggdb $(CVER) $(CMES)

SRCDIR=./src
BINDIR=./bin
BINNAME=ginec
BINPATH=$(BINDIR)/$(BINNAME)
DEBUGBINPATH=$(BINPATH)-debug
INSTALLPATH=/usr/local/bin

SOURCES=$(wildcard $(SRCDIR)/*.c)

build:
	$(MKDIR) $(BINDIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(BINPATH)

build-debug:
	$(MKDIR) $(BINDIR)
	$(CC) $(DEBUGCFLAGS) $(SOURCES) -o $(DEBUGBINPATH)

run:
	$(BINPATH)

clear:
	$(RM) $(BINDIR)

install:
	$(INSTALL) $(BINPATH) $(INSTALLPATH)

uninstall:
	$(RM) $(INSTALLPATH)/$(BINNAME)
