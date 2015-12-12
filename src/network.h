/**
 * @file
 *
 * Contains the declarations of all the network device classes as well as the
 * packet class.
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
#include <limits>
#include <set>

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
extern bool detail;
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

	/** Printouts nested to this depth, 2 spaces per nesting level. */
	int nest_depth;

public:

	/** Default constructor. Sets nesting depth to 0, name to empty string. */
	netelement ();

	/**
	 * Sets name to given one and sets nesting depth to 0.
	 * @param name of this network element.
	 */
	netelement (string name);

	/** Destructor. */
	virtual ~netelement();

	/**
	 * Getter for name.
	 * @return name of this network element.
	 */
	const string &getName() const;

	/**
	 * Setter for the nesting depth.
	 * @param depth
	 */
	void setNestingDepth(int depth);

	/**
	 * Gets a string with as many spaces as (nesting depth + delta) *
	 * spaces per level.
	 * @param delta effective nesting depth will be actual depth + delta
	 * @return string with number of spaces determined by desting depth
	 * and the delta from that desting depth.
	 */
	string nestingPrefix(int delta) const;

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
	if(&device) {
		device.printHelper(os);
	}
	return os;
}

// -------------------------------- netnode class -----------------------------

/**
 * Represents a node, which is either a node or a host, in a simple network.
 */
class netnode : public netelement {

protected:

	/**
	 * Pointers to all the links attached to this node. Devices with
	 * constraints on number of links that can be attached, like hosts,
	 * must enforce those constraints themselves.
	 */
	vector<netlink *> links;

public:

	/** Default constructor. */
	netnode ();

	/**
	 * Sets name to the given value and calls netelement's constuctor.
	 * @param name of this device
	 */
	netnode (string name);

	/**
	 * Sets the name of this device to the given one and copies the list
	 * of given link pointers into this object's list of links.
	 * @param name of this device
	 * @param links vector of pointers to links attached to this device
	 */
	netnode (string name, vector<netlink *> links);

	/**
	 * Returns the node connected to the input link that is not *this
	 * node.
	 * @param link get the node at the other end of this link
	 * @return pointer to the node at the other end of this link
	 */
	netnode *getOtherNode(netlink *link);

	/**
	 * Mutator that adds links to this device's list of links.
	 * @param link to add to this device's list of links
	 */
	virtual void addLink (netlink &link);

	/**
	 * Getter for the list of links attached to this device.
	 * @return the list of links attached to this device
	 */
	const vector<netlink *> &getLinks() const;

	/**
	 * Should be defined so this function returns true for packet-routers and
	 * false for packet-consumers.
	 * @return true if this node is capable of routing packets.
	 */
	virtual bool isRoutingNode() const = 0;

	/**
	 * Print helper function. Partially overrides superclass's.
	 * @param os The output stream to which to write netdevice information.
	 */
	virtual void printHelper(std::ostream &os) const;
};

// ------------------------------- nethost class ------------------------------

/**
 * Represents a host. The main difference between a @c nethost and a @c netnode
 * is that a host can be attached to only one link. Hosts implement a
 * selective ACK scheme; see the @c received field in @c netflow.
 */
class nethost : public netnode {

private:

	// No new fields.

public:

	/**
	 * Sets the name to the given one and the nesting depth to 0 via the
	 * @c netnode constructor.
	 * @param name of this host
	 */
	nethost (string name);

	/**
	 * Sets the name to the given one and sets exactly one link for this host.
	 * @param name of this host
	 * @param link the only link that's attached to this host
	 */
	nethost (string name, netlink &link);

	/**
	 * Returns false because this is a host, not a router.
	 * @return false
	 */
	bool isRoutingNode() const;

	/**
	 * Gets the first (and only, since this is a host) link.
	 * @return link attached to this host
	 */
	netlink *getLink() const;

