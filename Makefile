CC=gcc
RM=rm -rf

CFLAGS=-c -Wall -Wextra -Wpedantic -Werror -Wshadow
LDFLAGS=

SRCDIR=./src
OBJDIR=./obj
BINDIR=./bin
BINNAME=ginec
BINPATH=$(BINDIR)/$(BINNAME)

SOURCES=$(wildcard $(SRCDIR)/*.c)
TEMP=$(SOURCES:.c=.o)
OBJECTS=$(TEMP:$(SRCDIR)%=$(OBJDIR)%)

build: $(BINNAME)

run:
	$(BINPATH)

clear:
	$(RM) $(OBJDIR) $(BINDIR)

install:
	install $(BINPATH) /usr/local/bin

uninstall:
	rm -f /usr/local/bin/$(BINNAME)

$(BINNAME): $(OBJECTS)
	mkdir -p $(BINDIR)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(BINPATH)

$(OBJECTS): $(SOURCES)
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
