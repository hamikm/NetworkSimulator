/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef EVENTS_H
#define EVENTS_H

// Standard includes.
#include <iostream>

// Forward declarations.
class router_discovery_event;
class start_flow_event;
class send_packet_event;
class receive_packet_event;
class timeout_event;
class simulation;

// Custom headers.
#include "simulation.h"
#include "network.h"

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
	 * logging data in this function.
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
	packet *pkt;

public:

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * sets the flow from which this packet originates, and sets the packet.
	 */
	receive_packet_event(double time, simulation &sim,
			netflow &flow, packet &pkt);

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

	/**
	 * Flow to which the packet is being sent.
	 */
	netflow *flow;

	/**
	 * Packet being sent.
	 */
	packet *pkt;

public:

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * sets the flow from which this packet originates, and sets the packet.
	 */
	send_packet_event(double time, simulation &sim,
			netflow &flow, packet &pkt);

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
 * Event that runs when a flow is about to start. Initializes a flow's state
 * variables and queues a timeout event, since a timeout just takes the window
 * size to one then sends a packet. TODO bring in line with document on gdriv
 */
class start_flow_event : public event {

private:

	/** Flow that we're going to start. */
	netflow *flow;

public:

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * and sets the router from which this router discovery event should run.
	 */
	start_flow_event(double time, simulation &sim, netflow &flow);

	~start_flow_event();

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

// ---------------------------- timeout_event class ---------------------------

/**
 * Event that takes the window size to one then sends a packet by trying to
 * queue a packet arrival event; if the link buffer can't support the
 * additional packet the packet gets dropped though. This event also queues
 * another timeout event.
 */
class timeout_event : public event {

private:

	/**
	 * Flow whose window size will change and from which a packet will be
	 * sent.
	 */
	netflow *flow;

public:

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * and sets the router from which this router discovery event should run.
	 */
	timeout_event(double time, simulation &sim, netflow &flow);

	~timeout_event();

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

#endif // EVENTS_H
