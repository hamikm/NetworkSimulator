/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef TIMEOUT_EVENT_H
#define TIMEOUT_EVENT_H

#include <iostream>
#include "netflow.h"
#include "event.h"
#include "simulation.h"

using namespace std;

extern bool debug;
extern ostream &debug_os;

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
	timeout_event(double time, simulation &sim, netflow &flow) :
		event(time, sim), flow(&flow) { }

	~timeout_event() { }

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
		os << " --> [timeout_event. flow: " << endl
				<< "  " << *flow << endl << "]";
	}
};

#endif // TIMEOUT_EVENT_H
