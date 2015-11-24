/*
 * See header file for comments.
 */

// Custom headers.
#include "events.h"
#include "simulation.h"
#include "network.h"

// -------------------------------- event class -------------------------------

long event::id_generator = 1;

event::event() : time(-1), id(-1), sim(NULL) { }

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
		event(time, sim), flow(&flow), pkt(pkt),
		step_destination(&step_destination) { }

receive_packet_event::~receive_packet_event() { }

void receive_packet_event::runEvent() {

	if(debug) {
		debug_os << "STARTING: " << *this << endl;
	}

	/*
	 * If this arrival event is at a router then forward the packet. The
	 * router class takes care of that by consulting the routing
	 * table for the link, then this function uses it to generate a
	 * send_packet_event down that link.
	 *
	 * TODO This likely works for ACK, FLOW, as well as ROUTING packets, since
	 * the receivePacket function just returns all the packets to send and
	 * corresponding links to follow.
	 */
	if (step_destination->isRoutingNode()) {
		netrouter *router = dynamic_cast<netrouter *>(step_destination);
		map<netlink *, packet> link_pkt_map =
				router->receivePacket(getTime(), *sim, *flow, pkt);

		// Iterate over all the packets that must be sent, making a
		// send packet event for each.
		map<netlink *, packet>::iterator it = link_pkt_map.begin();
		while (it != link_pkt_map.end()) {
			send_packet_event e(getTime(), *sim, *flow, pkt, *(it->first),
					*step_destination);
			sim->addEvent(e);
			it++;
		}
	}

	/*
	 * If the packet is arriving at a host then we create an ACK packet, make
	 * and queue a duplicate_ack_event to send the first ACK, and queue a
	 * future duplicate_ack_event in case the correct successor packet never
	 * arrives. We also remove the flow's pending duplicate_ack_events.
	 */
	else {
		flow->receivedFlowPacket(pkt, getTime());
	}
}

void receive_packet_event::printHelper(ostream &os) const {
	event::printHelper(os);
	os << " --> [receive_packet_event. flow: " << endl
			<< "  " << *flow << endl << ", packet: " << endl
			<< "  " << pkt << endl << "]";
}

// ------------------------- router_discovery_event class ---------------------

router_discovery_event::router_discovery_event(
		double time, simulation &sim, netrouter &router) :
				event(time, sim), router(&router) { }

router_discovery_event::~router_discovery_event() { }

void router_discovery_event::runEvent() {

	if(debug) {
		debug_os << "STARTING: " << *this << endl;
	}
	// TODO
}

void router_discovery_event::printHelper(ostream &os) const {
	event::printHelper(os);
	os << " --> [router_discovery_event. router: " << endl
			<< "  " << *router << endl << "]";
}

// -------------------------- send_packet_event class -------------------------

send_packet_event::send_packet_event() :
		event(), flow(NULL), link(NULL), departure_node(NULL) { }

send_packet_event::send_packet_event(double time, simulation &sim,
		netflow &flow, packet &pkt, netlink &link, netnode &departure_node) :
				event(time, sim), flow(&flow), pkt(pkt), link(&link),
				departure_node(&departure_node) {

	// Make sure the given departure node matches one of the endpoints of the
	// given link.
	assert((strcmp(this->departure_node->getName().c_str(),
				   this->link->getEndpoint1()->getName().c_str()) == 0) ||
		   (strcmp(this->departure_node->getName().c_str(),
				   this->link->getEndpoint2()->getName().c_str()) == 0));
}

send_packet_event::~send_packet_event() { }

netnode *send_packet_event::getDestinationNode() const {

	if (strcmp(departure_node->getName().c_str(),
			link->getEndpoint1()->getName().c_str()) == 0) {
		return link->getEndpoint2();
	}
	else if (strcmp(departure_node->getName().c_str(),
			link->getEndpoint2()->getName().c_str()) == 0) {
		return link->getEndpoint1();
	}
	else {
		assert(false);
	}

	return NULL;
}

void send_packet_event::runEvent() {

	if(debug) {
		debug_os << "STARTING: " << *this << endl;
	}

	// Find (absolute) arrival time to the next node from the given departure
	// node down the given link.
	double arrival_time = link->getWaitTimeIntervalMs() + getTime();

	// Use the arrival time to queue a receive_packet_event (does nothing if
	// the link buffer has no room, thereby dropping the packet).
	if (link->sendPacket(pkt)) {
		receive_packet_event e(arrival_time, *sim, *flow, pkt,
				*getDestinationNode());
		sim->addEvent(e);
	}
	else { // packet was dropped

		if (debug) {
			debug_os << "This packet was DROPPED: " << pkt << endl;
		}
		// do nothing, don't make or queue a receive_packet_event
	}

}

