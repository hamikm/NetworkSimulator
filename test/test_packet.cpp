/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 *
 * Tests the packet class.
 */

#ifndef TEST_PACKET_CPP
#define TEST_PACKET_CPP

// Standard includes.
#include "gtest/gtest.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>

using namespace std;

/*
 * This is a "test fixture" that sets up things we need in the actual unit
 * tests below. Note that an object of this class is created before
 * each test case begins and is torn down when each test case ends.
 */
class packetTest : public ::testing::Test {
protected:

	virtual void SetUp() { }

	virtual void TearDown() { }
};

/*
 * Just tests that static ID number generation happens as expected.
 */
TEST_F(packetTest, constructorTest) {
	netlink l1("L1", 5, 10, 64);
	nethost h1("H1", l1);
	nethost h2("H2", l1);
	l1.setEndpoint1(h1);
	l1.setEndpoint1(h2);
	netflow testflow("F1", 1, 20, h1, h2);
	// packet p1(FLOW, "H1", "H2", 3, testflow); TODO finish
}

#endif // TEST_PACKET_CPP
