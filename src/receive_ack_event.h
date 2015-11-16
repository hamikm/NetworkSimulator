/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef RECEIVE_ACK_EVENT_H
#define RECEIVE_ACK_EVENT_H

#include <iostream>
#include "netflow.h"
#include "netlink.h"
#include "event.h"
#include "simulation.h"
#include "packet.h"

using namespace std;

extern bool debug;
extern ostream &debug_os;

/**
 * TODO
 */
class receive_ack_event : public event {

private:

	/**
	 * Flow from which the ACK is coming.
	 */
	netflow *flow;

	/**
	 * The ACK packet itself.
	 */
	packet *pkt;

public:

	/**
	 * Initializes this event's time to the given one, sets the event ID,
	 * sets the flow from which this packet originates, and sets the packet.
	 */
	receive_ack_event(double time, simulation &sim,
			netflow &flow, packet &pkt) :
		event(time, sim), flow(&flow), pkt(&pkt) { }

	~receive_ack_event() { }

	/**
	 * TODO
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
		os << " --> [receive_ack_event. flow: " << endl
				<< "  " << *flow << endl << ", packet: " << endl
				<< "  " << *pkt << endl << "]";
	}
};

#endif // RECEIVE_ACK_EVENT_H
