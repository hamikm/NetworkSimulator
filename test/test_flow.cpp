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
	simulation sim;
	netlink link;
	nethost h1, h2;
	netflow flow;
	packet a1, a2, a3;

	// Note that the link's buffer size is tiny--just 2kb
	flowTest() : link("L1", 5, 10, 2), h1("H1", link), h2("H2", link),
			flow("F1", 1, 20, h1, h2, sim), a1(ACK, flow, 2),
			a2(ACK, flow, 3), a3(ACK, flow, 4) {
		link.setEndpoint1(h1);
		link.setEndpoint2(h2);
	}

	virtual void SetUp() { }

	virtual void TearDown() { }
};

/*
 * Just tests that everything was constructed as expected.
 */
TEST_F(flowTest, constructorTest) {

	ASSERT_STREQ(flow.getSource()->getName().c_str(), "H1");
	ASSERT_STREQ(flow.getDestination()->getName().c_str(), "H2");

	ASSERT_EQ(1, flow.getLastAck());
	ASSERT_STREQ(flow.getName().c_str(), "F1");
	ASSERT_EQ(2560, flow.getNumTotalPackets());
	ASSERT_FLOAT_EQ(20, flow.getSizeMb());
	ASSERT_FLOAT_EQ(1000, flow.getStartTimeMs());
	ASSERT_FLOAT_EQ(1, flow.getStartTimeSec());
	ASSERT_FLOAT_EQ(netflow::DEFAULT_INITIAL_TIMEOUT,
			flow.getTimeoutLengthMs());
}

/*
 * Tests that the netflow class knows which packets should be sent for various
 * windows.
 */
TEST_F(flowTest, peekOutstandingPacketsTest) {
	vector<packet> pkts = flow.peekOutstandingPackets();

	ASSERT_EQ(1, pkts.size());
	ASSERT_EQ(1, pkts[0].getSeq());
	ASSERT_STREQ("H1", pkts[0].getSource().c_str());
	ASSERT_STREQ("H2", pkts[0].getDestination().c_str());
	ASSERT_EQ(1024, pkts[0].getSizeBytes());
	ASSERT_STREQ("F1", pkts[0].getParentFlow()->getName().c_str());
}

/*
 * Tests that popOutstandingPackets returns the expected packets and
 * corresponding timeouts. Also checks that the highest sent packet
 * number was updated, that the RTTs were updated, and that the flow itself
 * loggged the timeout_event.
 */
TEST_F(flowTest, popOutstandingPacketsTest) {

	vector<packet> pkts;
	vector<timeout_event> tevents;
	flow.popOutstandingPackets(1000, pkts, tevents);

	// Test the corresponding timeout events
	ASSERT_EQ(1, pkts.size());
	ASSERT_EQ(1, tevents.size());
	ASSERT_FLOAT_EQ(
			2000, tevents[0].getTime()); // flow starts @1, timeout starts @ 1
	ASSERT_EQ(1, flow.getHighestSentSeqnum()); // highest sent num changed
	ASSERT_FLOAT_EQ(-1000, flow.getRoundTripTimes().at(1)); // sent time logged
	ASSERT_FLOAT_EQ(2000, flow.getFutureTimeoutsEvents().at(1).getTime());
}
/*
 * Sends some fake ACKs to the receiveAck function to see if the window size
 * and start are updated correctly.
 */
TEST_F(flowTest, receiveAckWindowResizingAndShiftingTest) {

	ASSERT_FLOAT_EQ(1, flow.getWindowSize());
	ASSERT_EQ(1, flow.getWindowStart());
	ASSERT_EQ(1, flow.getHighestAckSeqnum());
	ASSERT_EQ(-1, flow.getLinGrowthWinsizeThreshold());

	vector<packet> pkts;
	vector<timeout_event> tevents;
	flow.popOutstandingPackets(1000, pkts, tevents);

	// If no buffering time this is round trip time
	double rtt_pkt1 = 2 * link.getTransmissionTimeMs(a1);

	flow.receivedAck(a1, 1000 + rtt_pkt1);

	// TODO test size of window, change in start index
}

/*
 * Sends some fake ACKs to the receiveAck function to see if round-trip times
 * are logged correctly.
 */
TEST_F(flowTest, receiveAckRTTCalcTest) {

}

/*
 * Sends some fake duplicate ACKs to the receiveAck function to see if
 * duplicates are handled correctly.
 */
TEST_F(flowTest, receiveAckDuplicatesTest) {

}

/*
 * Sends some fake duplicate ACKs to the receiveAck function to see if
 * duplicates are handled correctly.
 */
TEST_F(flowTest, receiveAckTimeoutAdjustmentTest) {

}

#endif // TEST_FLOW_CPP
