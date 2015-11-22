/*
 * See header file for comments.
 */

#include "events.h"

// -------------------------------- event class -------------------------------

long event::id_generator = 1;

event::event(double time, simulation &sim) :
		time(time), id(id_generator++), sim(&sim) { }

event::~event() {}

double event::getTime() const { return time; }

long event::getId() const { return id; }

void event::runEvent() {}

void event::printHelper(ostream &os) const {
	os << "[event. id: " << id << ", time: " << time << "]";
}

// ------------------------- receive_packet_event class -----------------------

receive_packet_event::receive_packet_event(double time, simulation &sim,
			netflow &flow, packet &pkt, netnode &step_destination) :
		event(time, sim), flow(&flow), pkt(&pkt),
		step_destination(&step_destination) { }

receive_packet_event::~receive_packet_event() { }

void receive_packet_event::runEvent() {

	step_destination->receivePacket(getTime(), *sim, *flow, *pkt);

	if(debug) {
		debug_os << "STARTING: " << *this << endl;
	}
}

void receive_packet_event::printHelper(ostream &os) const {
	event::printHelper(os);
	os << " --> [receive_packet_event. flow: " << endl
			<< "  " << *flow << endl << ", packet: " << endl
			<< "  " << *pkt << endl << "]";
}

// ------------------------- router_discovery_event class ---------------------

router_discovery_event::router_discovery_event(
		double time, simulation &sim, netrouter &router) :
				event(time, sim), router(&router) { }

router_discovery_event::~router_discovery_event() { }

void router_discovery_event::runEvent() {

	// TODO

	if(debug) {
		debug_os << "STARTING: " << *this << endl;
	}
}

void router_discovery_event::printHelper(ostream &os) const {
	event::printHelper(os);
	os << " --> [router_discovery_event. router: " << endl
			<< "  " << *router << endl << "]";
}

// -------------------------- send_packet_event class -------------------------

send_packet_event::send_packet_event(double time, simulation &sim,
		netflow &flow, packet &pkt) :
	event(time, sim), flow(&flow), pkt(&pkt) { }

send_packet_event::~send_packet_event() { }

void send_packet_event::runEvent() {

	// TODO

	if(debug) {
		debug_os << "STARTING: " << *this << endl;
	}
}

void send_packet_event::printHelper(ostream &os) const {
	event::printHelper(os);
	os << " --> [send_packet_event. flow: " << endl
			<< "  " << *flow << endl << ", packet: " << endl
			<< "  " << *pkt << endl << "]";
}

// --------------------------- start_flow_event class -------------------------

start_flow_event::start_flow_event(double time, simulation &sim, netflow &flow) :
	event(time, sim), flow(&flow) { }

start_flow_event::~start_flow_event() { }

void start_flow_event::runEvent() {

	// TODO

	if(debug) {
		debug_os << "STARTING: " << *this << endl;
	}
}

void start_flow_event::printHelper(ostream &os) const {
	event::printHelper(os);
	os << " --> [start_flow_event. flow: " << endl
			<< "  " << *flow << endl << "]";
}

// ----------------------------- timeout_event class --------------------------

timeout_event::timeout_event(double time, simulation &sim, netflow &flow) :
	event(time, sim), flow(&flow) { }

timeout_event::~timeout_event() { }

/**
 * Runs the Bellman-Ford algorithm from this router.
 */
void timeout_event::runEvent() {

	// TODO

	if(debug) {
		debug_os << "STARTING: " << *this << endl;
	}
}

/**
 * Print helper function.
 * @param os The output stream to which to write event information.
 */
void timeout_event::printHelper(ostream &os) const {
	event::printHelper(os);
	os << " --> [timeout_event. flow: " << endl
			<< "  " << *flow << endl << "]";
}
