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
#include <queue>
#include <cmath>

// Custom headers
#include "util.h"

// Forward declarations.
class netevent;
class netlink;
class netflow;
class netnode;
class nethost;
class netrouter;
class packet;
class router_discovery_event;
class start_flow_event;
class send_packet_event;
class receive_packet_event;
class timeout_event;
class ack_event;
class simulation;
class eventTimeSorter;

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

	netelement ();

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

	netnode ();

	netnode (string name);

	netnode (string name, vector<netlink *> links);

	virtual void addLink (netlink &link);

	const vector<netlink *> &getLinks() const;

	/** @return true if this node is capable of routing packets. */
	virtual bool isRoutingNode() const;

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

	bool isRoutingNode() const;

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
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;
};

// ------------------------------ netrouter class -----------------------------

/**
 * Represents a router in a simple network.
 */
class netrouter : public netnode {

private:

	/** Routing table implemented as map from destination names to links. */
	map<string, netlink *> rtable;

public:

	netrouter (string name);

	netrouter (string name, vector<netlink *> links);

	virtual bool isRoutingNode() const;

	/**
	 * If this is an ACK or FLOW packet this function forwards it along the
	 * best link for it to get to its destination as determined by the routing
	 * table; if it's a ROUTING packet then... TODO
	 *
	 * Note that it's the caller's responsibility to generate the actual
	 * send_packet_events based on the returned map.
	 * @param time of packet receipt
	 * @param sim
	 * @param flow parent flow, NULL if ROUTING type
	 * @param pkt the arriving packet
	 * @return a map from the links to use for the corresponding packets
	 */
	map<netlink *, packet> receivePacket(double time, simulation &sim,
			netflow &flow, packet &pkt);

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;
};

// ------------------------------- netflow class ------------------------------

/**
 * Represents a flow in a simple network. Assumes that flow sizes are multiples
 * of 1024 bytes.
 */
class netflow : public netelement {

private:

	/** Start time in seconds from beginning of simulation. */
	double start_time_sec;

	/** Transmission size in megabits. */
	double size_mb;

	/** Number of megabits sent. */
	double amt_sent_mb;

	/** Pointer to one end of this link. */
	nethost *source;

	/** Pointer to the other end of this link. */
	nethost *destination;

	/** Highest received ACK sequence number (at source). */
	int highest_received_ack_seqnum;

	/** Highest sent FLOW packet sequence number (at source). */
	int highest_sent_flow_seqnum;

	/** Highest received FLOW sequence number (at destination). */
	int highest_received_flow_seqnum;

	/** The TCP protocol's window size. */
	double window_size;

	/** Sequence number at which the current transmission window starts. */
	int window_start;

	/** Number of duplicate ACKs seen. */
	int num_duplicate_acks;

	/**
	 * This is the maximum length of time in milliseconds that can pass between
	 * a send_packet_event and its corresponding receive_ack_event before
	 * a timeout_event will run. This timeout length is dynamically
	 * readjusted with the (recursively determined) average RTT and
	 * deviation of RTT.
	 */
	double timeout_length_ms;

	/**
	 * Half of the window size when a timeout happens; remembered so that in
	 * the next window resizing phase exponential growth happens until this
	 * threshold then linear growth happens after. If this is negative then
	 * it hasn't been set due to a timeout.
	 */
	double lin_growth_winsize_threshold;

	/**
	 * Average round-trip time for a packet in this flow. If zero should be
	 * initialized to first RTT.
	 */
	double avg_RTT;

	/**
	 * Standard deviation of round-trip time for a packet in this flow. If 0
	 * should be initialized to first RTT.
	 */
	double std_RTT;

	/**
	 * This is a map from sequence numbers of packets to their future,
	 * corresponding timeout events. Each time a send_packet_event is pushed
	 * onto the global events queue a corresponding future timeoutout_event is
	 * also pushed. If an ACK for the packet sent through the send_packet_event
	 * is received before that packet's timeout_event has been processed
	 * then it's the receive_ack_event's job to remove the corresponding
	 * timeout_event from the global events queue by looking up the
	 * timeout_event in this map then deleting by event id number and time.
	 */
	map<int, timeout_event *> future_timeouts_events;

