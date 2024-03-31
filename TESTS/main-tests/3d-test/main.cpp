// Angelo Maoudis 14074479
// Angelo Main Unit Testing
#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "utest_default_handlers.h"
#include "utest_specification.h"
#include "utest_types.h"

#include "my3d.h"
Object3D Render3D(-200); // initialize a Render object for this unit test

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

static control_t saveVerticesTest(const size_t callCount){
        // Storing 8 Vertices of a cube in render object.
    Render3D.Vertices.x[0] = -40; Render3D.Vertices.y[0] = -40; Render3D.Vertices.z[0] = 40;  // front bottom left
    Render3D.Vertices.x[1] = 40; Render3D.Vertices.y[1] = -40; Render3D.Vertices.z[1] = 40;   // front bottom right
    Render3D.Vertices.x[2] = 40; Render3D.Vertices.y[2] = 40; Render3D.Vertices.z[2] = 40;    // front top right
    Render3D.Vertices.x[3] = -40; Render3D.Vertices.y[3] = 40; Render3D.Vertices.z[3] = 40;   // front top left
    Render3D.Vertices.x[4] = -40; Render3D.Vertices.y[4] = -40; Render3D.Vertices.z[4] = -40; // back bottom left
    Render3D.Vertices.x[5] = 40; Render3D.Vertices.y[5] = -40; Render3D.Vertices.z[5] = -40;  // back bottom right
    Render3D.Vertices.x[6] = 40; Render3D.Vertices.y[6] = 40; Render3D.Vertices.z[6] = -40;   // back top right
    Render3D.Vertices.x[7] = -40; Render3D.Vertices.y[7] = 40; Render3D.Vertices.z[7] = -40;  // back top left
    Render3D.saveVertices();

        // after saving vertices, does vertices buffer = saved vertices?
    TEST_ASSERT_EQUAL(Render3D.Vertices.x, Render3D.VerticesSAVE.x);
    TEST_ASSERT_EQUAL(Render3D.Vertices.y, Render3D.VerticesSAVE.z);
    TEST_ASSERT_EQUAL(Render3D.Vertices.z, Render3D.VerticesSAVE.z);

    return CaseNext;
}

static control_t restoreVerticesTest(const size_t callCount){
        // add a bunch of garbage into the vertex buffer
    Render3D.Vertices.x[0] = -232; Render3D.Vertices.y[0] = -540; Render3D.Vertices.z[0] = 2;  // front bottom left
    Render3D.Vertices.x[1] = 443; Render3D.Vertices.y[1] = -1420; Render3D.Vertices.z[1] = 5;   // front bottom right
    Render3D.Vertices.x[2] = 560; Render3D.Vertices.y[2] = 780; Render3D.Vertices.z[2] = 1;    // front top right
    Render3D.Vertices.x[3] = 870; Render3D.Vertices.y[3] = 940; Render3D.Vertices.z[3] = 140;   // front top left
    Render3D.Vertices.x[4] = -90; Render3D.Vertices.y[4] = -2; Render3D.Vertices.z[4] = -460; // back bottom left
    Render3D.Vertices.x[5] = 040; Render3D.Vertices.y[5] = -17; Render3D.Vertices.z[5] = -770;  // back bottom right
    Render3D.Vertices.x[6] = 990; Render3D.Vertices.y[6] = 7; Render3D.Vertices.z[6] = 670;   // back top right
    Render3D.Vertices.x[7] = -480; Render3D.Vertices.y[7] = 9; Render3D.Vertices.z[7] = 76;  // back top left
    
    // restore the previous save
    Render3D.restoreSave();

        // after restoring vertices, does vertices buffer = saved vertices?
    TEST_ASSERT_EQUAL(Render3D.Vertices.x, Render3D.VerticesSAVE.x);
    TEST_ASSERT_EQUAL(Render3D.Vertices.y, Render3D.VerticesSAVE.z);
    TEST_ASSERT_EQUAL(Render3D.Vertices.z, Render3D.VerticesSAVE.z);

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
    Case("saveVertices", saveVerticesTest),
    Case("restoreVertices", restoreVerticesTest)
};

Specification specification(greenteaSetup, cases);

// MUST RENAME MAIN FUNCTION IN SOURCE FILE WHEN RUNNING TESTS
// DOES NOT AFFECT BUILDING/RUNNING PROJECT, ONLY BUILDING/RUNNING TESTS
int main(){
    return !Harness::run(specification);
}