	/**
	 * Deletes all links then adds the given one.
	 * @param link link to add after deleting the others
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
 * Represents a router in a simple network. The main difference from a
 * @c netnode is that this object contains a routing table and a collection
 * of distances to other nodes which is used to update the routing table.
 */
class netrouter : public netnode {

private:

	/**
	 * Routing table implemented as map from destination names to
	 * next-hop link.
	 */
	map<string, netlink *> rtable;

	/**
	 * Table of distances from this router to each node in the network.
	 * Distance to self and adjacent hosts are initialized to 0. Distance
	 * to other routers are initialized to max double, which is defined in
	 * @c climits.
	 */
	map<string, double> rdistances;

public:

	/**
	 * Default constructor. Sets name to empty string and nesting depth to 0.
	 */
	netrouter ();

	/**
	 * Sets name to the given one and nesting depth to 0.
	 * @param name of this router
	 */
	netrouter (string name);

	/**
	 * Sets name to the given one and the list of links attached to this
	 * router to the given one.
	 * @param name of this router
	 * @param links list of pointers to links attached to this router to copy
	 * into this object's list of link pointers
	 */
	netrouter (string name, vector<netlink *> links);

	/**
	 * Returns true, since this is a router.
	 * @return true
	 */
	virtual bool isRoutingNode() const;

	/**
	 * This function forwards it along the best link for it to get to its
	 * destination as determined by the routing table. Note that it's the
	 * caller's responsibility to generate the actual send_packet_events based
	 * on the returned map.
	 * @param time of packet receipt
	 * @param sim
	 * @param flow parent flow, NULL if ROUTING type
	 * @param pkt the arriving packet
	 * @return a map from the links to use to the corresponding packets
	 * @warning deprecated for use with ROUTING packets! Use
	 * @c receiveRoutingPacket instead.
	 */
	map<netlink *, packet> receivePacket(double time, simulation &sim,
			netflow &flow, packet &pkt);

	/**
	 * If this is a ROUTING packet, this function will update the router's
	 * routing table and distances table if necessary. If an update is made,
	 * it will also trigger send_packet_events to deliver additional routing
	 * packets to its neighbors.
	 * @param time of receipt of trigger packet
	 * @param sim
	 * @param pkt received
	 * @param link it was received on
	 */
	void receiveRoutingPacket(double time, simulation &sim, 
			packet &pkt, netlink &link);

	/**
	 * Getter for the node distances collection.
	 * @return collection of the distances of this router from all other nodes
	 */
	map<string, double> getRDistances() const;

	/**
	 * Called once at the beginning of the simulation, after parsing in an
	 * input file. Sets distance to self and adjacent hosts to 0. 
	 * Sets the correct link to adjacent hosts since each host
	 * has only one outgoing link. Sets other links to NULL.
	 *
	 * @param host_list list of all hosts
	 * @param router_list list of all routers
	 */
	void initializeTables(map<string, nethost*> host_list, 
						  map<string, netrouter*> router_list);

	/**
	 * Called from each router_discovery_event before recalculating distances.
	 * Sets distances to all other routers and nonadjacent hosts to infinity.
	 * @param host_list list of all hosts
	 * @param router_list list of all routers
	 */
	void resetDistances(map<string, nethost*> host_list, 
						map<string, netrouter*> router_list);

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;
};

// ------------------------------- netflow class ------------------------------

/**
 * Represents a flow in a simple network. Uses various constants defined in
 * @c util.cpp.
 */
class netflow : public netelement {

private:

	/** Start time in seconds from beginning of simulation. */
	double start_time_sec;

	/** Transmission size in megabits. */
	double size_mb;

	/** Number of megabits received. */
	double amt_received_mb;

	/** Pointer to one end of this flow. */
	nethost *source;

	/** Pointer to the other end of this flow. */
	nethost *destination;
	
	/** For plotting, number of flow packets received by destination */
	int pktTally;
	
	/** For plotting, start time of the packet count interval */
	double leftTime;
	
	/** For plotting, end time of the packet count interval */
	double rightTime;

