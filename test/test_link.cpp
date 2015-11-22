/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 *
 * Tests the packet class.
 */

#ifndef TEST_LINK_CPP
#define TEST_LINK_CPP

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
class linkTest : public ::testing::Test {
protected:

	netlink link;
	nethost h1, h2;
	netflow flow;
	// packet p1, p2, p3, p4, p5, p6; TODO prob. not necessary

	linkTest() : link("L1", 5, 10, 64), h1("H1", link), h2("H2", link),
			flow("F1", 1, 20, h1, h2) {
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
TEST_F(linkTest, constructionTest) {
	ASSERT_STREQ("H1", flow.getSource()->getName().c_str());
	ASSERT_STREQ("H2", flow.getDestination()->getName().c_str());
	ASSERT_STREQ("H1", link.getEndpoint1()->getName().c_str());
	ASSERT_STREQ("H2", link.getEndpoint2()->getName().c_str());

	// Necessary because there are lots of unit conversions in link class.
	ASSERT_EQ(64 * 1024, link.getBuflen());
	ASSERT_EQ(64, link.getBuflenKB());
	ASSERT_FLOAT_EQ(5, link.getRateMbps());
	ASSERT_FLOAT_EQ(5 * 1024 * 1024 / 8, link.getRateBytesPerSec());
}

/*
 * Tests that the buffer occupancy is given correctly.
 */
TEST_F(linkTest, bufferOccupancyTest) {

}

/*
 * Tests that the arrival time of a packet is given correctly by the
 * sendPacket function.
 */
TEST_F(linkTest, sendPacketTest) {

}

#endif // TEST_LINK_CPP
