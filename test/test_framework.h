#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_RESET "\033[0m"

#define ASSERT(condition, ...)                                                                     \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            fprintf(stderr, COLOR_RED "  ✗ FAILED: " COLOR_RESET);                                 \
            fprintf(stderr, __VA_ARGS__);                                                          \
            fprintf(stderr, "\n    at %s:%d\n", __FILE__, __LINE__);                               \
            return 0;                                                                              \
        }                                                                                          \
    } while (0)

typedef int (*test_func)(void);

typedef struct
{
    const char* name;
    test_func func;
} Test;

static void run_test(const char* name, test_func func)
{
    tests_run++;
    printf("Testing: %s\n", name);
    int result = func();
    if (result)
    {
        tests_passed++;
        printf(COLOR_GREEN "  ✓ PASSED" COLOR_RESET "\n\n");
    }
    else
    {
        tests_failed++;
        printf("\n");
    }
}

static void print_summary(void)
{
    printf("========================================\n");
    printf("Test Summary:\n");
    printf("  Total:  %d\n", tests_run);
    printf(COLOR_GREEN "  Passed: %d\n" COLOR_RESET, tests_passed);
    if (tests_failed > 0)
    {
        printf(COLOR_RED "  Failed: %d\n" COLOR_RESET, tests_failed);
    }
    else
    {
        printf("  Failed: 0\n");
    }
    printf("========================================\n");
}

#endif // TEST_FRAMEWORK_H
