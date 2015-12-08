/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef EVENTS_H
#define EVENTS_H

// Standard includes.
#include <iostream>

// Custom headers
#include "util.h"
#include "network.h"

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

// -------------------------------- event class -------------------------------

/**
 * Base class for events in an event-driven network simulation.
 */
class event {

private:

	/** Time of this event. */
	double time;

	/** ID number of this object. */
	long id;

protected:

	/**
	 * Pointer to simulation this event is in so generated events can be
	 * added to its events queue.
	 */
	simulation *sim;

public:

	/** Unique ID number generator. Initialized in corresponding cpp file. */
	static long id_generator;

	event();

	/**
	 * Initializes this event's time to the given one and sets the event id
	 * to whatever the static ID generates spits out.
	 */
	event(double time, simulation &sim);

	virtual ~event();

	double getTime() const;

	long getId() const;

	/**
	 * Subclasses--i.e. more specific events--will run operations like
	 * sending packets, adding new events to the simulation event queue, etc.
	 * Every event will log data.
	 */
	virtual void runEvent();

	/**
	 * Print helper function. Derived classes should (partially) override this.
	 * @param os The output stream to which to write event information.
	 */
	virtual void printHelper(ostream &os);
};

/**
 * Output operator override for printing contents of the given event
 * to an output stream. Uses the printHelper function, which is virtual
 * because derived classes will want to modify or enhance printing behavior.
 * @param os The output stream to which to write.
 * @param device The @c event object to write.
 * @return The same output stream for operator chaining.
 */
inline ostream & operator<<(ostream &os, event &e) {
	e.printHelper(os);
	return os;
}

// --------------------------- receive_packet_event class ---------------------

/**
 * Event that represents the arrival of a packet at either an intermediate
 * node (router) or a final destination (host). The packet can be a FLOW,
 * ACK, or ROUTING packet.
 */
class receive_packet_event : public event {

private:

	/**  Flow to which the received packet belongs. Is NULL for routing
	 * packets. */
	netflow *flow;

	/** Packet received by the flow. */
	packet pkt;

	/** Host or router that is receiving the packet. */
	netnode *step_destination;

	/** Link on which this packet arrived. */
	netlink *link;

	/** Size of the window to which this packet belongs, 1 for lone packets. */
	int window_size;

	/**
	 * Sequence number of the first packet in this windowload. Assuming the
	 * sequence numbers of the packets are contiguous (i.e., no selective
	 * ACK scheme) then we can stack the transmissions of the packets in
	 * the window (ordered by their sequence numbers) using the relative
	 * distance from the first sequence number of the windowload to find
	 * the arrival time of each packet.
	 */
	int window_start;

	void constructorHelper(netflow *flow, packet &pkt,
			netnode *step_destination, netlink *link,
			int window_size, int window_start);
public:

	/**
	 * Constructor for receive packet events for lone packets; the window size
	 * and start are set to 1 and this packet's sequence number, respectively.
	 */
	receive_packet_event(double time, simulation &sim,
			netflow &flow, packet &pkt, netnode &step_destination,
			netlink &link);

	/**
	 * Constructor for receive packet events for packets that do not belong
	 * to a flow (i.e., routing packets).
	 */
	receive_packet_event(double time, simulation &sim, packet &pkt, 
			netnode &step_destination, netlink &link);

	/**
	 * Constructor for receive packets events for packets that belong to
	 * window loads; the window size and window start values are set to the
	 * given ones.
	 */
	receive_packet_event(double time, simulation &sim,
			netflow &flow, packet &pkt, netnode &step_destination,
			netlink &link, int window_size, int window_start);

	~receive_packet_event();

	/**
	 * If this arrival event is at a router then we consult the routing
	 * table for the link to use for this packet's destination and use
	 * it to generate a send_packet_event down that link.
	 *
	 * If the packet is arriving at a host then we create an ACK packet, make
	 * and queue a send_packet_event for the ACK, and queue a future
	 * send_packet_event for a duplicate ACK in case we don't see the next
	 * packet in the sequence. We also remove the flow's pending duplicate
	 * ACK send_packet_event for the packet with the preceding sequence
	 * number.
	 */
	void runEvent();

	/**
	 * Print helper function.
	 * @param os The output stream to which to write event information.
	 */
	void printHelper(ostream &os);
};

// ------------------------- router_discovery_event class ---------------------

/**
 * Event that triggers a given router's routing-table-population algorithm.
 */
class router_discovery_event : public event {

private:

	/** Router whose routing table will be updated by this event. */
	netrouter *router;

public:

	/**
	 * Initializes this event's time to the given one and sets the event ID,
	 */
	router_discovery_event(double time, simulation &sim);

	~router_discovery_event();

	/**
	 * Runs the distributed Bellman-Ford algorithm from this router.
	 */
	void runEvent();

	/**
	 * Print helper function.
	 * @param os The output stream to which to write event information.
	 */
	void printHelper(ostream &os);
};

// --------------------------- update_window_event class ------------------------

/**
 * Event that triggers update of a given flow's window size. 
 * Only to be used for FAST TCP.
 */
class update_window_event : public event {

private:
	/** Flow whose window size will be modified by this event. */
	netflow *flow;

public: 
	update_window_event(double time, simulation &sim, netflow &flow);

	~update_window_event();

	/** Updates window size based on FAST specifications. */
	void runEvent();