	/** Highest received ACK sequence number (at source). */
	int highest_received_ack_seqnum;

	/** Highest sent FLOW packet sequence number (at source). */
	int highest_sent_flow_seqnum;
	
	/**
	 * Vector keeping track of which flow packets have/have not been
	 * received. The value at index i indicates whether flow packet 
	 * i has been received at the destination.
	 */
	vector<bool> received;

	/**
	 * Seqnum of next ack to be received. This should be the first empty slot
	 * after a contiguous sequence in the received vector.
	 */
	int next_ack_seqnum;

	/** The TCP protocol's window size. */
	double window_size;

	/** Sequence number at which the current transmission window starts. */
	int window_start;

	/** Number of duplicate ACKs seen. */
	int num_duplicate_acks;

	/**
	 * This is the maximum length of time in milliseconds that can pass b/t
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
	 * Flag that is true if this flow is using FAST TCP for congestion
	 * control. 
	 */
	bool FAST_TCP;

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
	 * Minimum round-trip time for a packet in this flow.
	 */
	double min_RTT;
	
	/** 
	 * Time elapsed since packet was send and corresponding acknowledgment
	 * was received by the source. Updated every receive_packet_event.
	 * Exists for purpose of plotting metric packet delay.
	 */ 
	double pkt_RTT;

	/**
	 * Pointer to the timer associated with the flow. Each time an ack is
	 * received, the timer is pushed back by removing this timeout event 
	 * from the events queue and replacing it with a new timeout event. The
	 * new event's timer extends the previous event's timer by
	 * timeout_length_ms.
	 */
	timeout_event *flow_timeout;

	/**
	 * Map from sequence numbers to round-trip times of those packets. If a
	 * value is negative then it's the FLOW packet departure time; the
	 * corresponding ACK hasn't arrived yet. When it does its arrival
	 * time will be added to the negative number.
	 */
	map<int, double> rtts;

	/**
	 * Don't send a duplicate ACK until this time. This time should be set to
	 * roughly another RTT when a duplicate ACK is sent.
	 */
	double dont_send_duplicate_ack_until;

	/**
	 * Wait for an ACK w/ this sequence number before resuming transmission
	 * after a duplicate ACK situation.
	 */
	int waiting_for_seqnum_before_resuming;

	/** Pointer to simulation so timeout_events can be made in this class. */
	simulation *sim;

	/**
	 * Updates the average round-trip time and standard deviation of round-
	 * trip times using recursive formulas.
	 * @param rtt time of receipt of a packet. Used to compute the
	 * round-trip time.
	 * @param flow_seqnum the sequence number of the packet that was just
	 * received.
	 */
	void updateTimeoutLength(double rtt, int flow_seqnum);

	/**
	 * Constructor helper. Does naive assignments; logic should be in the
	 * calling constructors.
	 */
	void constructorHelper (double start_time, double size_mb,
			nethost &source, nethost &destination, double window_size,
			bool usingFAST, double timeout_length_ms, simulation &sim);

public:

	/** Number of duplicate ACKs before fast retransmit is used. */
	static const int FAST_RETRANSMIT_DUPLICATE_ACK_THRESHOLD = 3;

	/**
	 * The timeout value for the very first packet sent in a flow; the
	 * timeout value is later seeded with the first packet's RTT and then
	 * changed with recursive average and deviation algorithms.
	 */
	static constexpr double DEFAULT_INITIAL_TIMEOUT = 1000.0;

	/** The constant 'b' from the recursive average and std formulas. */
	static constexpr double B_TIMEOUT_CALC = 0.1;

	/**
	 * A small fraction of a millisecond used to order timeout events and
	 * regular send packet events.
	 */
	static constexpr double TIME_EPSILON = 0.0000000001;

