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

void event::runEvent() { }

double event::getTime() const { return time; }

long event::getId() const { return id; }

void event::printHelper(ostream &os) {
	os << "event. id: " << id << ", time: " << time << " ";
}

// ------------------------- receive_packet_event class -----------------------

receive_packet_event::receive_packet_event(double time, simulation &sim,
			netflow &flow, packet &pkt, netnode &step_destination,
			netlink &link) :
		event(time, sim), flow(&flow), pkt(pkt),
		step_destination(&step_destination), link(&link) { }

receive_packet_event::~receive_packet_event() { }

void receive_packet_event::runEvent() {

	if(debug) {

		debug_os << "RECEIVING " << pkt.getTypeString() << " PACKET: "
				<< *this << endl;
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
			send_packet_event *e = new send_packet_event(
					getTime(), *sim, *flow, pkt, *(it->first),
					*step_destination);
			sim->addEvent(e);
			it++;
		}
	}

	/*
	 * If we have a FLOW packet arriving at a host then we create an ACK packet,
	 * make and queue a ack_event to send the first ACK, and queue a
	 * future ack_event in case the correct successor packet never
	 * arrives. We also remove the flow's pending ack_events.
	 */
	else if (pkt.getType() == FLOW) {
		flow->receivedFlowPacket(pkt, getTime());
	}
	/*
	 * If we're an ACK packet arriving at a source then we need to trigger the
	 * ACK-handling behavior in the flow class and send a new window-load
	 * of FLOW packets.
	 */
	else if (pkt.getType() == ACK) {
		flow->receivedAck(pkt, getTime());

		// Get the current window's packet(s) to send.
		vector<packet> pkts_to_send = flow->popOutstandingPackets(getTime());

		// TODO refactor following into the receivedAck method.

		// Iterate over the packets to send, making send_packet_events for
		// each. The timout_events have already been added to the flow and to
		// the simulation's queue.
		vector<packet>::iterator pkt_it = pkts_to_send.begin();
		while(pkt_it != pkts_to_send.end()) {
			send_packet_event *e = new send_packet_event(getTime(), *sim,
					*flow, *pkt_it, *(flow->getSource()->getLink()),
					*(flow->getSource()));
			sim->addEvent(e);
			pkt_it++;
		}
	}
	else if (pkt.getType() == ROUTING) {
		// should have been handled by the "am at a router" condition
		assert(false);
	}
	else {
		assert(false);
	}

	// No matter what the packet type or node type we need to tell the link
	// that we're done using it.
	link->receivedPacket(pkt.getId());
}

void receive_packet_event::printHelper(ostream &os) {
	event::printHelper(os);

	flow->setNestingDepth(1);
	link->setNestingDepth(1);
	pkt.setNestingDepth(1);

	os << "<-- receive_packet_event. {" << endl <<
			"  flow: " << *flow << endl <<
			"  packet: " << pkt << endl <<
			"  link: " << *link << endl << "}";

	flow->setNestingDepth(0);
	link->setNestingDepth(0);
	pkt.setNestingDepth(0);
}

// ------------------------- router_discovery_event class ---------------------

router_discovery_event::router_discovery_event(
		double time, simulation &sim, netrouter &router) :
				event(time, sim), router(&router) { }

router_discovery_event::~router_discovery_event() { }

void router_discovery_event::runEvent() {

	if(debug) {
		debug_os << "ROUTING: " << *this << endl;
	}
	// TODO
}

void router_discovery_event::printHelper(ostream &os) {
	event::printHelper(os);

	router->setNestingDepth(1);

	os << "<-- router_discovery_event. {" << endl <<
			"  router: " << *router << endl << "}";

	router->setNestingDepth(0);
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
		debug_os << "SENDING " << pkt.getTypeString() << " PACKET: "
				<< *this << endl;
	}

	// Find (absolute) arrival time to the next node from the given departure
	// node down the given link.
	double arrival_time = link->getWaitTimeIntervalMs() +
			link->getTransmissionTimeMs(pkt) + getTime();

	// Use the arrival time to queue a receive_packet_event (does nothing if
	// the link buffer has no room, thereby dropping the packet).
	if (link->sendPacket(pkt)) {
		receive_packet_event *e = new receive_packet_event(arrival_time, *sim,
				*flow, pkt, *getDestinationNode(), *link);
		sim->addEvent(e);
	}
	else { // packet was dropped

		if (debug) {
			debug_os << "This packet was DROPPED: " << pkt << endl;
		}
		// do nothing, don't make or queue a receive_packet_event
	}

}

