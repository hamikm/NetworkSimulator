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
 * Tests the <, >, <=, >= operator overloads.
 */
TEST_F(eventTest, comparatorOverloadsTest) {

	// Test <
	ASSERT_TRUE(e1 < e2);
	ASSERT_TRUE(e1 < e3);
	ASSERT_TRUE(e2 < e3);
	ASSERT_FALSE(e3 < e1);
	ASSERT_FALSE(e3 < e3);

	// Test <=
	ASSERT_TRUE(e1 <= e2);
	ASSERT_TRUE(e1 <= e3);
	ASSERT_TRUE(e2 <= e3);
	ASSERT_TRUE(e1 <= e1);
	ASSERT_TRUE(e1 <= e4);
	ASSERT_FALSE(e3 <= e1);

	// Test >
	ASSERT_TRUE(e3 > e2);
	ASSERT_TRUE(e3 > e1);
	ASSERT_TRUE(e2 > e1);
	ASSERT_FALSE(e1 > e3);
	ASSERT_FALSE(e3 > e3);

	// Test >=
	ASSERT_TRUE(e3 >= e2);
	ASSERT_TRUE(e3 >= e1);
	ASSERT_TRUE(e2 >= e1);
	ASSERT_TRUE(e2 >= e2);
	ASSERT_TRUE(e2 >= e4);
	ASSERT_FALSE(e1 >= e2);
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
