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

TEST_VM_SRCS = src/vm.c
TEST_VM_OBJS = $(TEST_VM_SRCS:.c=.o)
TEST_SRCS = $(wildcard test/test_*.c)
TEST_BINS = $(TEST_SRCS:.c=)

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS) $(SDL2_LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test/test_%: test/test_%.c $(TEST_VM_OBJS)
	$(CC) $(CFLAGS) $< $(TEST_VM_OBJS) -o $@

test: $(TEST_BINS)
	@echo "\n========================================="
	@echo "Running all tests..."
	@echo "=========================================\n"
	@failed=0; \
	for test_bin in $(TEST_BINS); do \
		./$$test_bin || failed=$$((failed + 1)); \
	done; \
	if [ $$failed -eq 0 ]; then \
		echo "\n$(COLOR_GREEN)All test suites passed!$(COLOR_RESET)"; \
		exit 0; \
	else \
		echo "\n$(COLOR_RED)$$failed test suite(s) failed!$(COLOR_RESET)"; \
		exit 1; \
	fi

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f $(TEST_VM_OBJS) $(TEST_BINS)
	rm -f game.roma game.romb

run: $(TARGET)
	./$(TARGET)

fmt:
	clang-format -i $(SRCS) $(wildcard include/*.h)

asm-01:
	cd asm && python gac.py examples/01.asm ../game.roma ../game.romb

hooks:
	git config core.hooksPath hooks

