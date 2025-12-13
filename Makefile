CC = gcc

STANDARD_FLAGS = -std=c99
OPTIMIZATION_FLAGS = -O3 -ffast-math -funroll-loops -fstrict-aliasing -falign-functions
WARNING_FLAGS = -Wall -Wextra -Wpedantic -Werror -Wconversion -Wshadow

INCLUDE_FLAGS = -Iinclude
LD_FLAGS =

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS
    RAYLIB_PREFIX := $(shell brew --prefix raylib)
    INCLUDE_FLAGS += -I$(RAYLIB_PREFIX)/include
    LDFLAGS += -L$(RAYLIB_PREFIX)/lib \
        -lraylib \
        -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
else
    # linux
    LDFLAGS += -lraylib -lm -lpthread -ldl -lrt -lX11
endif

CFLAGS = $(STANDARD_FLAGS) $(OPTIMIZATION_FLAGS) $(WARNING_FLAGS) $(INCLUDE_FLAGS)

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = ginec

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

