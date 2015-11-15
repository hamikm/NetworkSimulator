/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 *
 * Tests the event (base) class.
 */

#ifndef TEST_EVENT_CPP
#define TEST_EVENT_CPP

#include "gtest/gtest.h"
#include <iostream>
#include <cstdlib>
#include "event.h"
#include <vector>
#include <algorithm>

using namespace std;

/*
 * This is a "test fixture" that sets up things we need in the actual unit
 * tests below. Note that an object of this class is created before
 * each test case begins and is torn down when each test case ends.
 */
class eventTest : public ::testing::Test {
protected:

	event e1, e2, e3, e4;

	eventTest() : e1(0), e2(2.5), e3(2.6), e4(0) {}

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
 * Sorts some events (hopefully on time) with the STL sort function
 * and the comparison functor in the event header, then makes sure
 * the sorting happened as expected.
 */
TEST_F(eventTest, comparatorTest) {

	vector<event> events;
	events.push_back(e3);
	events.push_back(e1);
	events.push_back(e2);
	events.push_back(e4);

	sort(events.begin(), events.end(), eventTimeSorter());

	ASSERT_TRUE(e1.getId() == events[0].getId() ||
			e4.getId() == events[0].getId());
	ASSERT_TRUE(e1.getId() == events[1].getId() ||
			e4.getId() == events[1].getId());
	ASSERT_TRUE(e2.getId() == events[2].getId());
	ASSERT_TRUE(e3.getId() == events[3].getId());

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