	/**
	 * Initializes the flow's basic attributes as well as attributes required
	 * to perform TCP Reno transmissions, like window size, last ACK seen,
	 * number of duplicate ACKs, etc.
	 * @param name of this flow
	 * @param start_time in seconds at which this flow starts
	 * @param size_mb size of this flow in megabits
	 * @param source host at which this flow starts
	 * @param destination host at which this flow ends
	 * @param usingFAST true if using FAST TCP, false if using TCP Tahoe
	 * @param sim
	 */
	netflow (string name, double start_time, double size_mb,
			nethost &source, nethost &destination, bool usingFAST,
			simulation &sim);

	/**
	 * Same as other constructor but @c usingFAST is set to false.
	 * @param name of this flow
	 * @param start_time in seconds at which this flow starts
	 * @param size_mb size of this flow in megabits
	 * @param source host at which this flow starts
	 * @param destination host at which this flow ends
	 * @param sim
	 */
	netflow (string name, double start_time, double size_mb,
			nethost &source, nethost &destination, simulation &sim);

	// --------------------------- Accessors ----------------------------------

	/**
	 * Getter for start time in seconds.
	 * @return start time in seconds from beginning of simulation.
	 */
	double getStartTimeSec() const;

	/**
	 * Getter for start time in milliseconds.
	 * @return start time in milliseconds from beginning of simulation.
	 */
	double getStartTimeMs() const;

	/**
	 * Getter for size in megabits.
	 * @return size in megabits
	 */
	double getSizeMb() const;

	/**
	 * Getter for pointer to destination host.
	 * @return pointer to destination host.
	 */
	nethost *getDestination() const;

	/**
	 * Getter for pointer to source host.
	 * @return get pointer to the source host.
	 */
	nethost *getSource() const;

	/**
	 * Getter for the count of flow packets received by destination within the
	 * interval.
	 * @return number of flow packets received
	 */
	int getPktTally() const;
	
	/**
	 * Getter for the start time of the packet count interval.
	 * @return left side of pkt count interval
	 */
	double getLeftTime() const;
	
	/**
	 * Getter for the ending time of the packet count interval.
	 * @return right side of pkt count interval
	 */
	double getRightTime() const;

	/**
	 * Getter for number of packets used in a flawless transmission of this
	 * flow. In reality some packets may be dropped or lost, so this is really
	 * the largest packet sequence number in this flow, where the first packet
	 * has a sequence number of 1.
	 * @return number of packets in this flow
	 */
	int getNumTotalPackets() const;

	/**
	 * Getter for the dynamically adjusted timeout length in milliseconds.
	 * @return timeout length in milliseconds.
	 */
	double getTimeoutLengthMs() const;

	/**
	 * Getter for the sequence number of the last ACK seen.
	 * @return last ACK's sequence number.
	 */
	int getHighestAckSeqnum() const;

	/**
	 * Getter for the number of duplicate ACKs seen so far.
	 * @return num duplicate ACKs
	 */
	int getNumDuplicateAcks() const;

	/**
	 * Getter for the average round trip time.
	 * @return average RTT
	 */
	double getAvgRTT() const;

	/**
	 * Getter for the minimum round trip time seen so far for this flow.
	 * @return min RTT
	 */
	double getMinRTT() const;

	/**
	 * True if using TCP FAST, false if Tahoe.
	 * @return true for FAST, false for Tahoe
	 */
	bool isUsingFAST() const;

	/**
	 * Getter for a const reference to the round-trip times map. This map
	 * stores the start time for packets in transit; when packets arrive
	 * their arrival times are added to their (negated) start times to get
	 * the round trip time, then their entries are removed from the RTT map.
	 * @return const reference to round trip times map
	 */
	const map<int, double>& getRoundTripTimes() const;

	/**
	 * Getter for the current window size
	 * @return window size
	 */
	double getWindowSize() const;

	/**
	 * Getter for the sequence number at which the current window starts.
	 * @return the packet on which the window starts.
	 */
	int getWindowStart() const;

	/**
	 * Getter for the linear growth threshold (i.e., the window size after
	 * which the window size will be increased as 1/n instead of 1 per ACK)
	 * @return linear growth window size threshold
	 */
	double getLinGrowthWinsizeThreshold() const;

