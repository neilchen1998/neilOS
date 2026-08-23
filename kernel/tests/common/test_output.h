#ifndef KERNEL_TESTS_COMMON_TEST_OUTPUT_H
#define KERNEL_TESTS_COMMON_TEST_OUTPUT_H

#include <stdarg.h>

// @brief Prints a success message in green.
//
// @param format Format string, followed by optional arguments.
void test_pass(const char* format, ...);

// @brief Prints a failure message in red.
//
// @param format Format string, followed by optional arguments.
void test_fail(const char* format, ...);

#endif // KERNEL_TESTS_COMMON_TEST_OUTPUT_H