void send_packet_event::printHelper(ostream &os) {
	event::printHelper(os);

	flow->setNestingDepth(1);
	link->setNestingDepth(1);
	pkt.setNestingDepth(1);

	os << "<-- send_packet_event. {" << endl <<
			"  flow: " << *flow << endl <<
			"  packet: " << pkt << endl <<
			"  link: " << *link << endl << "}";

	flow->setNestingDepth(0);
	link->setNestingDepth(0);
	pkt.setNestingDepth(0);
}

// --------------------------- start_flow_event class -------------------------

start_flow_event::start_flow_event(
		double time, simulation &sim, netflow &flow) :
				event(time, sim), flow(&flow) { }

start_flow_event::~start_flow_event() { }

void start_flow_event::runEvent() {

	if(debug) {
		debug_os << "STARTING FLOW: " << *this << endl;
	}

	// Get the current (i.e. the first) window's packet(s) to send.
	vector<packet> pkts_to_send = flow->popOutstandingPackets(getTime());

	// Iterate over the packets to send, making a send_packet_event for each.
	// The timout_events have already been added to the flow and to the
	// simulation's queue.
	vector<packet>::iterator pkt_it = pkts_to_send.begin();
	while(pkt_it != pkts_to_send.end()) {
		send_packet_event *e = new send_packet_event(getTime(), *sim, *flow,
				*pkt_it, *(flow->getSource()->getLink()),
				*(flow->getSource()));
		sim->addEvent(e);
		pkt_it++;
	}
}

void start_flow_event::printHelper(ostream &os) {
	event::printHelper(os);
	os << "<-- start_flow_event {" << endl <<
			"  flow: " << *flow << endl << "}";
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
		debug_os << "TIMEOUT TRIGGERED: " << *this << endl;
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
		send_packet_event *e = new send_packet_event(getTime(), *sim, *flow,
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
void timeout_event::printHelper(ostream &os) {
	event::printHelper(os);

	flow->setNestingDepth(1);
	timedout_pkt.setNestingDepth(1);

	os << "<-- timeout_event. {" << endl <<
			"  timedout_pkt: " << timedout_pkt << endl <<
			"  flow: " << *flow << endl << "}";

	flow->setNestingDepth(0);
	timedout_pkt.setNestingDepth(0);
}

// ------------------------- ack_event class ------------------------

ack_event::ack_event() :
		event(), flow(NULL), dup_pkt(packet()) { }

ack_event::ack_event(double time, simulation &sim,
		netflow &flow, packet &dup_pkt) :
				event(time, sim), flow(&flow), dup_pkt(dup_pkt) { }

ack_event::~ack_event() { }

void ack_event::runEvent() {

	if(debug) {
		debug_os << "ACK TRIGGERED: " << *this << endl;
	}

	// Make and queue the ACK's send_packet_event.
	send_packet_event *e = new send_packet_event(getTime(), *sim, *flow,
			dup_pkt, *(flow->getDestination()->getLink()),
			*(flow->getDestination()));
	sim->addEvent(e);

	// N.B. we wait the same amount of time to send a duplicate ACK as we do
	// a timeout_event, since the destination can just perform the same
	// computations as the source.
	flow->registerSendDuplicateAckAction(dup_pkt.getSeq(),
			getTime() + flow->getTimeoutLengthMs());
}

void ack_event::printHelper(ostream &os) {
	event::printHelper(os);

	flow->setNestingDepth(1);
	dup_pkt.setNestingDepth(1);

	os << "<-- ack_event. {" << endl <<
			"  ack_pkt: " << dup_pkt << endl <<
			"  flow: " << *flow << endl << "}";

	flow->setNestingDepth(0);
	dup_pkt.setNestingDepth(0);
}