	double getPktRTT() const;

	/**
	 * Print helper function which partially overrides the one in netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;
	
	/**
	 * Getter for the flow rate in bytes per second.
	 * @return flow rate in bytes/sec.
	 */
	double getFlowRateBytesPerSec() const;

	/**
	 * Getter for the flow rate in megabits per second.
	 * @return flow rate.
	 */
	double getFlowRateMbps(double time) const;

	/**
	 * Getter for the percentage of flow that has been transmitted so far
	 * Useful for debugging; can be graphed along with other flow metrics.
	 * @return the percentage of the flow that's been completed.
	 */
	double getFlowPercentage() const;

	/**
	 * Packet delay is defined as time elapsed since packet is send and
	 * acknowledgment is received.
	 * @param currTime current time
	 * @return packet delay
	 */
	double getPktDelay(double currTime) const;

	/** Returns true if flow has finished transmitting
	 * @return bool
	 */
	bool doneTransmitting();

	// --------------------------- Mutators -----------------------------------

	/**
	 * Setter for the source host.
	 * @param source
	 */
	void setSource(nethost &source);

	/**
	 * Setter for the destination host.
	 * @param destination
	 */
	void setDestination(nethost &destination);
	
	/**
	 * Updates packet tally depending on whether @c receive_packet_event
	 * occurs during rate_interval.
	 * @param time
	 */
	void updatePktTally(double time);

	/**
	 * Changes a flow's window size during an update_window_event. Used only
	 * if the flow is using FAST TCP for congestion control.
	 * @param new_size
	 */
	void setFASTWindowSize(double new_size);
	
	/**
	 * Sets left time.
	 * @param newTime
	 */
	void setLeftTime(double newTime);
	
	/**
	 * Sets right time.
	 * @param newTime
	 */
	void setRightTime(double newTime);

	/**
	 * Makes an @c ack_event and puts it on the simulation's event queue.
	 * @param seq the @c ack_event will contain an ACK packet with this
	 * sequence number
	 * @param arrival_time 
	 * @param sent_time 
	 */
	void registerAckEvent(int seq, double arrival_time, double sent_time);

	/**
	 * Gets all the packets in this window that must be sent. This function
	 * assumes the user isn't going to send them, so it doesn't change the
	 * last packet sent number.
	 *
	 * @return all the outstanding packets in the window
	 */
	vector<packet> peekOutstandingPackets();

	/**
	 * Gets all the packets in this window that must be sent. This function
	 * assumes the user WILL send them, so it DOES change the last sent packet
	 * number. It also stores the starting time for each packet so RTTs
	 * can be computed later. It stores corresponding timeout events locally
	 * and puts them on the simulation's event queue too.
	 *
	 * @param start_time time at which packets will enter link buffer
	 * @param linkFreeAt time in milliseconds at packets will get on the link
	 * @return all the outstanding FLOW packets in the window
	 */
	vector<packet> popOutstandingPackets(double start_time, double linkFreeAt);

	/**
	 * When an ACK is received (it's assumed that the packet is arriving
	 * at this flow's source) this function must be called so the window will
	 * slide and resize, so duplicate ACKs will register, and so the timeout
	 * length will adjust.
	 *
	 * @param pkt the received ACK packet.
	 * @param end_time_ms time in milliseconds at which this ACK was received;
	 * used to calculate RTTs.
	 * @param linkFreeAtTime absolute time at which the link will be free
	 *
	 * @warning DOESN'T SEND PACKETS, use @c popOutstandingPackets to get the
	 * packets to send after this function is called.
	 */
	void receivedAck(packet &pkt, double end_time_ms, double linkFreeAtTime);

