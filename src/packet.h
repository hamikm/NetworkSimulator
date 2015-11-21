/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef PACKET_H
#define PACKET_H

#include <iostream>
#include <cassert>
#include <string>
#include "nethost.h"
#include "netflow.h"
#include "netelement.h"

using namespace std;

extern bool debug;
extern ostream &debug_os;

/**
 * Describes a packet in the simulated network, which can be one of the types
 * in the enumerated type @c packetType. Note that a real packet would have a
 * payload but this class doesn't provide for one.
 */
class packet : public netelement {

public:

	/** Types of packets in this simulation. */
	enum packet_type {
		FLOW,
		ACK,
		ROUTING
	};

	/** Size of a flow packet in bytes. */
	static const long FLOW_PACKET_SIZE = 1024;

	/** Size of an ACK packet in bytes. */
	static const long ACK_PACKET_SIZE = 64;

	/** Size of a routing packet in bytes. */
	static const long ROUTING_PACKET_SIZE = 64;

	/** A sentinel used for the sequence numbers of routing and ACK packets. */
	static const int NO_FLOW_SEQNUM = -1;

private:

	/**
	 * Type of packet: either payload transmission, acknowledgement, or
	 * routing.
	 */
	packet_type t;

	/**
	 * IP address of source host, which is just the name of the source
	 * host in our simulation.
	 */
	string source_ip;

	/** IP address of destination host, which is similarly just a name. */
	string dest_ip;

	// TODO was told this is necessary but not sure why...
	/** Flow to which this packet belongs. */
	netflow *parent_flow;

	/** Packet size in MB (to be consistent with flow) */
	float size;

	/** Sequence number of packet */
	int seqnum;

public:

	/**
	 * This constructor infers the size of a packet from the given type.
	 * @param type one of the values of the @c packet_type enum.
	 * @param source_ip the NAME (since this simulation uses names, not IPs)
	 * of the original source, which must be a host for ACK or flow packets
	 * but a router for routing packets. Source constraints not enforced by
	 * the constructor.
	 * @param dest_ip the NAME of the ultimate destination, which must be a
	 * host for ACK or flow packets. Destination constraints not enforced
	 * by the constructor.
	 * @param seqnum sequence number for this packet, where the first packet in
	 * flow is numbered 1. Pass in anything (or @c NO_FLOW_SEQNUM) for ACK
	 * or routing packets; their seqnums are just set to @c NO_FLOW_SEQNUM.
	 * @param parent_flow flow to which this packet belongs. ACK and flow
	 * packets belong to flows, but routing packets don't. The latter have
	 * their parent flows set to NULL no matter what is passed in.
	 *
	 */
	packet(packet_type type, string source_ip,
		   string dest_ip, int seqnum, netflow &parent_flow);

	string getSource() const { return source_ip; }

	string getDestination() const { return dest_ip; }

	int getSeq() const { return seqnum; }

	netflow *getParentFlow() const { return parent_flow; }

	bool isAckPacket() const { return t == ACK; }

	bool isFlowPacket() const { return t == FLOW; }

	bool isRoutingPacket() const { return t == ROUTING; }

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const {
		netelement::printHelper(os);
		os << " ---> [packet. src: " << source_ip << ", dst: " << dest_ip <<
				", type: " << t << ", seq_num: " << seqnum << ", size: " <<
				size << "]";
	}
};

#endif // PACKET_H
