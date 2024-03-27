// Angelo Maoudis 14074479
// Angelo Main Unit Testing
#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "utest_default_handlers.h"
#include "utest_specification.h"
#include "utest_types.h"

// setup according to mbed docs
// os.mbed.com/mbed-os/v6.16/debug-test/unit-testing.html

using namespace utest::v1;

// test case example, does 1 + 1 = 2?
static control_t exampleTest(const size_t callCount){
    //stuff to test goes here
    TEST_ASSERT_EQUAL(2, 1 + 1);

    return CaseNext;
}

utest::v1::status_t greenteaSetup(const size_t numberOfCases){
    // specifiy timeout (i.e 60s) and the host test (using built in default)
    GREENTEA_SETUP(60, "default_auto");

    return greentea_test_setup_handler(numberOfCases);
}

// list all the test cases in this file here
Case cases[] = {
    Case("1 + 1", exampleTest)
};

Specification specification(greenteaSetup, cases);

int main(){
    return !Harness::run(specification);
}