	/**
	 * When a FLOW packet is received (it's assumed that the packet is
	 * arriving at this flow's destination) this function should be called
	 * to make and queue an immediate duplicate_ack_event. When the
	 * duplicate_ack_event runs it will send an ACK packet immediately and
	 * will also chain (queue) a future duplicate_ack_event in case the next
	 * FLOW packet never comes. This function also removes the previous
	 * packet's corresponding duplicate_ack_event from the local map.
	 *
	 * @param pkt arriving FLOW packet
	 * @param arrival_time
	 */
	void receivedFlowPacket(packet &pkt, double arrival_time);

	/**
	 * This function should be called after a timeout so the window size can
	 * change accordingly and so the old timeout_event can be cancelled. It's
	 * the caller's responsibility to make and queue a new send_packet_event
	 * and a new timeout_event.
	 */
	void timeoutOccurred();
};

// ------------------------------- netlink class ------------------------------

/**
 * Represents a half-duplex link.
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
	 * This link's FIFO buffer. Note that packets don't have real payloads
	 * so the size of this buffer in memory is small even though packets are
	 * stored by value. The keys are arrival times and the values are the
	 * actual packets in the link buffer; to get arrival time for a new
	 * packet about to be placed on the buffer add transmission (and possibly
	 * delay) time to the arrival time of the last element in the buffer.
	 */
	map<double, packet> buffer;

	/**
	 * Represents packet loss. Since assuming nothing happens to the packet
	 * whle the packet is 'in transit'. This value keeps track of the number
	 * of packets dropped in one go due to a full link buffer.
	 */
	int packets_dropped = 0;
	
	/**
	 * For plotting link rate. Keeps track of how many packets of each type
	 * were passing through link during given time interval. Key is a string
	 * instead of packet_type for easier printing.
	 */ 
	map<string, int> linkTraffic = { {"ack", 0}, {"flow", 0}, {"rtr", 0} };

	/** For plotting, start time of the packet count interval */
	double leftTime;
	
	/** For plotting, end time of the packet count interval */
	double rightTime;

	/** Destination of last packet added to the buffer. */
	netnode *destination_last_packet;

	/**
	 * Helper for the constructors. Converts the buffer length from kilobytes
	 * to bytes and the rate from megabits per second to bytes per second.
	 */
	void constructor_helper(double rate_mbps, int delay, int buflen_kb,
			netnode *endpoint1, netnode *endpoint2);

public:

	/**
	 * Use this when endpoints are known at construction time.
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
	 * Use this when endpoints are not known at construction time.
	 * @param name of this link
	 * @param rate_mbps link rate in megabits per second
	 * @param delay_ms link delay in milliseconds
	 * @param buflen_kb the size of the only buffer on this link in kilobytes
     * This is stored internally in bytes.
	 */
	netlink (string name, double rate_mbps, int delay_ms, int buflen_kb);

	// --------------------------- Accessors ----------------------------------

	/**
	 * Getter for the buffer's length in bytes (not occupancy).
	 * @return buffer length in bytes.
	 */
	long getBuflen() const;

	/**
	 * Getter for the buffer's length in kilobytes (not occupancy).
	 * @return buffer length in kB.
	 */
	long getBuflenKB() const;

	/**
	 * Getter for the link delay.
	 * @return delay in milliseconds.
	 */
	int getDelay() const;

	/**
	 * Getter for one of the endpoints that this link connects. This might be
	 * a node or a router.
	 * @return one of the endpoints
	 */
	netnode *getEndpoint1() const;

	/**
	 * Getter for the other endpoint that this link connects. This might be
	 * a node or a router.
	 * @return the other endpoint
	 */
	netnode *getEndpoint2() const;

	/**
	 * Getter for the capacity of this link in bits per second
	 * @return link capacity (bps)
	 */
	double getCapacityBitsPerSec() const;

	/**
	 * Getter for the capacity of this link in megabits per second
	 * @return link capacity (mbps)
	 */
	double getCapacityMbps() const;

	/**
	 * Getter for the current rate of the link in megabits per second.
	 * @return current link rate (mbps)
	 */
	double getRateMbps();

