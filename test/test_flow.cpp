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
	packet a1, a2, a3, a4, a5;

	// Note that the link's buffer size is tiny--just 2kb
	flowTest() : link("L1", 5, 10, 2), h1("H1", link), h2("H2", link),
			flow("F1", 1, 20, h1, h2, sim), a1(ACK, flow, 2),
			a2(ACK, flow, 3), a3(ACK, flow, 4), a4(ACK, flow, 5),
			a5(ACK, flow, 6) {
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
	pkts = flow.popOutstandingPackets(1000);
	map<int, timeout_event *> tevents = flow.getFutureTimeoutsEvents();

	ASSERT_EQ(1, pkts.size());
	ASSERT_EQ(1, tevents.size());
	ASSERT_FLOAT_EQ(
			2000, tevents[1]->getTime()); // flow starts @1, timeout starts @ 1
	ASSERT_EQ(1, flow.getHighestSentSeqnum()); // highest sent num changed
	ASSERT_FLOAT_EQ(-1000, flow.getRoundTripTimes().at(1)); // sent time logged
}
/*
 * Sends some fake ACKs to the receiveAck function to see if the window size
 * and start are updated correctly. Also checks if round-trip times are logged
 * correctly.
 */
TEST_F(flowTest, receiveAckWindowResizingAndShiftingTest) {

	ASSERT_FLOAT_EQ(1, flow.getWindowSize());
	ASSERT_EQ(1, flow.getWindowStart());
	ASSERT_EQ(1, flow.getHighestAckSeqnum());
	ASSERT_EQ(-1, flow.getLinGrowthWinsizeThreshold());

	vector<packet> pkts;

	/*
	 * Get the outstanding packets in the current window. Normally we would
	 * send the packets in pkts by making send_packet_events right after this
	 * call, but here we just want to test the flow class.
	 */
	pkts = flow.popOutstandingPackets(1000);

	/*
	 * After the send_packet_events are made, queued, and eventually dequeued,
	 * the send_packet_event will ask its link field about the arrival time
	 * for its packet at the other end of the link. (It will also put the
	 * packet on the link's buffer). The send_packet_event will make a
	 * receive_packet_event with this arrival time and will queue it up.
	 * When the receive_packet_event runs it will either result in its packet
	 * getting forwarded because it arrived at a router (by generating another
	 * send_packet_event), or it will result in a return ACK packet because
	 * it arrived at a host. The return ACK packet will generate a
	 * send_packet_event and another receive_packet_event, whose arrival time
	 * is forecasted by again asking the link through pick the packet is
	 * traveling for end-to-end time plus buffering time. The arrival time of
	 * the ACK minus the send time of the initial packet is the round-trip
	 * time, which I've approximated here for a vacant link buffer by
	 * multiplying the end-to-end transmission time by 2.
	 */

	// If no buffering time this is round trip time
	double rtt_pkt = 2 * link.getTransmissionTimeMs(a1);

	/*
	 * Indicate to the flow that an ACK was received and cancel pending
	 * timeouts in the host as well as the global event queue. Normally the
	 * receive_packet_event would call this function.
	 */
	flow.receivedAck(a1, 1000 + rtt_pkt);

	// Make sure the window size and start changed as expected.
	ASSERT_FLOAT_EQ(2, flow.getWindowSize());
	ASSERT_EQ(2, flow.getWindowStart());

	/*
	 * After the first ACK is received the window gets bumped to size 2 with
	 * start 2, so in the next invocation of popOutstandingPackets packets
	 * 2 and 3 should be sent. Test this here. Recall that processing time
	 * on both ends is zero, so 1000 + rtt_pkt1 is a good approximation for the
	 * time at which the new packets will be sent (assuming zero buffering).
	 */

	vector<packet> pkts2 = flow.popOutstandingPackets(1000 + rtt_pkt);
	map<int, timeout_event *> tevents = flow.getFutureTimeoutsEvents();
	ASSERT_EQ(2, pkts2.size());
	ASSERT_EQ(2, tevents.size()); // first was erased by now, so 2 instead of 3

	// avg_RTT and std_RTT are set to first RTT, so the timeout length after
	// the first round of packet sends is avg + 4 * std = 5 * RTT
	ASSERT_FLOAT_EQ(rtt_pkt + 4 * rtt_pkt, flow.getTimeoutLengthMs());
	ASSERT_FLOAT_EQ((1000 + rtt_pkt) + (rtt_pkt + 4 * rtt_pkt),
			tevents[2]->getTime());
	ASSERT_FLOAT_EQ((1000 + rtt_pkt) +
			(rtt_pkt + 4 * rtt_pkt) + netflow::TIMEOUT_DELTA,
			tevents[3]->getTime());
	ASSERT_EQ(3, flow.getHighestSentSeqnum()); // highest sent num changed

	ASSERT_FLOAT_EQ(-(1000 + rtt_pkt), flow.getRoundTripTimes().at(2));
	ASSERT_FLOAT_EQ(-(1000 + rtt_pkt), flow.getRoundTripTimes().at(3));
}

/*
 * Sends some fake duplicate ACKs to the receiveAck function to see if
 * duplicates are handled correctly.
 */
