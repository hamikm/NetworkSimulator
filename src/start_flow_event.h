/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef START_FLOW_EVENT_H
#define START_FLOW_EVENT_H

#include <iostream>
#include "netflow.h"
#include "netlink.h"
#include "event.h"
#include "simulation.h"

using namespace std;

extern bool debug;
extern ostream &debug_os;

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
	start_flow_event(double time, simulation &sim, netflow &flow) :
		event(time, sim), flow(&flow) { }

	~start_flow_event() { }

	/**
	 * Runs the Bellman-Ford algorithm from this router.
	 */
	void runEvent() {

		// TODO

		if(debug) {
			debug_os << "STARTING: " << *this << endl;
		}
	}

	/**
	 * Print helper function.
	 * @param os The output stream to which to write event information.
	 */
	void printHelper(ostream &os) const {
		event::printHelper(os);
		os << " --> [start_flow_event. flow: " << endl
				<< "  " << *flow << endl << "]";
	}
};

#endif // START_FLOW_EVENT_H
