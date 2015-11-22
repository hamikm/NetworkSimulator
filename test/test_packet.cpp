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

	packet p1(FLOW, testflow, 3);
	packet p2(ACK, testflow, 5);

	netrouter r1("R1");
	netrouter r2("R2");
	netrouter r3("R3");
	netlink l12("L12", 5, 10, 64);
	netlink l23("L23", 5, 12.5, 128);
	netlink l31("L31", 5, 10, 64);
	r1.addLink(l12); r1.addLink(l31);
	r2.addLink(l12); r2.addLink(l31);
	r3.addLink(l12); r3.addLink(l31);

	packet p4(ROUTING, r1.getName(), r2.getName());
	packet p5(ROUTING, r2.getName(), r3.getName());
	packet p6(ROUTING, r1.getName(), r3.getName());

	ASSERT_EQ(FLOW, p1.getType());
	ASSERT_EQ(ACK, p2.getType());

	ASSERT_EQ(ROUTING, p4.getType());
	ASSERT_EQ(ROUTING, p5.getType());
	ASSERT_EQ(ROUTING, p6.getType());

	// tests necessary because lots of internal unit changes in packet class
	ASSERT_FLOAT_EQ(1024, p1.getSizeBytes());
	ASSERT_FLOAT_EQ(64, p2.getSizeBytes());
	ASSERT_FLOAT_EQ(64, p4.getSizeBytes());
	ASSERT_FLOAT_EQ(1024, p1.getSizeBytes());
	ASSERT_FLOAT_EQ(64.0 / 1024 / 1024 * 8, p2.getSizeMb());
	ASSERT_FLOAT_EQ(64.0 / 1024 / 1024 * 8, p2.getSizeMb());

	ASSERT_EQ(3, p1.getSeq());
	ASSERT_LT(p2.getSeq(), 0);
	ASSERT_LT(p4.getSeq(), 0);
}

#endif // TEST_PACKET_CPP
