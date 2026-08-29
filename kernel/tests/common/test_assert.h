#ifndef KERNEL_TESTS_COMMON_TEST_ASSERT_H
#define KERNEL_TESTS_COMMON_TEST_ASSERT_H

#include <stdbool.h>

// @brief Expects a condition to be true.
//
// Reports an assertion failure if the condition evaluates to false.
// @param msg       Description of the expectation.
// @param condition Condition that is expected to be true.
void EXPECT_TRUE(const char* msg, bool condition);

// @brief Expects a condition to be false.
//
// Reports an assertion failure if the condition evaluates to true.
//
// @param msg       Description of the expectation.
// @param condition Condition that is expected to be false.
void EXPECT_FALSE(const char* msg, bool condition);

// @brief Expects two integer values to be equal.
//
// Reports an assertion failure if the two values are not equal.
//
// @param msg  Description of the expectation.
// @param val1 First value to compare.
// @param val2 Second value to compare.
void EXPECT_EQ(const char* msg, int val1, int val2);

// @brief Expects two integer values to be different.
//
// Reports an assertion failure if the two values are equal.
//
// @param msg  Description of the expectation.
// @param val1 First value to compare.
// @param val2 Second value to compare.
void EXPECT_NE(const char* msg, int val1, int val2);

// @brief Expects the first integer value to be less than the second.
//
// Reports an assertion failure if val1 is greater than or equal to val2.
//
// @param msg  Description of the expectation.
// @param val1 First value to compare.
// @param val2 Second value to compare.
void EXPECT_LT(const char* msg, int val1, int val2);

// @brief Expects the first integer value to be less than or equal to the second.
//
// Reports an assertion failure if val1 is greater than val2.
//
// @param msg  Description of the expectation.
// @param val1 First value to compare.
// @param val2 Second value to compare.
void EXPECT_LE(const char* msg, int val1, int val2);

// @brief Expects the first integer value to be greater than the second.
//
// Reports an assertion failure if val1 is less than or equal to val2.
//
// @param msg  Description of the expectation.
// @param val1 First value to compare.
// @param val2 Second value to compare.
void EXPECT_GT(const char* msg, int val1, int val2);

// @brief Expects the first integer value to be greater than or equal to the second.
//
// Reports an assertion failure if val1 is less than val2.
//
// @param msg  Description of the expectation.
// @param val1 First value to compare.
// @param val2 Second value to compare.
void EXPECT_GE(const char* msg, int val1, int val2);

#endif // KERNEL_TESTS_COMMON_TEST_ASSERT_H
