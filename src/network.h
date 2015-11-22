/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef NETWORK_H
#define NETWORK_H

// Standard includes.
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <map>

// Forward declarations.
class netlink;
class netflow;
class netnode;
class nethost;
class netrouter;
class packet;
class timeout_event;
class send_ack_event;

// Custom headers.
#include "util.h"
#include "simulation.h"

using namespace std;

extern bool debug;
extern ostream &debug_os;

// ------------------------------ netelement class ----------------------------

/**
 * Superclass of all network elements including routers, hosts, links, packets,
 * flows. "Device" is a bit of a misnomer for the latter but they all share a
 * superclass so they can share an output operator overload and so they can
 * co-exist in the same STL collection if necessary.
 */
class netelement {

private:

	/** E.g. a router or host name like "H1". */
	string name;

public:

	netelement (string name);

	virtual ~netelement();

	const string &getName() const;

	/**
	 * Print helper function. Derived classes should (partially) override this.
	 * @param os The output stream to which to write netdevice information.
	 */
	virtual void printHelper(std::ostream &os) const;
};

/**
 * Output operator override for printing contents of the given netelement
 * to an output stream. Uses the printHelper function, which is virtual
 * because derived classes will want to modify or enhance printing behavior.
 * @param os The output stream to which to write.
 * @param device The @c netdevice object to write.
 * @return The same output stream for operator chaining.
 */
inline ostream & operator<<(ostream &os, const netelement &device) {
	device.printHelper(os);
	return os;
}

// -------------------------------- netnode class -----------------------------

/**
 * Represents a node, which is either a node or a host, in a simple network.
 */
class netnode : public netelement {

protected:

	/** Pointers to all the links attached to this router. */
	vector<netlink *> links;

public:

	netnode (string name);

	netnode (string name, vector<netlink *> links);

	virtual void addLink (netlink &link);

	const vector<netlink *> &getLinks() const;

	// TODO virtual send packet function

	/** Receive packet method that is overridden by hosts and routers.
	 * Packet type-checking is performed by the host or router. */
	virtual void receivePacket(double time, simulation &sim,
		netflow &flow, packet &pkt) = 0;

	/**
	 * Print helper function. Partially overrides superclass's.
	 * @param os The output stream to which to write netdevice information.
	 */
	virtual void printHelper(std::ostream &os) const;
};

// ------------------------------- nethost class ------------------------------

/**
 * Represents a host in a simple network.
 */
class nethost : public netnode {

private:

public:

	nethost (string name);

	nethost (string name, netlink &link);

	/**
	 * Gets the first (and only, since this is a host) link.
	 * @return link attached to this host
	 */
	netlink *getLink() const;

	/**
	 * Deletes all links then adds the given one.
	 * @param link link to add after deleteing the others
	 */
	void setLink(netlink &link);

	/** 
	 * Receives a packet by checking whether it is a flow or ack packet, and 
	 * then calling the appropriate function. Ignores routing packets.
	 */
	void receivePacket(double time, simulation &sim, netflow &flow, packet &pkt);

	void receiveAckPacket(double time, simulation &sim, netflow &flow, packet &pkt);

	void receiveFlowPacket(double time, simulation &sim, netflow &flow, packet &pkt);

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;
};

// ------------------------------ netrouter class -----------------------------

/**
 * Represents a router in a simple network. TODO add detail to this comment.
 */
class netrouter : public netnode {

private:

	/** Routing table implemented as map from destination names to links. */
	map<string, netlink *> rtable;

public:

	netrouter (string name);

	netrouter (string name, vector<netlink *> links);

	void receivePacket(double time, simulation &sim, netflow &flow, packet &pkt);

	void forwardPacket(double time, simulation &sim, netflow &flow, packet &pkt);

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;
};

// ------------------------------- netflow class ------------------------------

/**
 * Represents a flow in a simple network. TODO add detail to this comment.
 */
class netflow : public netelement {

private:

	/** Start time in seconds from beginning of simulation. */
	double start_time_sec;

	/** Transmission size in megabits. */
	float size_mb;

	/** Pointer to one end of this link. */
	nethost *source;

	/** Pointer to the other end of this link. */
	nethost *destination;

	/**
	 * Just the flow size divided by the size of each packet.
	 */
	int num_total_packets;

	/**
	 * Sequence number of the last ACK received by the source host.
	 */
	int last_received_ack_seqnum;

	/**
	 * The TCP protocol's window size.
	 */
	int window_size;

	/**
	 * Number of duplicate ACKs seen.
	 */
	int num_duplicate_acks;

