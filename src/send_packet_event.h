/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef SEND_PACKET_EVENT_H
#define SEND_PACKET_EVENT_H

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
			netflow &flow, packet &pkt) :
		event(time, sim), flow(&flow), pkt(&pkt) { }

	~send_packet_event() { }

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
		os << " --> [send_packet_event. flow: " << endl
				<< "  " << *flow << endl << ", packet: " << endl
				<< "  " << *pkt << endl << "]";
	}
};

#endif // SEND_PACKET_EVENT_H
