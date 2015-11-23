/**
 * @file
 * @author Hamik Mukelyan
 */

// Standard includes
#include <iostream>
#include <cstdlib>
#include <string.h>

// Custom headers for use in all test suits
#include "events.h"
#include "network.h"
#include "simulation.h"

// Unit test suites
#include "gtest/gtest.h"
#include "test_jsonlib.cpp"
#include "test_simulation_input.cpp"
#include "test_simulation.cpp"
#include "test_event.cpp"
#include "test_packet.cpp"
#include "test_link.cpp"
#include "test_flow.cpp"

using namespace testing;

bool debug = false;
ostream &debug_os = cout;

/*
 * Runs all the test suites in the @c test directory.
 *
 * @param argc
 * @param argv
 *
 * @return Usually zero :-)
 */
int main(int argc, char **argv) {

	if (argc == 2 && strcmp(argv[1], "-d") == 0) {
		debug = true;
	}

	if (argc == 2 && strcmp(argv[1], "-help") == 0) {
		cerr << "Usage: " << argv[0] <<
				" [-d for debugging] [-help for this message]" << endl;
		exit(1);
	}

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