	/**
	 * This is a map from sequence numbers of packets to their future,
	 * corresponding timeout events. Each time a send_packet_event is pushed
	 * onto the global events queue a corresponding future timeoutout_event is
	 * also pushed. If an ACK for the packet sent through the send_packet_event
	 * is received before that packet's timeout_event has been processed
	 * then it's the receive_ack_event's job to remove the corresponding
	 * timeout_event from the global events queue by looking up the
	 * timeout_event in this map then using the pointer to delete from events.
	 */
	map<int, timeout_event *> future_timeouts_events;

	/**
	 * Every time a send_ack_event is pushed onto the global events queue we
	 * also push another send_ack_event (for a later time) onto the events
	 * queue. The second event is for duplicate acks. If a future
	 * receive_packet_event runs before the duplicate send_ack_event for
	 * the previous packet, the duplicate send_ack_event must be looked
	 * up in this map then deleted from the global events queue.
	 */
	map<int, send_ack_event *> future_send_ack_events;

	/**
	 * This is the maximum length of time in milliseconds that can pass between
	 * a send_packet_event and its corresponding receive_ack_event before
	 * a timeout_event will run. This timeout length is be dynamically
	 * readjusted with the (recursively determined) average RTT and
	 * deviation of RTT.
	 */
	double timeout_length_ms;

	void constructorHelper (double start_time, float size_mb, nethost &source,
			nethost &destination, int num_total_packets,
			int last_received_ack_seqnum, int window_size,
			int num_duplicate_acks, double timeout_length);

public:

	/**
	 * Size of each packet's payload in bytes.
	 */
	static const int PACKET_PAYLOAD_SIZE = 1024;

	/**
	 * The timeout value for the very first packet sent in a flow; the
	 * timeout value is later seeded with the first packet's RTT and then
	 * changed with recursive average and deviation algorithms.
	 */
	static const double DEFAULT_INITIAL_TIMEOUT = 1000.0;

	netflow (string name, float start_time, float size_mb,
			nethost &source, nethost &destination);

	nethost *getDestination() const;

	/** @return start time in seconds from beginning of simulation. */
	float getStartTimeSec() const;

	/**  @return start time in milliseconds from beginning of simulation. */
	float getStartTimeMs() const;

	void setDestination(nethost &destination);

	/** @return size in megabits */
	float getSizeMb() const;

	nethost *getSource() const;

	void setSource(nethost &source);

	int incrDuplicateAcks();

	int getLastAck();

	void updateLastAck(int new_seqnum);

	timeout_event* removeTimeoutEvent(int seq);

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 *
	 * TODO if more instance variables are added might want to add them to this
	 * printer for debugging help.
	 */
	virtual void printHelper(ostream &os) const;
};

// ------------------------------- netlink class ------------------------------

/**
 * Represents a link in a simple network. TODO add detail to this comment.
 */
class netlink : public netelement {

private:

	/** This link's rate in bytes per millisecond. */
	double rate_bpms;

	/** Signal propagation delay for this link in ms. */
	int delay_ms;

	/** Buffer size in bytes. */
	int buflen_bytes;

	/** Pointer to one end of this link. */
	netelement *endpoint1;

	/** Pointer to the other end of this link. */
	netelement *endpoint2;

	/** The link will be free after this (absolute, not relative) time (ms). */
	double available_at_time_ms;

	/**
	 * The number of packets on the buffer. Used with @c available_at_time
	 * to deduce buffer occupancy.
	 */
	int num_packets_waiting;

	/**
	 * Used for error checking: each packet passed to the sendPacket function
	 * must have a time >= this last known time.
	 */
	double last_known_time;

	/**
	 * Helper for the constructors. Converts the buffer length from kilobytes
	 * to bytes and the rate from megabits per second to bytes per second.
	 */
	void constructor_helper(double rate_mbps, int delay, int buflen_kb,
			netelement *endpoint1, netelement *endpoint2);

public:

	/**
	 * @param name of this link
	 * @param rate_mbps link rate in megabits per second
	 * @param delay_ms link delay in milliseconds
	 * @param buflen_kb the size of the only buffer on this link in kilobytes.
	 * This is stored internally in bytes.
	 * @param endpoint1 the host or router on one side of this link
	 * @param endpoint2 the host or router on the other side of this link
	 */
	netlink(string name, double rate_mbps, int delay_ms, int buflen_kb,
			netelement &endpoint1, netelement &endpoint2);