	/**
	 * Every time an ACK send_packet_event is pushed onto the global events
	 * queue we also push a duplicate_ack_event onto the events
	 * queue. When the duplicate_ack_event runs it chains (queues) another
	 * duplicate_ack_event onto this map and onto the gloval events queue.
	 * If the correct FLOW packet is received in the future then pending
	 * duplicate_ack_events are deleted from this map and from the global
	 * events queue.
	 */
	map<int, ack_event *> future_send_ack_events;

	/**
	 * Map from sequence numbers to round-trip times of those packets. If a
	 * value is negative then it's the FLOW packet departure time; the
	 * corresponding ACK hasn't arrived yet. When it does its arrival
	 * time will be added to the negative number.
	 */
	map<int, double> rtts;

	/** Pointer to simulation so timeout_events can be made in this class. */
	simulation *sim;

	void constructorHelper (double start_time, double size_mb,
			nethost &source, nethost &destination, int num_total_packets,
			double window_size, double timeout_length_ms, simulation &sim);

public:

	/** Size of a flow packet in bytes. */
	static const long FLOW_PACKET_SIZE = 1024;

	/** Size of an ACK packet in bytes. */
	static const long ACK_PACKET_SIZE = 64;

	/** Size of a routing packet in bytes. */
	static const long ROUTING_PACKET_SIZE = 64;

	/** Number of duplicate ACKs before fast retransmit is used. */
	static const int FAST_RETRANSMIT_DUPLICATE_ACK_THRESHOLD = 3;

	/**
	 * The timeout value for the very first packet sent in a flow; the
	 * timeout value is later seeded with the first packet's RTT and then
	 * changed with recursive average and deviation algorithms.
	 */
	static const double DEFAULT_INITIAL_TIMEOUT = 1000.0;

	/** The constant 'b' from the recursive average and std formulas. */
	static const double B_TIMEOUT_CALC = 0.1;

	/**
	 * Interval between execution times of consecutive packets' timeouts (if
	 * the packets are sent at the same time.
	 */
	static const double TIMEOUT_DELTA = 0.0000000001;

	/**
	 * Initializes the flow's basic attributes as well as attributes required
	 * to perform TCP Reno transmissions, like window size, last ACK seen,
	 * number of duplicate ACKs, etc.
	 */
	netflow (string name, double start_time, double size_mb,
			nethost &source, nethost &destination, simulation &sim);

	// --------------------------- Accessors ----------------------------------

	/** @return start time in seconds from beginning of simulation. */
	double getStartTimeSec() const;

	/**  @return start time in milliseconds from beginning of simulation. */
	double getStartTimeMs() const;

	/** @return size in megabits */
	double getSizeMb() const;

	nethost *getDestination() const;

	nethost *getSource() const;

	/**  @return last ACK's sequence number. */
	int getLastAck() const;

	/**
	 * Getter for number of packets used in a flawless transmission of this
	 * flow. In reality some packets may be dropped or lost, so this is really
	 * the largest packet sequence number in this flow, where the first packet
	 * has a sequence number of 1.
	 * @return number of packets in this flow
	 */
	int getNumTotalPackets() const;

	/** @return dynamically adjusted timeout length in milliseconds. */
	double getTimeoutLengthMs() const;

	int getHighestAckSeqnum() const;

	int getHighestSentSeqnum() const;

	int getNumDuplicateAcks() const;

	const map<int, double>& getRoundTripTimes() const;

	double getWindowSize() const;

	/** @return the packet on which the window starts. */
	int getWindowStart() const;

	double getLinGrowthWinsizeThreshold() const;

	const map<int, timeout_event *>& getFutureTimeoutsEvents() const;

	const map<int, ack_event*>& getFutureSendAckEvents() const;

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;

	// --------------------------- Mutators -----------------------------------

	void setSource(nethost &source);

	void setDestination(nethost &destination);

	void setLastACKNum(int new_seqnum);

	/**
	 * Cancels the timeout action corresponding the given sequence number
	 * by removing it from the local map and from the simulation's evnet
	 * queue. Invoke after an ACK is received or after another timeout is
	 * processed and a new one generated.
	 * @param seq sequence number corresponding to the packet that would have
	 * been retransmitted after a timeout
	 * @return the timeout event that was cancelled
	 * @post timeout_event erased from local map and from the global events
	 * queue.
	 */
	timeout_event *cancelTimeoutAction(int seq);

	/**
	 * Makes a timeout_event and puts it in the local map and on the
	 * simulation's event queue.
	 * @param seq the timeout_event will contain a FLOW packet with this
	 * sequence number
	 * @time time at which the timeout event should run
	 * @post timeout_event added to the local map and to the global events
	 * queue
	 */
	void registerTimeoutAction(int seq, double time);

