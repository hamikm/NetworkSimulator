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

void receive_packet_event::constructorHelper(netflow *flow, packet &pkt,
			netnode *step_destination, netlink *link,
			int window_size, int window_start) {
	this->flow = flow;
	this->pkt = pkt;
	this->step_destination = step_destination;
	this->link = link;
	this->window_size = window_size;
	this->window_start = window_start;
}

receive_packet_event::receive_packet_event(double time, simulation &sim,
			netflow &flow, packet &pkt, netnode &step_destination,
			netlink &link) : event(time, sim) {
	constructorHelper(&flow, pkt, &step_destination, &link, 1, pkt.getSeq());
}

receive_packet_event::receive_packet_event(double time, simulation &sim, 
			packet &pkt, netnode &step_destination, netlink &link) {
	constructorHelper(NULL, pkt, &step_destination, &link, 1, 
		SEQNUM_FOR_NONFLOWS);
}

receive_packet_event::receive_packet_event(double time, simulation &sim,
		netflow &flow, packet &pkt, netnode &step_destination,
		netlink &link, int window_size, int window_start) : event(time, sim) {
	constructorHelper(&flow, pkt, &step_destination, &link,
			window_size, window_start);
}

receive_packet_event::~receive_packet_event() { }

void receive_packet_event::runEvent() {
	

	if(debug) {
		debug_os << getTime() << "\tRECEIVING " << pkt.getTypeString()
				<< " PACKET: " << pkt.getSeq() << endl;
				// TODO: figure out why uncommenting this causes segfault
				//<< *this << endl;
		debug_os << "Before receipt: ";
		link->printBuffer(debug_os);
	}

	// update link traffic used to calculate link rate
	double time = getTime();
	link->updateLinkTraffic(time, pkt.getType());

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
		//cout << "REACHED RECEIVED 2" << endl;

		if (pkt.getType() == ROUTING) {
			// Handle routing packets differently here
			// New send_packet_event(s) will only be triggered within the 
			// router's receiveRoutingPacket method if there are any updates
			// made to the router table/distances.

			router->receiveRoutingPacket(getTime(), *sim, pkt, *link);

		}

		else {
			// FLOW and ACK packets are handled the same way here:
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
	}

	/*
	 * If we have a FLOW packet arriving at a host then we create an ACK,
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
		flow->receivedAck(pkt, getTime(), link->getLinkFreeAtTime());

		// TODO refactor following into the receivedAck method.

		if(debug && this) {
			debug_os << "Got ACK #" << pkt.getSeq() << endl;
		}

		// Get the current window's packet(s) to send.
		double linkFreeAt = flow->getSource()->getLink()->getLinkFreeAtTime();
		vector<packet> pkts_to_send =
				flow->popOutstandingPackets(getTime(),
						linkFreeAt == 0 ? getTime() : linkFreeAt);

		// Iterate over the packets to send, making send_packet_events for
		// each. The timout_events have already been added to the flow and to
		// the simulation's queue.
		int window_size = pkts_to_send.size();

		if (debug) {
			cout << "Num packets to send: " <<  pkts_to_send.size() << endl;
		}

		int first_seqnum_in_window = pkts_to_send.size() == 0 ? -1 :
				pkts_to_send[0].getSeq();
		vector<packet>::iterator pkt_it = pkts_to_send.begin();
		int i = 0;
		while(pkt_it != pkts_to_send.end()) {

			if(debug && this) {
				debug_os << "  Sending packet #" << pkt_it->getSeq() << endl;
			}

			send_packet_event *e = new send_packet_event(
					getTime() + i++ * netflow::TIME_EPSILON, *sim,
					*flow, *pkt_it, *(flow->getSource()->getLink()),
					*(flow->getSource()), window_size, first_seqnum_in_window);
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
	if (debug) {
		debug_os << "Removing " << pkt.getTypeString() << " packet "
				<< pkt.getSeq() << " from buffer" << endl;
	}

	if(!link->receivedPacket(pkt.getId()) && debug) {
		debug_os << "ERROR: packet at front of buffer wasn't the same as the"
				" one received." << endl;
	}

	// log data
	double currTime = getTime();
	sim->logEvent(currTime);
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
		double time, simulation &sim) :
				event(time, sim) { }

router_discovery_event::~router_discovery_event() { }

void router_discovery_event::runEvent() {

	if(debug && this) {
		debug_os << "ROUTING: " << *this << endl;
	}

	// Reset each router's distance table
	map<string, netrouter *> router_list = sim->getRouters();
	for (map<string, netrouter *>::iterator it = router_list.begin();
		 it != router_list.end(); it++) {
		it->second->resetDistances(sim->getHosts(), sim->getRouters());
	}

	// Have each router send packets to its neighbors
	for (map<string, netrouter *>::iterator it = router_list.begin();
		 it != router_list.end(); it++) {

		netrouter *r = it->second;

		vector<netlink *> adj_links = r->getLinks();
		for (unsigned i = 0; i < adj_links.size(); i++) {

			netnode *other_node = r->getOtherNode(adj_links[i]);

			// Check if other_node points to router
			if (other_node->isRoutingNode()) {

				packet rpack = packet(ROUTING, r->getName(), other_node->getName());
				rpack.setDistances(r->getRDistances()); 
				rpack.setTransmitTimestamp(getTime());

				// Queue up new packet. Send_packet_event will check when the
				// link is free.
				send_packet_event *e = new send_packet_event(getTime(), *sim,
					rpack, *adj_links[i], *r);
				sim->addEvent(e);

			}

		}
	}

}

void router_discovery_event::printHelper(ostream &os) {
	event::printHelper(os);

	router->setNestingDepth(1);

	os << "<-- router_discovery_event. {" << endl <<
			"  router: " << *router << endl << "}";

	router->setNestingDepth(0);
}

// --------------------------- update_window_event class ------------------------

update_window_event::update_window_event(
		double time, simulation &sim, netflow &flow) :
				event(time, sim) { 
	this->flow = &flow;
}

update_window_event::~update_window_event() { }

void update_window_event::runEvent() {
	double w = flow->getWindowSize();
	double new_windowsize = min(2*w, (1.0 - GAMMA) * w + 
		GAMMA * (flow->getMinRTT()*w/flow->getAvgRTT() + ALPHA));

	flow->setFASTWindowSize(new_windowsize);

}

void update_window_event::printHelper(ostream &os) {
	event::printHelper(os);

	flow->setNestingDepth(1);

	os << "<-- update_window_event. {" << endl <<
			"  flow: " << *flow << endl << "}";

	flow->setNestingDepth(0);
}

// -------------------------- send_packet_event class -------------------------

void send_packet_event::constructorHelper(netflow *flow, packet &pkt,
		netlink *link, netnode *departure_node,
		int window_size, int window_start) {
	this->flow = flow;
	this->pkt = pkt;
	this->link = link;
	this->departure_node = departure_node;
	this->window_size = window_size;
	this->window_start = window_start;

	// Make sure the given departure node matches one of the endpoints of the
	// given link.
	assert((strcmp(this->departure_node->getName().c_str(),
				   this->link->getEndpoint1()->getName().c_str()) == 0) ||
		   (strcmp(this->departure_node->getName().c_str(),
				   this->link->getEndpoint2()->getName().c_str()) == 0));
}

send_packet_event::send_packet_event() : event() {
	packet pkt;
	constructorHelper(NULL, pkt, NULL, NULL, -1, -1);
}

send_packet_event::send_packet_event(double time, simulation &sim, 
			packet &pkt, netlink &link, netnode &departure_node) : 
					event(time, sim) {
	constructorHelper(NULL, pkt, &link, &departure_node, 1, 
		SEQNUM_FOR_NONFLOWS);
}

send_packet_event::send_packet_event(double time, simulation &sim,
		netflow &flow, packet &pkt, netlink &link, netnode &departure_node) :
				event(time, sim) {
	constructorHelper(&flow, pkt, &link, &departure_node, 1, pkt.getSeq());
}

send_packet_event::send_packet_event(double time, simulation &sim,
		netflow &flow, packet &pkt, netlink &link, netnode &departure_node,
		int window_size, int window_start) : event(time, sim) {
	constructorHelper(&flow, pkt, &link, &departure_node,
			window_size, window_start);
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

	if(debug && this) {
		debug_os << getTime() << "\tSENDING " << pkt.getTypeString()
				<< " PACKET: "
				<< pkt.getSeq() << ", win size: " << window_size
				<< ", window start: " << window_start << endl;

		debug_os << "Before send: ";
		link->printBuffer(debug_os);

		if (detail) {
			//debug_os << *this << endl;
		}
	}

	// Find (absolute) arrival time to the next node from the given departure
	// node down the given link, taking into account that packets in window-
	// loads incur the link delay penalty once per WINDOW, not once per packet.
	bool use_delay = !link->isSameDirectionAsLastPacket(getDestinationNode());
	double arrival_time =
			link->getArrivalTime(pkt, use_delay, getTime());
	if (debug) {
		debug_os << "transmission time: " << link->getTransmissionTimeMs(pkt)
				<< ", event time: " << getTime() << ", relative pos: "
				<< pkt.getSeq() - window_start + 1 << endl;
		debug_os << "arrival time: " << arrival_time << endl;
	}

	// Use the arrival time to queue a receive_packet_event (does nothing if
	// the link buffer has no room, thereby dropping the packet).
	if (link->sendPacket(pkt, getDestinationNode(), use_delay, getTime())) {
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

	// log data
	double currTime = getTime();
	sim->logEvent(currTime);
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

	if(debug && this) {
		debug_os << getTime() << "\tSTARTING FLOW: " << *this << endl;
	}

	// Queue the timeout event for the flow, which runs if no acknowledgements
	// are received before the listed time is reached.
	//flow->initFlowTimeout();

	// Get the current (i.e. the first) window's packet(s) to send.
	double linkFreeAt = flow->getSource()->getLink()->getLinkFreeAtTime();
	vector<packet> pkts_to_send =
			flow->popOutstandingPackets(getTime(),
					linkFreeAt == 0 ? flow->getStartTimeMs() : linkFreeAt);

	if(debug && pkts_to_send.size() == 0) {
		debug_os << "Flow cannot start because there are no packets to send."
				<< endl;
		return;
	}

	// Iterate over the packets to send, making a send_packet_event for each.
	// The timout_events have already been added to the flow and to the
	// simulation's queue.
	int window_size = pkts_to_send.size();
	int first_seqnum_in_window = pkts_to_send[0].getSeq(); // assume is ordered
	vector<packet>::iterator pkt_it = pkts_to_send.begin();
	while(pkt_it != pkts_to_send.end()) {
		send_packet_event *e = new send_packet_event(getTime(), *sim, *flow,
				*pkt_it, *(flow->getSource()->getLink()),
				*(flow->getSource()), window_size, first_seqnum_in_window);
		sim->addEvent(e);
		pkt_it++;
	}

	// log data
	double currTime = getTime();
	sim->logEvent(currTime);
}

void start_flow_event::printHelper(ostream &os) {
	event::printHelper(os);

	flow->setNestingDepth(1);

	os << "<-- start_flow_event {" << endl <<
			"  flow: " << *flow << endl << "}";

	flow->setNestingDepth(0);
}

// ----------------------------- timeout_event class --------------------------

timeout_event::timeout_event() :
		event(), flow(NULL) { }

timeout_event::timeout_event(
		double time, simulation &sim, netflow &flow) :
				event(time, sim), flow(&flow) { }

timeout_event::~timeout_event() { }

void timeout_event::runEvent() {

	if(debug && this) {
		debug_os << getTime() << "\tTIMEOUT TRIGGERED: "
				<< *this << endl;
	}

	// Resize the window, set the linear growth threshold, cancel the
	// corresponding timeout locally (this event running means it was
	// dequeued in the events queue).
	flow->timeoutOccurred();

	// Now send the timed out packet again.
	double linkFreeAt = flow->getSource()->getLink()->getLinkFreeAtTime();
	vector<packet> pkts_to_send =
			flow->popOutstandingPackets(getTime(),
					linkFreeAt == 0 ? getTime(): linkFreeAt);

	// Iterate over the packets to send, making a send_packet_event for each.
	// The timout_events have already been added to the flow and to the
	// simulation's queue.
	int window_size = pkts_to_send.size();
	int first_seqnum_in_window = pkts_to_send[0].getSeq(); // assume is ordered
	vector<packet>::iterator pkt_it = pkts_to_send.begin();
	while(pkt_it != pkts_to_send.end()) {
		send_packet_event *e = new send_packet_event(getTime(), *sim, *flow,
				*pkt_it, *(flow->getSource()->getLink()),
				*(flow->getSource()), window_size, first_seqnum_in_window);
		sim->addEvent(e);
		pkt_it++;
	}

	// Queue up a new timeout event
	timeout_event *new_te = new timeout_event(getTime() +
			flow->getTimeoutLengthMs(), *sim, *flow);
	flow->setFlowTimeout(new_te);
	sim->addEvent(new_te);

	// log data
	double currTime = getTime();
	sim->logEvent(currTime);
}

/**
 * Print helper function.
 * @param os The output stream to which to write event information.
 */
void timeout_event::printHelper(ostream &os) {
	event::printHelper(os);

	flow->setNestingDepth(1);

	os << "<-- timeout_event. {" << endl <<
			"  flow: " << *flow << endl << "}";

	flow->setNestingDepth(0);
}

// ------------------------------- ack_event class ----------------------------

ack_event::ack_event() :
		event(), flow(NULL), dup_pkt(packet()) { }

ack_event::ack_event(double time, simulation &sim,
		netflow &flow, packet &dup_pkt) :
				event(time, sim), flow(&flow), dup_pkt(dup_pkt) { }

ack_event::~ack_event() { }

void ack_event::runEvent() {

	if(debug && this) {
		debug_os << getTime() << "\tACK TRIGGERED: "
				<< dup_pkt.getSeq() << endl;
		//		<< *this << endl;
	}

	// Make and queue the ACK's send_packet_event.
	send_packet_event *e = new send_packet_event(getTime(), *sim, *flow,
			dup_pkt, *(flow->getDestination()->getLink()),
			*(flow->getDestination()));
	sim->addEvent(e);

	// log data
	double currTime = getTime();
	sim->logEvent(currTime);
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
