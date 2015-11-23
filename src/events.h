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
class simulation;
class eventTimeSorter;

using namespace std;

extern bool debug;
extern ostream &debug_os;

// ---------------------- event class and event sorter ------------------------

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
	 * sending packets, adding new events to the simulation event queue, and
	 * logging data in this method.
	 */
	virtual void runEvent();

	/**
	 * Print helper function. Derived classes should (partially) override this.
	 * @param os The output stream to which to write event information.
	 */
	virtual void printHelper(ostream &os) const;
};

/**
 * Output operator override for printing contents of the given event
 * to an output stream. Uses the printHelper function, which is virtual
 * because derived classes will want to modify or enhance printing behavior.
 * @param os The output stream to which to write.
 * @param device The @c event object to write.
 * @return The same output stream for operator chaining.
 */
inline ostream & operator<<(ostream &os, const event &e) {
	e.printHelper(os);
	return os;
}

// --------------------------- receive_packet_event class ---------------------

/**
 * TODO
 */
class receive_packet_event : public event {

private:

	/**
	 * Flow to which the received packet belongs.
	 */
	netflow *flow;

	/**
	 * Packet received by the flow.
	 */
	packet pkt;

	/**
	 * Host or router that is receiving the packet.
	 */
	netnode *step_destination;

public:

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * sets the flow from which this packet originates, and sets the packet.
	 */
	receive_packet_event(double time, simulation &sim,
			netflow &flow, packet &pkt, netnode &step_destination);

	~receive_packet_event();

	/**
	 * TODO
	 */
	void runEvent();

	/**
	 * Print helper function.
	 * @param os The output stream to which to write event information.
	 */
	void printHelper(ostream &os) const;
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
	 * Initializes this event's time to the given one, sets the event ID,
	 * and sets the router from which this router discovery event should run.
	 */
	router_discovery_event(double time, simulation &sim, netrouter &router);

	~router_discovery_event();

	/**
	 * Runs the Bellman-Ford algorithm from this router.
	 */
	void runEvent();

	/**
	 * Print helper function.
	 * @param os The output stream to which to write event information.
	 */
	void printHelper(ostream &os) const;
};

// --------------------------- send_packet_event class ------------------------

/**
 * TODO
 */
class send_packet_event : public event {

private:

	/** Parent flow of this packet, null if ROUTING type. */
	netflow *flow;

	/** The packet being sent. */
	packet pkt;

	/** The link being used in this leg of the packet's journey. */
	netlink *link;

public:

	send_packet_event();

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * sets the flow from which this packet originates, and sets the packet.
	 */
	send_packet_event(double time, simulation &sim,
			netflow &flow, packet &pkt, netlink &link);

	~send_packet_event();

	/**
	 * TODO
	 */
	void runEvent();

	/**
	 * Print helper function.
	 * @param os The output stream to which to write event information.
	 */
	void printHelper(ostream &os) const;
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
	void printHelper(ostream &os) const;
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

	/** The packet that timed out. */
	packet timedout_pkt;

public:

	timeout_event();

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * and the flow to which this timeout_event belongs.
	 */
	timeout_event(double time, simulation &sim, netflow &flow, packet &to_pkt);

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
	void printHelper(ostream &os) const;
};

#endif // EVENTS_H