	/**
	 * Cancels a future duplicate_ack_event. Invoke after an in-order FLOW
	 * packet is received, nullifying the pending duplicate_ack_event.
	 * @param seq sequence number corresponding to the pending
	 * duplicate_ack_event
	 * @return the duplicate_ack_event that was cancelled
	 * @post duplicate_ack_event erased from the local map and from the global
	 * events queue.
	 */
	ack_event *cancelSendDuplicateAckAction(int seq);

	/**
	 * Makes a duplicate_ack_event and puts it in the local map and on the
	 * simulation's event queue.
	 * @param seq the duplicate_ack_event will contain an ACK packet with this
	 * sequence number
	 * @time time at which the duplicate_ack_event should run
	 * @post duplicate_ack_event added to the local map and to the global
	 * events queue.
	 */
	void registerSendDuplicateAckAction(int seq, double time);

	/**
	 * Gets all the packets in this window that must be sent. This function
	 * assumes the user isn't going to send them, so it doesn't change the
	 * last packet sent number.
	 * @return all the outstanding packets in the window
	 */
	vector<packet> peekOutstandingPackets();

	/**
	 * Gets all the packets in this window that must be sent. This function
	 * assumes the user WILL send them, so it DOES change the last sent packet
	 * number. It also stores the starting time for each packet so RTTs
	 * can be computed later. It stores corresponding timeout events locally
	 * and puts them on the simulation's event queue too.
	 * @param start_time_ms time in millisaeconds at which the packets are put
	 * on the initial link buffer
	 * @return all the outstanding FLOW packets in the window
	 * @post timeout events have been added locally and pushed onto simulation
	 * event queue
	 */
	vector<packet> popOutstandingPackets(double start_time_ms);

	// TODO handle out-of-order ACKs (e.g. ACK 3 might get routed a slow way),
	// ACK 4 might get routed a fast way.

	/**
	 * When an ACK is received (it's assumed that the packet is arriving
	 * at this flow's source) this function must be called so the window will
	 * slide and resize, so duplicate ACKs will register, so the timeout
	 * length will adjust, and so the corresonding timeout event will be
	 * removed from this flow and simulation.
	 * @param pkt the received ACK packet.
	 * @param end_time_ms time in milliseconds at which this ACK was received;
	 * used to calculate RTTs.
	 * @warning DOESN'T SEND PACKETS, use @c popOutstandingPackets to get the
	 * packets to send after this function is called.
	 */
	void receivedAck(packet &pkt, double end_time_ms);

	/**
	 * When a FLOW packet is received (it's assumed that the packet is
	 * arriving at this flow's destination) this function should be called
	 * to make and queue an immediate duplicate_ack_event. When the
	 * duplicate_ack_event runs it will send an ACK packet immediately and
	 * will also chain (queue) a future duplicate_ack_event in case the next
	 * FLOW packet never comes. This function also removes the previous
	 * packet's corresponding duplicate_ack_event from the local map.
	 * @param pkt arriving FLOW packet
	 * @param arrival_time
	 */
	void receivedFlowPacket(packet &pkt, double arrival_time);

	/**
	 * This function should be called after a timeout so the window size can
	 * change accordingly and so the old timeout_event can be cancelled. It's
	 * the caller's responsibility to make and queue a new send_packet_event
	 * and a new timeout_event.
	 * @param to_pkt the packet that timed out
	 */
	void timeoutOccurred(const packet &to_pkt);
};

// ------------------------------- netlink class ------------------------------

/**
 * Represents a link in a simple network.
 */
class netlink : public netelement {

private:

	/** This link's rate in bytes per millisecond. */
	double rate_bpms;

	/** Signal propagation delay for this link in ms. */
	int delay_ms;

	/** Buffer capacity in bytes. */
	int buffer_capacity;

	/** Pointer to one end of this link. */
	netnode *endpoint1;

	/** Pointer to the other end of this link. */
	netnode *endpoint2;

	/**
	 * The sum of the end-to-end transmission times of all the packets in
	 * link buffer.
	 */
	double wait_time;

	/** The sum of the packet sizes in this buffer. */
	int buffer_occupancy;

	/**
	 * This link's FIFO buffer. Note that packets don't have real payloads
	 * so the size of this buffer in memory is small even though packets are
	 * stored by value.
	 */
	queue<packet> buffer;

