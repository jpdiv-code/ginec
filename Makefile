CC = gcc
STANDARD_FLAGS = -std=c99
OPTIMIZATION_FLAGS = -O3 -ffast-math -funroll-loops -fstrict-aliasing -falign-functions
WARNING_FLAGS = -Wall -Wextra -Wpedantic -Werror -Wconversion -Wshadow

INCLUDE_FLAGS = -Iinclude
LDFLAGS =

SDL2_CFLAGS  := $(shell sdl2-config --cflags)
SDL2_LDFLAGS := $(shell sdl2-config --libs)

CFLAGS = $(STANDARD_FLAGS) $(OPTIMIZATION_FLAGS) $(WARNING_FLAGS) $(INCLUDE_FLAGS) $(SDL2_CFLAGS)

RELEASE_SRCS = src/main.c src/vm.c src/host_common.c src/host_normal.c
RELEASE_OBJS = $(RELEASE_SRCS:.c=.o)
RELEASE_TARGET = ginec

DEBUG_SRCS = src/main.c src/vm.c src/host_common.c src/host_debug.c
DEBUG_OBJS = $(DEBUG_SRCS:.c=.o)
DEBUG_TARGET = ginec_debug

TEST_VM_SRCS = src/vm.c
TEST_VM_OBJS = $(TEST_VM_SRCS:.c=.o)
TEST_SRCS = $(wildcard test/test_*.c)
TEST_BINS = $(TEST_SRCS:.c=)

.PHONY: all fmt test clean_release clean_debug clean_examples clean_test clean build_release build_debug run_release run_debug example/% hooks

all: build_debug run_debug

fmt:
	clang-format -i $(RELEASE_SRCS) $(DEBUG_SRCS) $(wildcard include/*.h)

test/test_%: test/test_%.c $(TEST_VM_OBJS)
	$(CC) $(CFLAGS) $< $(TEST_VM_OBJS) -o $@

test: $(TEST_BINS)
	@printf "\n========================================\n"
	@printf "Running all tests..."
	@printf "\n=========================================\n"
	@failed=0; \
	for test_bin in $(TEST_BINS); do \
		./$$test_bin || failed=$$((failed + 1)); \
	done; \
	if [ $$failed -eq 0 ]; then \
		printf "\nAll test suites passed!\n"; \
		exit 0; \
	else \
		printf "\n$$failed test suite(s) failed!\n"; \
		exit 1; \
	fi

clean_release:
	rm -f $(RELEASE_OBJS) $(RELEASE_TARGET)

clean_debug:
	rm -f $(DEBUG_OBJS) $(DEBUG_TARGET)

clean_examples:
	rm -f game.roma game.romb

clean_test:
	rm -f $(TEST_VM_OBJS) $(TEST_BINS)

clean: clean_release clean_debug clean_examples clean_test

build_release: clean_release $(RELEASE_OBJS)
	$(CC) $(CFLAGS) $(RELEASE_OBJS) -o $(RELEASE_TARGET) $(LDFLAGS) $(SDL2_LDFLAGS)

build_debug: CFLAGS += -DDEBUG_MODE -g -Ilib
build_debug: LDFLAGS += -lpthread
build_debug: clean_debug $(DEBUG_OBJS)
	$(CC) $(CFLAGS) $(DEBUG_OBJS) -o $(DEBUG_TARGET) $(LDFLAGS) $(SDL2_LDFLAGS)

run_release: build_release
	./$(RELEASE_TARGET)

run_debug: build_debug
	./$(DEBUG_TARGET)

example/%: 
	cd asm && python gac.py examples/$*.asm ../game.roma ../game.romb

hooks:
	git config core.hooksPath hooks

