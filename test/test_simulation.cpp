/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 *
 * Tests the simulation class.
 *
 * TODO just tests input from JSON file for now; probably want to test a
 * bunch of other stuff too.
 */

#ifndef TEST_SIMULATION_CPP
#define TEST_SIMULATION_CPP

#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <string.h>
#include "simulation.h"

using namespace std;

/*
 * This is a "test fixture" that sets up things we need in the actual unit
 * tests below. Note that an object of this class is created before
 * each test case begins and is torn down when each test case ends.
 */
class simulationTest : public ::testing::Test {
protected:

	virtual void SetUp() { }

	virtual void TearDown() { }
};

/*
 * Checks that input to simulation object from JSON file proceeds as expected.
 * I.e. tests that the Simulation class's constructor works by printing
 * in-memory network devices to cout for visual inspection.
 */
TEST_F(simulationTest, Simulation) {
	const char *input_file = "input_files/test_case_2";
	simulation sim(input_file);
	sim.print_network(cout);
}

#endif // TEST_SIMULATION_CPP
