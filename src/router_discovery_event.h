/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef ROUTER_DISCOVERY_EVENT_H
#define ROUTER_DISCOVERY_EVENT_H

#include <iostream>
#include "netrouter.h"
#include "event.h"

using namespace std;

extern bool debug;
extern ostream &debug_os;

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
	router_discovery_event(double time, netrouter &router) :
		event(time), router(&router) { }

	~router_discovery_event() { }

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
		os << " --> [router_discovery_event. router: " << endl
				<< "  " << *router << endl << "]";
	}
};

#endif // ROUTER_DISCOVERY_EVENT_H