	/**
	 * Print helper function.
	 * @param os The output stream to which to write event information.
	 */
	void printHelper(ostream &os);
};

// --------------------------- send_packet_event class ------------------------

/**
 * Sends a packet from a given departure node and down a given link whether
 * it's an ACK, FLOW, or ROUTING packet. Assumes that timeout_events
 * and other flow attributes like highest_sent_seqnum have been dealt with
 * before this event runs.
 */
class send_packet_event : public event {

private:

	/** Parent flow of this packet, null if packet has ROUTING type. */
	netflow *flow;

	/** The packet being sent. */
	packet pkt;

	/** The link being used in this leg of the packet's journey. */
	netlink *link;

	/** Node from which the packet is going to leave. */
	netnode *departure_node;

	/** Size of the window to which this packet belongs, 1 for lone packets. */
	int window_size;

	/**
	 * Sequence number of the first packet in this windowload. Assuming the
	 * sequence numbers of the packets are contiguous (i.e., no selective
	 * ACK scheme) then we can stack the transmissions of the packets in
	 * the window (ordered by their sequence numbers) using the relative
	 * distance from the first sequence number of the windowload to find
	 * the arrival time of each packet.
	 */
	int window_start;

	/** @return node at which this packet arrives. */
	netnode *getDestinationNode() const;

	void constructorHelper(netflow *flow, packet &pkt,
			netlink *link, netnode *departure_node,
			int window_size, int window_start);
public:

	send_packet_event();

	/**
	 * Constuctor for lone packets--window size is set to 1 and window_start
	 * is set to this packet's sequence number.
	 */
	send_packet_event(double time, simulation &sim, netflow &flow,
			packet &pkt, netlink &link, netnode &departure_node);

	/**
	 * Constructor for packets that do not belong to a flow (i.e., routing 
	 * packets).
	 */
	send_packet_event(double time, simulation &sim, packet &pkt, 
			netlink &link, netnode &departure_node);

	/**
	 * Constructor for windowloads of packets--window_size is set to the given
	 * value and window_start is set to the given value.
	 */
	send_packet_event(double time, simulation &sim, netflow &flow,
			packet &pkt, netlink &link, netnode &departure_node,
			int window_size, int window_start);

	~send_packet_event();

	/**
	 * Finds time of arrival to next node from the given departure node down
	 * the given link and uses the arrival time to queue a receive_packet_event
	 * (does nothing if the link buffer has no room, thereby dropping the
	 * packet). Doesn't make or queue timeout events because they should have
	 * been made and queued in parallel with the send_packet_event.
	 */
	void runEvent();

	/**
	 * Print helper function.
	 * @param os The output stream to which to write event information.
	 */
	void printHelper(ostream &os);
};

// ---------------------------- start_flow_event class ------------------------

/**
 * Event that runs when a flow is about to start. Sends the first packet and
 * has the flow register a timeout event internally and queue it up in the
 * simulation's event queue.
 */
class start_flow_event : public event {

private:

	/** Flow that we're going to start. */
	netflow *flow;

public:

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * and sets the flow that this start_flow_event is going to start.
	 */
	start_flow_event(double time, simulation &sim, netflow &flow);

	~start_flow_event();

	/**
	 * Sends the first packet in this event's flow and queues a timeout_event
	 * internally and on the simulation event queue.
	 */
	void runEvent();

	/**
	 * Print helper function.
	 * @param os The output stream to which to write event information.
	 */
	void printHelper(ostream &os);
};

// ---------------------------- timeout_event class ---------------------------

/**
 * Event that sets the window size to one then sends a packet. This event
 * queues another timeout_event; timeout_events are chained so that the
 * flow will keep trying to send packets in the face of timeouts indefinitely.
 */
class timeout_event : public event {

private:

	/** Flow to which to register a timeout. */
	netflow *flow;

public:

	timeout_event();

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * and sets the flow to which this timeout_event belongs.
	 */
	timeout_event(double time, simulation &sim, netflow &flow);

	~timeout_event();

	/**
	 * Registers a timeout with the flow, which means it changes its window
	 * size and linear growth threshold internally. This function also chains
	 * (queues another) timeout_event and sends a packet by queueing a new
	 * send_packet_event.
	 */
	void runEvent();

	/**
	 * Print helper function.
	 * @param os The output stream to which to write event information.
	 */
	void printHelper(ostream &os);
};

// ------------------------- ack_event class ------------------------

/**
 * This event should be queued when a destination host wants to send an ACK
 * (not just duplicate ACKs--any ACKS). The event automatically queues another
 * ack_event so that duplicate ACKs will be sent in the future
 * if the destination doesn't get the correct FLOW packets. Pending
 * ack_events should be cancelled when the destination gets the
 * correct FLOW packet.
 */
class ack_event : public event {

private:

	/** Flow to which to register a duplicate ACK. */
	netflow *flow;

	/** The duplicate ACK. */
	packet dup_pkt;

public:

	ack_event();

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * and sets the flow to which this ack_event belongs.
	 */
	ack_event(double time, simulation &sim,
			netflow &flow, packet &dup_pkt);

	~ack_event();

	/**
	 * Send an ACK packet then chains (queues another) ack_event.
	 */
	void runEvent();

	/**
	 * Print helper function.
	 * @param os The output stream to which to write event information.
	 */
	void printHelper(ostream &os);
};

#endif // EVENTS_H
