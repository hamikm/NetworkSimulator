/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 *
 * Tests the netflow class.
 */

#ifndef TEST_FLOW_CPP
#define TEST_FLOW_CPP

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
class flowTest : public ::testing::Test {
protected:

	netlink link;
	nethost h1, h2;
	netflow flow;

	// Note that the link's buffer size is tiny--just 2kb
	flowTest() : link("L1", 5, 10, 2), h1("H1", link), h2("H2", link),
			flow("F1", 1, 20, h1, h2, NULL) {
		link.setEndpoint1(h1);
		link.setEndpoint2(h2);
	}

	virtual void SetUp() { }

	virtual void TearDown() { }
};

/*
 * Just tests that construction happens correctly and that unit conversions
 * don't mess anything up.
 */
TEST_F(flowTest, z) {

}

/*
 *
 */
TEST_F(flowTest, y) {

}

/*
 *
 */
TEST_F(flowTest, x) {

}

#endif // TEST_FLOW_CPP
