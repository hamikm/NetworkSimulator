/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 *
 * Tests the event (base) class.
 */

#ifndef TEST_EVENT_CPP
#define TEST_EVENT_CPP

// Standard includes.
#include "gtest/gtest.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>

using namespace std;

/*
 * @deprecated
 * This used to test my event comparator, but we dropped it because we
 * decided to use a map sorted on events times instead of a priority queue.
 * Since the map didn't need a comparator we dropped EventTimeSorter.
 *
 * This is a "test fixture" that sets up things we need in the actual unit
 * tests below. Note that an object of this class is created before
 * each test case begins and is torn down when each test case ends.
 */
class eventTest : public ::testing::Test {
protected:

	simulation sim;
	event e1, e2, e3, e4;

	eventTest() : e1(0, sim), e2(2.5, sim), e3(2.6, sim), e4(0, sim) {}

	virtual void SetUp() { }

	virtual void TearDown() { }
};

/*
 * Just tests that static ID number generation happens as expected.
 */
TEST_F(eventTest, idGeneratorTest) {

	ASSERT_EQ(1, e1.getId());
	ASSERT_EQ(2, e2.getId());
	ASSERT_EQ(3, e3.getId());
}

/*
 * Tests that basic printing happens correctly. Just prints to cout.
 */
TEST_F(eventTest, printTest) {
	cout << endl << "Printing some base event objects. Note that the event ID "
			"numbers are offset from 1 because other unit tests precede"
			" this one." << endl;
	cout << e1 << endl << e2 << endl << e3 << endl << endl;
}

#endif // TEST_EVENT_CPP
