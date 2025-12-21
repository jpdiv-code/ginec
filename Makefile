CC = gcc

STANDARD_FLAGS = -std=c99
OPTIMIZATION_FLAGS = -O3 -ffast-math -funroll-loops -fstrict-aliasing -falign-functions
WARNING_FLAGS = -Wall -Wextra -Wpedantic -Werror -Wconversion -Wshadow

INCLUDE_FLAGS = -Iinclude
LDFLAGS =

UNAME_S := $(shell uname -s)

SDL2_CFLAGS  := $(shell sdl2-config --cflags)
SDL2_LDFLAGS := $(shell sdl2-config --libs)

CFLAGS = $(STANDARD_FLAGS) $(OPTIMIZATION_FLAGS) $(WARNING_FLAGS) $(INCLUDE_FLAGS) $(SDL2_CFLAGS)

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = ginec

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS) $(SDL2_LDFLAGS)

%.o: %
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

fmt:
	clang-format -i $(SRCS) $(wildcard include/*.h)

hooks:
	git config core.hooksPath hooks