	/**
	 * Sets the endpoints of this link to NULL.
	 * @param name of this link
	 * @param rate_mbps link rate in megabits per second
	 * @param delay_ms link delay in milliseconds
	 * @param buflen_kb the size of the only buffer on this link in kilobytes
     * This is stored internally in bytes.
	 */
	netlink (string name, double rate_mbps, int delay_ms, int buflen_kb);

	/** @return buffer length in bytes. */
	long getBuflen() const;

	/**  @return buffer length in kilobytes. */
	long getBuflenKB() const;

	/** @return delay in milliseconds. */
	int getDelay() const;

	netelement *getEndpoint1() const;

	netelement *getEndpoint2() const;

	/** @return link rate in bytes per second. */
	double getRateBytesPerSec() const;

	/** @return link rate in megabits per second. */
	double getRateMbps() const;

	void setEndpoint1(netelement &endpoint1);

	void setEndpoint2(netelement &endpoint2);

	/**
	 * Returns the buffer occupancy at a particular time.
	 * @param current_time time at which invoking event occurs
	 * @return number of bytes in use in the buffer (assuming a magically
	 * unfragmentable buffer)
	 */
	long getBufferOccupancy(double current_time) const;

	/**
	 * If the link buffer has space the link's future availability time
	 * @c available_at_time is updated and the number of packets on the
	 * buffer, @c num_packets_waiting, is incremented. If the
	 * buffer can't accommodate this packet then neither field is updated,
	 * which models the behavior of packet dropping. Updates @c last_known_time
	 * @param pkt the packet to send
	 * @param current_time the simulation's current time as contained in event
	 * @return the time at which the given packet will be RECEIVED. If the
	 * packet is dropped because the buffer is full returns
	 * @c PKT_DROPPED_SENTINEL, which is a negative number. The caller should
	 * check for a negative return value instead of doing equality comparison
	 * with the sentinel.
	 */
	double sendPacket(const packet &pkt, double current_time);

	/**
	 * Called when a packet is received to free the link for subsequent
	 * packets; just decrements the number of packets on the buffer.
	 * @post @c num_packets_waiting is decremented
	 */
	void receivedPacket();

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;
};

// -------------------------------- packet class ------------------------------

/**
 * Describes a packet in the simulated network, which can be one of the types
 * in the enumerated type @c packetType. Note that a real packet would have a
 * payload but this class doesn't provide for one.
 */
class packet : public netelement {

private:

	/**
	 * Type of packet: either payload transmission, acknowledgement, or
	 * routing.
	 */
	packet_type type;

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

	/** Packet size in megabits (to be consistent with flow) */
	float size;

	/** Sequence number of packet */
	int seqnum;

	/**
	 * Constructor helper. Does naive assignments; logic should be in the
	 * calling constructors.
	 */
	void constructorHelper(packet_type type, const string &source_ip,
				   const string &dest_ip, int seqnum,
				   netflow *parent_flow, float size);

public:

	/**
	 * This constructor infers the size of a packet from the given type, which
	 * must be ROUTING since there's no parent flow and no sequence number.
	 * and no sequence number.
	 * @param type one of the values of the @c packet_type enum, but must
	 * be ROUTING for this constructor
	 * @param source_ip the NAME (since this simulation uses names, not IPs)
	 * of the original source which must be a router
	 * @param dest_ip the NAME of the ultimate destination, which must be a
	 * router
	 * @warning assertion triggered if the packet type isn't ROUTING
	 */
	packet(packet_type type, const string &source_ip, const string &dest_ip);

	/**
	 * This constructor infers the size of a packet from the given type, which
	 * must be ACK or FLOW since the source and destination are inferred from
	 * a parent flow.
	 * @param type one of the values of the @c packet_type enum, but must
	 * be ROUTING for this constructor
	 * @param parent_flow flow to which this packet belongs. ACK and FLOW
	 * packets belong to flows.
	 * @param seqnum sequence number for this packet, where the first packet in
	 * flow is numbered 1. Pass in anything (or @c SEQNUM_FOR_NONFLOWS) for ACK
	 * packets; their seqnums are just set to @c SEQNUM_FOR_NONFLOWS.
	 * @warning assertion is triggered if the packet type is not ACK or FLOW.
	 */
	packet(packet_type type, netflow &parent_flow, int seqnum);

	string getSource() const;

	string getDestination() const;

	int getSeq() const;

	netflow *getParentFlow() const;

	packet_type getType() const;

	/** @return size in megabits. */
	float getSizeMb() const;

	/** @return size in bytes. */
	float getSizeBytes() const;

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;
};

#endif // NETWORK_H
