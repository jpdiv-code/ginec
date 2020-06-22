CC=gcc

RM=rm -rf
INSTALL=install
MKDIR=mkdir -p

CVER=-std=c99
COPT=-O2
CMES=-Wall -Wextra -Wpedantic -Wshadow

CFLAGS=$(CVER) $(COPT) $(CMES)
DEBUGCFLAGS=-ggdb $(CVER) $(CMES)
TESTCFLAGS=-ggdb -D TEST -Itest $(CVER) $(CMES)

SRCDIR=./src
TESTRCDIR=./test
BINDIR=./bin
BINNAME=ginec
BINPATH=$(BINDIR)/$(BINNAME)
DEBUGBINPATH=$(BINPATH)-debug
TESTBINPATH=$(BINPATH)-test
INSTALLPATH=/usr/local/bin

SOURCES=$(wildcard $(SRCDIR)/*.c)
TESTSOURCES=$(SOURCES) $(wildcard $(TESTRCDIR)/*.c)

build:
	$(MKDIR) $(BINDIR)
	$(CC) $(SOURCES) $(CFLAGS) -o $(BINPATH)

build-debug:
	$(MKDIR) $(BINDIR)
	$(CC) $(SOURCES) $(DEBUGCFLAGS) -o $(DEBUGBINPATH)

build-test:
	$(MKDIR) $(BINDIR)
	$(CC) $(TESTSOURCES) $(TESTCFLAGS) -o $(TESTBINPATH)

run:
	$(BINPATH)

run-test:
	$(TESTBINPATH)

clear:
	$(RM) $(BINDIR)

install:
	$(INSTALL) $(BINPATH) $(INSTALLPATH)

uninstall:
	$(RM) $(INSTALLPATH)/$(BINNAME)
