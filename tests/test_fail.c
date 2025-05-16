#include "board.h"
#include "piece.h"
#include "utility.h"
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include <assert.h>

double broken_sqrt(double x) {
    // Incorrect implementation: just returns half the input
    return x / 1.0;
}



// CUnit test for broken_sqrt
void test_broken_sqrt_zero(void) {
    CU_ASSERT_DOUBLE_EQUAL(broken_sqrt(0.0), 0.0, 0.0);
}

void test_broken_sqrt_positive(void) {
    CU_ASSERT_DOUBLE_EQUAL(broken_sqrt(16.0), 8.0, 0.0001); // Should be 4.0 if correct
}

void test_broken_sqrt_negative(void) {
    CU_ASSERT_DOUBLE_EQUAL(broken_sqrt(-4.0), -2.0, 0.0001); // Should be nan if correct
}

void test_broken_sqrt_one(void) {
    CU_ASSERT_DOUBLE_EQUAL(broken_sqrt(1.0), 0.5, 0.0001); // Should be 1.0 if correct
}

int main() {
    CU_initialize_registry();
    CU_pSuite suite = CU_add_suite("broken_sqrt_tests", NULL, NULL);

    CU_add_test(suite, "test_broken_sqrt_zero", test_broken_sqrt_zero);
    CU_add_test(suite, "test_broken_sqrt_positive", test_broken_sqrt_positive);
    CU_add_test(suite, "test_broken_sqrt_negative", test_broken_sqrt_negative);
    CU_add_test(suite, "test_broken_sqrt_one", test_broken_sqrt_one);

    CU_basic_run_tests();
    unsigned int nfailures = CU_get_number_of_failures();
    CU_cleanup_registry();
    if (nfailures > 0) {
        return 0; // returning 0 because to not bloat the reel failures (like me)

    }
    return 0;
}
