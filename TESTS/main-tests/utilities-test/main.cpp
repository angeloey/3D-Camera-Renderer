// Angelo Maoudis 14074479
// Angelo Main Unit Testing
#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "utest_default_handlers.h"
#include "utest_specification.h"
#include "utest_types.h"

#include "myUtils.h"
Utilities Utils;

// setup according to mbed docs
// os.mbed.com/mbed-os/v6.16/debug-test/unit-testing.html

// see all tests: mbed test --compile-list
// run test with: mbed test -t GCC_ARM -m auto -v -n tests-main-tests-<TEST-NAME>

using namespace utest::v1;

// test case example, does 1 + 1 = 2?
static control_t exampleTest(const size_t callCount){
    //stuff to test goes here
    TEST_ASSERT_EQUAL(2, 1 + 1);

    return CaseNext;
}

static control_t returnPositiveTest(const size_t callCount){
    // test if returnPositive behaves as expected
    TEST_ASSERT_EQUAL(0, Utils.returnPositive(-5));
    TEST_ASSERT_EQUAL(5, Utils.returnPositive(5));
    TEST_ASSERT_EQUAL(0, Utils.returnPositive(0));

    return CaseNext;
}

static control_t argbToHexTest(const size_t callCount){
    // test if argbToHex returns values as expected
    TEST_ASSERT_EQUAL(0x00000000, Utils.argbToHex(0x00, 0x00, 0x00, 0x00));
    TEST_ASSERT_EQUAL(0xFFFFFFFF, Utils.argbToHex(0xFF, 0xFF, 0xFF, 0xFF));
    TEST_ASSERT_EQUAL(0xFF00FF00, Utils.argbToHex(255, 0, 255, 0));

    return CaseNext;
}

static control_t valMapTest(const size_t callCount){
    // test if valMap accurates maps value ranges
    TEST_ASSERT_EQUAL(50, Utils.valMap(5, 0, 10, 0, 100));
    TEST_ASSERT_EQUAL(100, Utils.valMap(10, 1, 10, 0, 100));

    return CaseNext;
}

utest::v1::status_t greenteaSetup(const size_t numberOfCases){
    // specifiy timeout (i.e 60s) and the host test (using built in default)
    GREENTEA_SETUP(60, "default_auto");

    return greentea_test_setup_handler(numberOfCases);
}

// list all the test cases in this file here
Case cases[] = {
    Case("Does 1+1=2", exampleTest),
    Case("argbToHex", argbToHexTest),
    Case("returnPositive", returnPositiveTest),
    Case("valMap", valMapTest)
};

Specification specification(greenteaSetup, cases);

// MUST RENAME MAIN FUNCTION IN SOURCE FILE WHEN RUNNING TESTS
// DOES NOT AFFECT BUILDING/RUNNING PROJECT, ONLY BUILDING/RUNNING TESTS
int main(){
    return !Harness::run(specification);
}