void send_packet_event::printHelper(ostream &os) const {
	event::printHelper(os);
	os << " --> [send_packet_event. flow: " << endl
			<< "  " << *flow << endl << ", packet: " << endl
			<< "  " << pkt << endl << "]";
}

// --------------------------- start_flow_event class -------------------------

start_flow_event::start_flow_event(
		double time, simulation &sim, netflow &flow) :
				event(time, sim), flow(&flow) { }

start_flow_event::~start_flow_event() { }

void start_flow_event::runEvent() {

	if(debug) {
		debug_os << "STARTING: " << *this << endl;
	}

	// Get the current (i.e. the first) window's packet(s) to send.
	vector<packet> pkts_to_send = flow->popOutstandingPackets(getTime());

	// Iterate over the packets to send, making a send_packet_event for each.
	// The timout_events have already been added to the flow and to the
	// simulation's queue.
	vector<packet>::iterator pkt_it = pkts_to_send.begin();
	while(pkt_it != pkts_to_send.end()) {
		nethost *starting_host = flow->getSource();
		send_packet_event e(getTime(), *sim, *flow,
				*pkt_it, *starting_host->getLink(), *starting_host);
		sim->addEvent(e);
		pkt_it++;
	}
}

void start_flow_event::printHelper(ostream &os) const {
	event::printHelper(os);
	os << " --> [start_flow_event. flow: " << endl
			<< "  " << *flow << endl << "]";
}

// ----------------------------- timeout_event class --------------------------

timeout_event::timeout_event() :
		event(), flow(NULL), timedout_pkt(packet()) { }

timeout_event::timeout_event(
		double time, simulation &sim, netflow &flow, packet &to_pkt) :
				event(time, sim), flow(&flow), timedout_pkt(to_pkt) { }

timeout_event::~timeout_event() { }

void timeout_event::runEvent() {

	if(debug) {
		debug_os << "STARTING: " << *this << endl;
	}

	// Resize the window, set the linear growth threshold, cancel the
	// corresponding timeout locally (this event running means it was
	// dequeued in the events queue).
	flow->timeoutOccurred(timedout_pkt);

	// Now send the timed out packet again.
	vector<packet> pkts_to_send =
			flow->popOutstandingPackets(getTime()); // queues new timeout too

	// Iterate over the packets to send, making a send_packet_event for each.
	// The timout_events have already been added to the flow and to the
	// simulation's queue.
	vector<packet>::iterator pkt_it = pkts_to_send.begin();
	while(pkt_it != pkts_to_send.end()) {
		send_packet_event e(getTime(), *sim, *flow,
				*pkt_it, *(flow->getSource()->getLink()),
				*(flow->getSource()));
		sim->addEvent(e);
		pkt_it++;
	}
}

/**
 * Print helper function.
 * @param os The output stream to which to write event information.
 */
void timeout_event::printHelper(ostream &os) const {
	event::printHelper(os);
	os << " --> [timeout_event. timedout_pkt: " << timedout_pkt <<
			", flow: " << endl << "  " << *flow << endl << "]";
}

// ------------------------- duplicate_ack_event class ------------------------

duplicate_ack_event::duplicate_ack_event() :
		event(), flow(NULL), dup_pkt(packet()) { }

duplicate_ack_event::duplicate_ack_event(double time, simulation &sim,
		netflow &flow, packet &dup_pkt) :
				event(time, sim), flow(&flow), dup_pkt(dup_pkt) { }

duplicate_ack_event::~duplicate_ack_event() { }

void duplicate_ack_event::runEvent() {

	if(debug) {
		debug_os << "STARTING: " << *this << endl;
	}

	// Make and queue the ACK's send_packet_event.
	send_packet_event e(getTime(), *sim, *flow, dup_pkt,
			*(flow->getDestination()->getLink()), *(flow->getDestination()));
	sim->addEvent(e);

	// N.B. we wait the same amount of time to send a duplicate ACK as we do
	// a timeout_event, since the destination can just perform the same
	// computations as the source.
	flow->registerSendDuplicateAckAction(dup_pkt.getSeq(),
			getTime() + flow->getTimeoutLengthMs());
}

void duplicate_ack_event::printHelper(ostream &os) const {
	event::printHelper(os);
	os << " --> [duplicate_ack_event. timedout_pkt: " << dup_pkt <<
			", flow: " << endl << "  " << *flow << endl << "]";
}
