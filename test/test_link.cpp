/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 *
 * Tests the netlink class.
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
	simulation sim;
	netlink link;
	nethost h1, h2;
	netflow flow;
	packet p1, p2, p3;

	// Note that the link's buffer size is tiny--just 2kb
	linkTest() : link("L1", 5, 10, 2), h1("H1", link), h2("H2", link),
			flow("F1", 1, 20, h1, h2, sim), p1(FLOW, flow, 1),
			p2(FLOW, flow, 2), p3(ACK, flow, 3) {
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
	ASSERT_EQ(2 * 1024, link.getBuflen());
	ASSERT_EQ(2, link.getBuflenKB());
	ASSERT_FLOAT_EQ(5, link.getRateMbps());
	ASSERT_FLOAT_EQ(5 * 1024 * 1024 / 8, link.getRateBytesPerSec());
}

/*
 * Tests that the buffer occupancy is given correctly for well-spaced packets.
 */
TEST_F(linkTest, wellSpacedPacketsBufferTest) {

	// With the current link rate of 5 mbps and link delay of 10 ms the
	// source to destination total transmission time is 11.5625 ms.
	// Note that link buffer is only 2kb (2 packets' worth) in size.

	/*
	ASSERT_FLOAT_EQ(0, link.getWaitTimeIntervalMs());
	ASSERT_EQ(0, link.getBufferOccupancy());
	ASSERT_TRUE(link.sendPacket(p1, true));              // send p1
	ASSERT_EQ(1024, link.getBufferOccupancy());
	ASSERT_FLOAT_EQ(11.5625, link.getWaitTimeIntervalMs());

	ASSERT_FALSE(link.receivedPacket(p2.getId(), true)); // wrong id
	ASSERT_FALSE(link.receivedPacket(p3.getId(), true)); // wrong id
	ASSERT_TRUE(link.receivedPacket(p1.getId(), true));  // correct
	ASSERT_FALSE(link.receivedPacket(p1.getId(), true)); // empty now

	ASSERT_EQ(0, link.getBufferOccupancy());
	ASSERT_FLOAT_EQ(0, link.getWaitTimeIntervalMs());

	ASSERT_TRUE(link.sendPacket(p2, true));              // send p2
	ASSERT_EQ(1024, link.getBufferOccupancy());
	ASSERT_FLOAT_EQ(11.5625, link.getWaitTimeIntervalMs());

	ASSERT_FALSE(link.receivedPacket(p1.getId(), true)); // wrong id
	ASSERT_FALSE(link.receivedPacket(p3.getId(), true)); // wrong id
	ASSERT_TRUE(link.receivedPacket(p2.getId(), true));  // correct
	ASSERT_FALSE(link.receivedPacket(p1.getId(), true)); // empty now & wrong

	ASSERT_EQ(0, link.getBufferOccupancy());
	ASSERT_FLOAT_EQ(0, link.getWaitTimeIntervalMs());
	*/
}

/*
 * Now test that packets buffer correctly when sent quickly (with overflows)
 */
TEST_F(linkTest, sendPacketTest) {

	/*
	ASSERT_FLOAT_EQ(0, link.getWaitTimeIntervalMs());
	ASSERT_EQ(0, link.getBufferOccupancy());
	ASSERT_TRUE(link.sendPacket(p1, true));              // send p1
	ASSERT_EQ(1024, link.getBufferOccupancy());
	ASSERT_FLOAT_EQ(11.5625, link.getWaitTimeIntervalMs());

	ASSERT_TRUE(link.sendPacket(p2, true));              // send p2
	ASSERT_FLOAT_EQ(23.125, link.getWaitTimeIntervalMs());
	ASSERT_EQ(2048, link.getBufferOccupancy());

	ASSERT_FALSE(link.sendPacket(p3, true));              // fail to send p3
	ASSERT_FLOAT_EQ(23.125, link.getWaitTimeIntervalMs());

	ASSERT_FALSE(link.receivedPacket(p2.getId(), true)); // wrong id
	ASSERT_FALSE(link.receivedPacket(p3.getId(), true)); // wrong id
	ASSERT_TRUE(link.receivedPacket(p1.getId(), true));  // correct
	ASSERT_FALSE(link.receivedPacket(p1.getId(), true)); // already dequeued

	ASSERT_EQ(1024, link.getBufferOccupancy());

	ASSERT_FLOAT_EQ(11.5625, link.getWaitTimeIntervalMs());
	ASSERT_TRUE(link.sendPacket(p3, true));              // send p3
	ASSERT_EQ(1088, link.getBufferOccupancy());
	ASSERT_FLOAT_EQ(21.66015625, link.getWaitTimeIntervalMs());

	ASSERT_TRUE(link.sendPacket(p3, true));              // send p3 again
	ASSERT_EQ(1152, link.getBufferOccupancy());
	ASSERT_FLOAT_EQ(31.7578125, link.getWaitTimeIntervalMs());

	ASSERT_TRUE(link.receivedPacket(p2.getId(), true));  // correct
	ASSERT_TRUE(link.receivedPacket(p3.getId(), true));  // correct
	ASSERT_TRUE(link.receivedPacket(p3.getId(), true));  // correct
	ASSERT_FALSE(link.receivedPacket(p1.getId(), true));  // buffer was empty
	*/
}

#endif // TEST_LINK_CPP
