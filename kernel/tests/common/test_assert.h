#ifndef KERNEL_TESTS_COMMON_TEST_ASSERT_H
#define KERNEL_TESTS_COMMON_TEST_ASSERT_H

// @brief Checks a test condition and reports an assertion failure.
//
// @param msg Description of the assertion.
// @param condition Condition that must evaluate to true.
void test_assert(const char* msg, bool condition);

#endif // KERNEL_TESTS_COMMON_TEST_ASSERT_H
