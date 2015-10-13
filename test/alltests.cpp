/**
 * @file
 * @author Hamik Mukelyan
 */

#include "gtest/gtest.h"
#include "test_jsonlib.cpp"
#include "test_simulation_input.cpp"
#include "test_simulation.cpp"
#include "netdevice.h"
#include "nethost.h"
#include "netrouter.h"
#include "netlink.h"
#include "netflow.h"

using namespace testing;

/*
 * Runs all the test suites in the @c test directory.
 *
 * @param argc
 * @param argv
 *
 * @return Usually zero :-)
 */
int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
