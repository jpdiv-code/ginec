CC = gcc
STANDARD_FLAGS = -std=c99
OPTIMIZATION_FLAGS = -O3 -ffast-math -funroll-loops -fstrict-aliasing -falign-functions
WARNING_FLAGS = -Wall -Wextra -Wpedantic -Werror -Wconversion -Wshadow

INCLUDE_FLAGS = -Iinclude
LDFLAGS =

SDL2_CFLAGS  := $(shell sdl2-config --cflags)
SDL2_LDFLAGS := $(shell sdl2-config --libs)

BASE_CFLAGS = $(STANDARD_FLAGS) $(OPTIMIZATION_FLAGS) $(WARNING_FLAGS) $(INCLUDE_FLAGS) $(SDL2_CFLAGS)

# Build directories for different configurations
BUILD_DIR_RELEASE = build/release
BUILD_DIR_DEBUG   = build/debug
BUILD_DIR_TEST    = build/test

# Source files for different builds
RELEASE_SRCS = src/main.c src/vm.c src/host_common.c src/host_normal.c
RELEASE_OBJS = $(patsubst src/%.c,$(BUILD_DIR_RELEASE)/%.o,$(RELEASE_SRCS))
RELEASE_TARGET = ginec

DEBUG_SRCS = src/main.c src/vm.c src/host_common.c src/host_debug.c
DEBUG_OBJS = $(patsubst src/%.c,$(BUILD_DIR_DEBUG)/%.o,$(DEBUG_SRCS))
DEBUG_TARGET = ginec_debug

TEST_VM_SRCS = src/vm.c
TEST_VM_OBJS = $(patsubst src/%.c,$(BUILD_DIR_TEST)/%.o,$(TEST_VM_SRCS))
TEST_SRCS = $(wildcard test/test_*.c)
TEST_BINS = $(TEST_SRCS:.c=)

.PHONY: all fmt test clean_release clean_debug clean_examples clean_test clean build_release build_debug run_release run_debug example/% hooks

all: build_release build_debug

fmt:
	clang-format -i $(RELEASE_SRCS) $(DEBUG_SRCS) $(wildcard include/*.h)

test/test_%: test/test_%.c $(TEST_VM_OBJS)
	$(CC) $(BASE_CFLAGS) $< $(TEST_VM_OBJS) -o $@

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

# Create build directories (order-only prerequisites)
$(BUILD_DIR_RELEASE) $(BUILD_DIR_DEBUG) $(BUILD_DIR_TEST):
	mkdir -p $@

# Compilation rules for release build
$(BUILD_DIR_RELEASE)/%.o: src/%.c | $(BUILD_DIR_RELEASE)
	$(CC) $(BASE_CFLAGS) -c $< -o $@

# Compilation rules for debug build
$(BUILD_DIR_DEBUG)/%.o: src/%.c | $(BUILD_DIR_DEBUG)
	$(CC) $(BASE_CFLAGS) -DDEBUG_MODE -g -Ilib -c $< -o $@

# Compilation rules for test build
$(BUILD_DIR_TEST)/%.o: src/%.c | $(BUILD_DIR_TEST)
	$(CC) $(BASE_CFLAGS) -c $< -o $@

# Build targets
build_release: $(RELEASE_OBJS)
	$(CC) $(BASE_CFLAGS) $(RELEASE_OBJS) -o $(RELEASE_TARGET) $(LDFLAGS) $(SDL2_LDFLAGS)

build_debug: $(DEBUG_OBJS)
	$(CC) $(BASE_CFLAGS) $(DEBUG_OBJS) -o $(DEBUG_TARGET) $(LDFLAGS) -lpthread $(SDL2_LDFLAGS)

# Clean targets
clean_release:
	rm -rf $(BUILD_DIR_RELEASE) $(RELEASE_TARGET)

clean_debug:
	rm -rf $(BUILD_DIR_DEBUG) $(DEBUG_TARGET)

clean_examples:
	rm -f game.roma game.romb

clean_test:
	rm -rf $(BUILD_DIR_TEST) $(TEST_BINS)

clean: clean_release clean_debug clean_examples clean_test
	rm -rf build

run_release: build_release
	./$(RELEASE_TARGET)

run_debug: build_debug
	./$(DEBUG_TARGET)

example/%: 
	cd asm && python gac.py examples/$*.asm ../game.roma ../game.romb

hooks:
	git config core.hooksPath hooks

