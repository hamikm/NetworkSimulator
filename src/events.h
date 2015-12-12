/**
 * @file
 *
 * Contains the declarations of all the event classes used in this
 * simulation. Objects of these events are put onto an event queu in a
 * global simulation object.
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
 * Base class for events in our event-driven network simulation.
 */
class event {

private:

	/** Time of this event. */
	double time;

	/** ID number of this object. */
	long id;

protected:

	/**
	 * Pointer to simulation so generated events can be
	 * added to its events queue.
	 */
	simulation *sim;

public:

	/** Unique ID number generator. Initialized in corresponding cpp file. */
	static long id_generator;

	/** Default constructor; sets time and ID to -1 and simulation to NULL. */
	event();

	/**
	 * Initializes this event's time to the given one and sets the event id
	 * to whatever the static ID generates spits out. Also sets the
	 * simulation pointer.
	 * @param time at which this event should run
	 * @param sim
	 */
	event(double time, simulation &sim);

	/** Destructor */
	virtual ~event();

	/**
	 * Getter for the time at which this event should run.
	 * @return time at which this event should run.
	 */
	double getTime() const;

	/**
	 * Getter for the unique ID number generated for this event.
	 * @return ID number
	 */
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

	/**
	 * Flow to which the received packet belongs. NULL for routing
	 * packets.
	 */
	netflow *flow;

	/** Packet received by the flow. */
	packet pkt;

	/** Host or router that is receiving the packet. */
	netnode *step_destination;

	/** Link on which this packet arrived. */
	netlink *link;

	/**
	 * Constructor helper. Does naive assignments; logic should be in the
	 * calling constructors.
	 * @param flow Flow packet originated from
	 * @param pkt Packet contents
	 * @param step_destination Node at the other side of link packet is
	 * currently traveling
	 * @param link Link packet is currently traveling
	 */
	void constructorHelper(netflow *flow, packet &pkt,
			netnode *step_destination, netlink *link);
public:

	/**
	 * Constructor for receive packet events for lone packets; the window size
	 * and start are set to 1 and this packet's sequence number, respectively.
	 * @param time Time when packet should be received
	 * @param sim
	 * @param flow Flow packet originated from
	 * @param pkt Packet associated with this event
	 * @param step_destination Node at the other side of link packet is
	 * currently traveling
	 * @param link Link packet is currently traveling
	 */
	receive_packet_event(double time, simulation &sim,
			netflow &flow, packet &pkt, netnode &step_destination,
			netlink &link);

	/**
	 * Constructor for receive packet events for packets that do not belong
	 * to a flow (i.e., routing packets).
	 * @param time Time when packet should be received
	 * @param sim
	 * @param pkt Packet associated with this event
	 * @param step_destination Node at the other side of link packet is
	 * currently traveling
	 * @param link Link packet is currently traveling
	 */
	receive_packet_event(double time, simulation &sim, packet &pkt, 
			netnode &step_destination, netlink &link);

	/** Destructor. */
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
	 * Initializes this event's time to the given one and sets the event ID.
	 * @param time
	 * @param sim
	 */
	router_discovery_event(double time, simulation &sim);

	/** Destructor */
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
 * Event that triggers update of a given flow's window size. Only to be used
 * for FAST TCP.
 */
class update_window_event : public event {

private:

	/** Flow whose window size will be modified by this event. */
	netflow *flow;

public: 

	/**
	 * Constructor.
	 * @param time
	 * @param sim
	 * @param flow
	 */
	update_window_event(double time, simulation &sim, netflow &flow);

	/** Destructor. */
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

	/** @return node at which this packet arrives. */
	netnode *getDestinationNode() const;

	/**
	 * Constructor helper. Does naive assignments; logic should be in the
	 * calling constructors.
	 * @param flow
	 * @param pkt
	 * @param link
	 * @param departure_node
	 */
	void constructorHelper(netflow *flow, packet &pkt,
			netlink *link, netnode *departure_node);
public:

	/** Default constructor. Sets everything to dummy or NULL values. */
	send_packet_event();

	/**
	 * Constructor for lone packets--window size is set to 1 and window_start
	 * is set to this packet's sequence number.
	 * @param time time at which to send the packet
	 * @param sim
	 * @param flow flow to which this packet belongs
	 * @param pkt packet to send
	 * @param link to send the packet on
	 * @param departure_node node from which this packet will leave
	 */
	send_packet_event(double time, simulation &sim, netflow &flow,
			packet &pkt, netlink &link, netnode &departure_node);

	/**
	 * Constructor for packets that do not belong to a flow (i.e., routing 
	 * packets).
	 * @param time time at which to send the packet
	 * @param sim
	 * @param pkt packet to send
	 * @param link to send the packet on
	 * @param departure_node node from which this packet will leave
	 */
	send_packet_event(double time, simulation &sim, packet &pkt, 
			netlink &link, netnode &departure_node);

	/** Destructor */
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
	 * @param time Time when to start the flow
	 * @param sim
	 * @param flow Flow to start
	 */
	start_flow_event(double time, simulation &sim, netflow &flow);

	/** Destructor. */
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
 * @deprecated because timeouts aren't currently being used
 */
class timeout_event : public event {

private:

	/** Flow to which to register a timeout. */
	netflow *flow;

	/** Sequence number that timed out. */
	int seqnum;

public:

	/** Default constructor, sets everything to dummy or NULL. */
	timeout_event();

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * and sets the flow to which this timeout_event belongs.
	 *
	 * @param time
	 * @param sim
	 * @param flow
	 * @param seqnum
	 */
	timeout_event(double time, simulation &sim, netflow &flow, int seqnum);

	/** Destructor. */
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

	/** Default constructor, sets everything to dummy or NULL. */
	ack_event();

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * and sets the flow to which this ack_event belongs.
	 * @param time
	 * @param sim
	 * @param flow
	 * @param dup_pkt
	 */
	ack_event(double time, simulation &sim,
			netflow &flow, packet &dup_pkt);

	/** Destructor. */
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