	/**
	 * Getter for end-to-end time in milliseconds for the given packet on this
	 * link NOT INCLUDING DELAY.
	 * @param pkt
	 * @return end-to-end time for this packet (ms)
	 */
	double getTransmissionTimeMs(const packet &pkt) const;

	/**
	 * Getter for the absolute time in milliseconds when this link will be
	 * available for the next packet.
	 * @return time at which link will be available for next-queued packet
	 * @warning if zero need to substitute current time in for free at time!
	 * the link doesn't know the current time, so it can't tell you when
	 * the link is free if there's nothing in the buffer.
	 */
	double getLinkFreeAtTime() const;

	/**
	 * Getter for the number of bytes of the link buffer than are in-use.
	 * @return the buffer occupancy (bytes)
	 */
	long getBufferOccupancy() const;

	/**
	 * Gette for the packet loss, which is number of packets dropped since
	 * we are assuming nothing happens to a packet while in transit.
	 * @return packet loss
	 */
	int getPktLoss() const;

	/** 
	 * Getter for the link traffic.
	 * @return linkTraffic
	 */
	map<string, int> getLinkTraffic() const;

	/**
	 * This function is critical for our half-duplex implementation.
	 * Returns true if the direction of the last packet in the buffer is the
	 * same as the direction of the packet about to be added; if the
	 * direction is the same then the link delay should not be used, since it
	 * was already used once for the first packet in this run of same-direction
	 * packets.
	 * @param destination
	 * @return true if the destination is the same as the destination of the
	 * last packet in the buffer
	 */
	bool isSameDirectionAsLastPacket(netnode *destination);

	/**
	 * Gets the arrival time of a packet on the other end of the link.
	 * @param pkt
	 * @param useDelay
	 * @param time of the triggered event
	 * @return arrival time
	 */
	double getArrivalTime(const packet &pkt, bool useDelay, double time);

	/**
	 * Prints the contents and size of the buffer to the given stream.
	 * Extremely useful for debugging.
	 * @param os print the buffer contents to this output stream.
	 */
	void printBuffer(ostream &os);

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;

	// --------------------------- Mutators -----------------------------------

	/**
	 * Setter for the first endpoint.
	 * @param endpoint1
	 */
	void setEndpoint1(netnode &endpoint1);

	/**
	 * Setter for the first endpoint.
	 * @param endpoint1
	 */
	void setEndpoint2(netnode &endpoint2);
	
	/**
	 * If the link buffer has space the given packet is added to the buffer
	 * and the rolling wait time and buffer occupancy are increased.
	 *
	 * @param pkt the packet to add to the buffer
	 * @param destination of the packet
	 * @param useDelay true if link delay should be used to sum into the
	 * buffer's wait time. Should be used ONCE per window
	 * @param time at which we're trying to send this packet
	 *
	 * @return true if added to buffer successfully, false if dropped
	 */
	bool sendPacket(const packet &pkt, netnode *destination,
			bool useDelay, double time);

	/**
	 * Called when a lone packet is received to free the link for subsequent
	 * packets; just dequeues the packet from the buffer.
	 *
	 * @param pkt_id the given packet ID number must match the ID of the
	 * packet about to be dequeued.
	 *
	 * @return true if the packet was dequeued (i.e. the given id matched
	 * and the buffer wasn't empty)
	 */
	bool receivedPacket(long pkt_id);

	/**
	 * Call this instead of @c receivedLonePacket when an arriving packet
	 * is a part of a window so that the link buffer's wait time is adjusted
	 * correctly. For the first packet in a windowload the buffer's wait time
	 * is decreasing by the link delay + one transmission time, but for
	 * subsequent packets the buffer wait time is decreased by only one
	 * transmission time.
	 *
	 * @param pkt_id the given packet ID number must match the ID of the
	 * packet about to be dequeued.
	 * @param first_packet true if first packet in the window
	 *
	 * @return true if the packet was dequeued (i.e. the given id matched
	 * and the buffer wasn't empty)
	 */
	bool receivedPacketInWindow(long pkt_id, bool first_packet);