	/**
	 * Helper for the constructors. Converts the buffer length from kilobytes
	 * to bytes and the rate from megabits per second to bytes per second.
	 */
	void constructor_helper(double rate_mbps, int delay, int buflen_kb,
			netnode *endpoint1, netnode *endpoint2);

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
			netnode &endpoint1, netnode &endpoint2);

	/**
	 * Sets the endpoints of this link to NULL.
	 * @param name of this link
	 * @param rate_mbps link rate in megabits per second
	 * @param delay_ms link delay in milliseconds
	 * @param buflen_kb the size of the only buffer on this link in kilobytes
     * This is stored internally in bytes.
	 */
	netlink (string name, double rate_mbps, int delay_ms, int buflen_kb);

	// --------------------------- Accessors ----------------------------------

	/** @return buffer length in bytes. */
	long getBuflen() const;

	/**  @return buffer length in kilobytes. */
	long getBuflenKB() const;

	/** @return delay in milliseconds. */
	int getDelay() const;

	netnode *getEndpoint1() const;

	netnode *getEndpoint2() const;

	/** @return link rate in bytes per second. */
	double getRateBytesPerSec() const;

	/** @return link rate in megabits per second. */
	double getRateMbps() const;

	/**
	 * @return end-to-end time in milliseconds for the given packet on this
	 * link.
	 */
	double getTransmissionTimeMs(const packet &pkt) const;

	/**
	 * @return time INTERVAL in milliseconds until this link will be available
	 * for the next packet.
	 * @warning does not return an absolute time
	 */
	double getWaitTimeIntervalMs() const;

	/**
	 * @return the buffer occupancy
	 */
	int getBufferOccupancy() const;

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;

	// --------------------------- Mutators -----------------------------------

	void setEndpoint1(netnode &endpoint1);

	void setEndpoint2(netnode &endpoint2);

	/**
	 * If the link buffer has space the given packet is added to the buffer
	 * and the rolling wait time and buffer occupancy are increased.
	 * @param pkt the packet to add to the buffer
	 * @return true if added to buffer successfully, false if dropped
	 */
	bool sendPacket(const packet &pkt);

	/**
	 * Called when a packet is received to free the link for subsequent
	 * packets; just dequeues the packet from the buffer.
	 * @param pkt_id the given packet ID number must match the ID of the
	 * packet about to be dequeued.
	 * @return true if the packet was dequeued (i.e. the given id matched
	 * and the buffer wasn't empty)
	 */
	bool receivedPacket(long pkt_id);
};

// -------------------------------- packet class ------------------------------

/**
 * Describes a packet in the simulated network, which can be one of the types
 * in the enumerated type @c packetType. Note that a real packet would have a
 * payload but this class doesn't provide for one.
 */
class packet : public netelement {

private:

	/** Unique ID number for this packet. */
	long pkt_id;

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

	/** Flow to which this packet belongs. */
	netflow *parent_flow;

	/** Packet size in megabits (to be consistent with flow) */
	double size;

	/** Sequence number of packet */
	int seqnum;

	/**
	 * Constructor helper. Does naive assignments; logic should be in the
	 * calling constructors.
	 */
	void constructorHelper(packet_type type, const string &source_ip,
				   const string &dest_ip, int seqnum,
				   netflow *parent_flow, double size);

public:

	/** Unique ID number generator. Initialized in corresponding cpp file. */
	static long id_gen;

	/** Size of a flow packet in bytes. */
	static const long FLOW_PACKET_SIZE = 1024;

	/** Size of an ACK packet in bytes. */
	static const long ACK_PACKET_SIZE = 64;

	/** Size of a routing packet in bytes. */
	static const long ROUTING_PACKET_SIZE = 64;

	packet();

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

	/**
	 * Some functions are forced to return a packet value, so the default
	 * packet constructor makes packets with ids of 0, and they're considered
	 * null in calling functions. This function just checks if a given packet
	 * is a "fake" packet.
	 * @return
	 */
	bool isNullPacket() const;

	string getSource() const;

	string getDestination() const;

	int getSeq() const;

	long getId() const;

	netflow *getParentFlow() const;

	packet_type getType() const;

	/** @return size in megabits. */
	double getSizeMb() const;

	/** @return size in bytes. */
	long getSizeBytes() const;

	/** @return type as a string */
	string getTypeString() const;

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;
};

#endif // NETWORK_H