TEST_F(flowTest, receiveAckDuplicatesTest) {

	// send first packet
	vector<packet> pkts = flow.popOutstandingPackets(1000);

	// round trip time estimate
	double rtt_pkt = 2 * link.getTransmissionTimeMs(a1);

	// process the ACK
	flow.receivedAck(a1, 1000 + rtt_pkt);

	// send 2nd and 3rd packets
	vector<packet> pkts2 = flow.popOutstandingPackets(1000 + rtt_pkt);

	/*
	 * Now we'll pretend that packet 2 is lost so that the ACK for packet
	 * 1 (which contains sequence number 2) registers repeatedly.
	 * Make sure number of duplicate ACKs increased and that the highest
	 * send packet number and window size/start didn't change, so that if
	 * pop outstanding packets function is called NOTHING WILL HAPPEN
	 */
	double earlier_window_size = flow.getWindowSize();
	int earlier_window_start = flow.getWindowStart();

	// the actual time of delivery of the second ACK for packet 1 will
	// depend on the host at the other end...
	flow.receivedAck(a1, 1000 + 2 * rtt_pkt);

	ASSERT_EQ(1, flow.getNumDuplicateAcks());
	ASSERT_FLOAT_EQ(earlier_window_size, flow.getWindowSize());
	ASSERT_EQ(earlier_window_start, flow.getWindowStart());

	/*
	 * Now repeat by processing another duplicate ACK; the duplicate ACK
	 * count should have increased by one but the window size and start
	 * should still be the same.
	 */

	flow.receivedAck(a1, 1000 + 3 * rtt_pkt);

	ASSERT_EQ(2, flow.getNumDuplicateAcks());
	ASSERT_FLOAT_EQ(earlier_window_size, flow.getWindowSize());
	ASSERT_EQ(earlier_window_start, flow.getWindowStart());

	/*
	 * Now process the third ACK. At the third one the window size should
	 * be halved, the linear growth threshold should be set to half of the
	 * window size when the third duplicate ACK was received, and the highest
	 * sent packet number should be dropped to one more than the sequence
	 * number of the duplicated ACK so that the missing one can be
	 * retransmitted.
	 */

	// the actual time of delivery of the second ACK for packet 1 will
	// depend on the host at the other end...
	flow.receivedAck(a1, 1000 + 4 * rtt_pkt);

	ASSERT_EQ(0, flow.getNumDuplicateAcks()); // b/c reset to 0
	ASSERT_EQ(1, flow.getLinGrowthWinsizeThreshold());
	ASSERT_FLOAT_EQ(1, flow.getWindowSize());
	ASSERT_EQ(2, flow.getWindowStart());
}

/*
 * Similar to receiveAckDuplicatesTest but sends and receives several more
 * packets successfully before the duplicate ACK situation arises; this is
 * used to test that the window size/start changes work when the values
 * are larger.
 */
TEST_F(flowTest, receiveAckDuplicatesTest2) {

	// send first packet
	vector<packet> pkts = flow.popOutstandingPackets(1000);
	ASSERT_EQ(1, pkts.size());

	// round trip time estimate
	double rtt_pkt = 2 * link.getTransmissionTimeMs(a1);

	// process the 1st ACK
	flow.receivedAck(a1, 1000 + rtt_pkt);
	ASSERT_EQ(2, flow.getWindowStart());
	ASSERT_FLOAT_EQ(2, flow.getWindowSize());

	// send 2nd and 3rd packets
	vector<packet> pkts2 = flow.popOutstandingPackets(1000 + rtt_pkt);
	ASSERT_EQ(2, pkts2.size());

	// process the 2nd ACK
	flow.receivedAck(a2, 1000 + 2 * rtt_pkt);
	ASSERT_EQ(3, flow.getWindowStart());
	ASSERT_FLOAT_EQ(3, flow.getWindowSize());

	// send the 4th, 5th packets
	vector<packet> pkts3 = flow.popOutstandingPackets(1000 + 2 * rtt_pkt);
	ASSERT_EQ(2, pkts3.size());

	// Assume the 3rd ACK arrives slightly behind the 2nd b/c of buffers
	flow.receivedAck(a3, 1000 + 2 * rtt_pkt + 1);
	ASSERT_EQ(4, flow.getWindowStart());
	ASSERT_FLOAT_EQ(4, flow.getWindowSize());

	// send the 6th and 7th packets
	vector<packet> pkts4 = flow.popOutstandingPackets(1000 + 2 * rtt_pkt + 1);
	ASSERT_EQ(2, pkts4.size());

	// Process 4th ACK
	flow.receivedAck(a4, 1000 + 3 * rtt_pkt);
	ASSERT_EQ(5, flow.getWindowStart());
	ASSERT_FLOAT_EQ(5, flow.getWindowSize());

	// Assume 5th packet was dropped, so process some duplicate 4th ACKs
	double earlier_window_size = flow.getWindowSize();
	int earlier_window_start = flow.getWindowStart();

	flow.receivedAck(a4, 1000 + 4 * rtt_pkt);
	ASSERT_EQ(1, flow.getNumDuplicateAcks());
	ASSERT_FLOAT_EQ(earlier_window_size, flow.getWindowSize());
	ASSERT_EQ(earlier_window_start, flow.getWindowStart());

	flow.receivedAck(a4, 1000 + 5 * rtt_pkt);
	ASSERT_EQ(2, flow.getNumDuplicateAcks());
	ASSERT_FLOAT_EQ(earlier_window_size, flow.getWindowSize());
	ASSERT_EQ(earlier_window_start, flow.getWindowStart());

	flow.receivedAck(a4, 1000 + 6 * rtt_pkt);
	ASSERT_EQ(0, flow.getNumDuplicateAcks()); // b/c reset to 0
	ASSERT_EQ(2.5, flow.getLinGrowthWinsizeThreshold());
	ASSERT_FLOAT_EQ(2.5, flow.getWindowSize());
	ASSERT_EQ(5, flow.getWindowStart());
}

#endif // TEST_FLOW_CPP