	/**
	 * Resets all values in linkTraffic map to 0.
	 */
	void resetLinkTraffic();	
	
	/**
	 * Updates value of linkTraffic accordingly
	 * @param time
	 * @param type
	 */
	void updateLinkTraffic(int time, packet_type type);
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

	/** Distance vector for use in routing messages. **/
	map<string, double> distance_vec;

	/** Transmit time (stored as double), for calculating link costs. */
	double transmit_timestamp;

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

	/**
	 * Default contructor. Sets everything to dummy values.
	 */
	packet();

	/**
	 * This constructor infers the size of a packet from the given type, which
	 * must be ROUTING since there's no parent flow and no sequence number.
	 * and no sequence number.
	 *
	 * @param type one of the values of the @c packet_type enum, but must
	 * be ROUTING for this constructor
	 * @param source_ip the NAME (since this simulation uses names, not IPs)
	 * of the original source which must be a router
	 * @param dest_ip the NAME of the ultimate destination, which must be a
	 * router
	 *
	 * @warning assertion triggered if the packet type isn't ROUTING
	 */
	packet(packet_type type, const string &source_ip, const string &dest_ip);

	/**
	 * This constructor infers the size of a packet from the given type, which
	 * must be ACK or FLOW since the source and destination are inferred from
	 * a parent flow.
	 *
	 * @param type one of the values of the @c packet_type enum, but must
	 * be ROUTING for this constructor
	 * @param parent_flow flow to which this packet belongs. ACK and FLOW
	 * packets belong to flows.
	 * @param seqnum sequence number for this packet, where the first packet in
	 * flow is numbered 1. Pass in anything (or @c SEQNUM_FOR_NONFLOWS) for ACK
	 * packets; their seqnums are just set to @c SEQNUM_FOR_NONFLOWS.
	 *
	 * @warning assertion is triggered if the packet type is not ACK or FLOW.
	 */
	packet(packet_type type, netflow &parent_flow, int seqnum);

	/**
	 * Some functions are forced to return a packet value, so the default
	 * packet constructor makes packets with ids of 0, and they're considered
	 * null in calling functions. This function just checks if a given packet
	 * is a "fake" packet.
	 *
	 * @return true if null packets
	 */
	bool isNullPacket() const;

	/**
	 * Getter for the source host of this packet.
	 * @return source host name
	 */
	string getSource() const;

	/**
	 * Getter for the destination host of this packet.
	 * @return destination host name
	 */
	string getDestination() const;

	/**
	 * Getter for the sequence number of this packet.
	 * @return sequence number
	 */
	int getSeq() const;

	/**
	 * Getter for the unique ID of this packet.
	 * @return ID number
	 */
	long getId() const;

	/**
	 * Getter for the distances map.
	 * @return distnaces map
	 */
	const map<string, double> &getDistances() const;

	/**
	 * Setter for the distances map.
	 * @param distances
	 */
	void setDistances(map<string, double> distances);

	/**
	 * Getter for the parent flow of this packet.
	 * @return parent flow
	 */
	netflow *getParentFlow() const;

	/**
	 * Getter for the type of this packet.
	 * @return packet type
	 */
	packet_type getType() const;

	/**
	 * Getter for the size in megabits of this packet.
	 * @return size in megabits.
	 */
	double getSizeMb() const;

	/**
	 * Getter for the size in bytes of this packet.
	 * @return size in bytes.
	 */
	long getSizeBytes() const;

	/**
	 * Getter for the type of this packet as a string.
	 * @return type as a string.
	 */
	string getTypeString() const;

	/**
	 * Getter for the transmit timestamp for this packet.
	 * @return transmit timestamp
	 */
	double getTransmitTimestamp() const;

	/**
	 * Setter for the transmit timestamp.
	 * @param time
	 */
	void setTransmitTimestamp(double time);

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const;
};

#endif // NETWORK_H
