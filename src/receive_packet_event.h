/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef RECEIVE_PACKET_EVENT_H
#define RECEIVE_PACKET_EVENT_H

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
			netflow &flow, packet &pkt) :
		event(time, sim), flow(&flow), pkt(&pkt) { }

	~receive_packet_event() { }

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
		os << " --> [receive_packet_event. flow: " << endl
				<< "  " << *flow << endl << ", packet: " << endl
				<< "  " << *pkt << endl << "]";
	}
};

#endif // RECEIVE_PACKET_EVENT_H
