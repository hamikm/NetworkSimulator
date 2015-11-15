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

	cout << endl;
	const char *input_file0 = "input_files/test_case_0";
	cout << "Reading the file \"" << input_file0 << "\" into the simulation. "
			<< "Here are the simulation elements after the read: " << endl;
	simulation sim0(input_file0);
	sim0.print_network(cout);
	cout << endl;

	const char *input_file1 = "input_files/test_case_1";
	cout << "Reading the file \"" << input_file1 << "\" into the simulation. "
			<< "Here are the simulation elements after the read: " << endl;
	simulation sim1(input_file1);
	sim1.print_network(cout);
	cout << endl;

	const char *input_file2 = "input_files/test_case_2";
	cout << "Reading the file \"" << input_file2 << "\" into the simulation. "
			<< "Here are the simulation elements after the read: " << endl;
	simulation sim2(input_file2);
	sim2.print_network(cout);
	cout << endl;
}

#endif // TEST_SIMULATION_CPP
