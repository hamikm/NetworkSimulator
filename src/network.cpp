/*
 * See header file for comments.
 */

#include "network.h"

// ------------------------------ netelement class ----------------------------

netelement::netelement (string name) : name(name) { }

netelement::~netelement() { }

const string &netelement::getName() const { return name; }

void netelement::printHelper(std::ostream &os) const {
	os << "[device. name: " << name << "]";
}

// ------------------------------- netnode class ------------------------------

netnode::netnode (string name) : netelement(name) { }

netnode::netnode (string name, vector<netlink *> links) :
	netelement(name), links(links) { }

void netnode::addLink (netlink &link) { links.push_back(&link); }

const vector<netlink *> &netnode::getLinks() const { return links; }

void netnode::printHelper(ostream &os) const {
	netelement::printHelper(os);
	os << " ---> [node. links: { ";
	for (unsigned int i = 0;
			i < (links.size() == 0 ? 0 : links.size() - 1); i++) {
		os << links[i]->getName() << ", ";
	}
	if (links.size() > 0) {
		os << links[links.size() - 1]->getName();
	}
	os << " } ]";
}

// ------------------------------- nethost class ------------------------------

nethost::nethost (string name) : netnode(name) { }

nethost::nethost (string name, netlink &link) : netnode(name) {
	addLink(link);
}

netlink *nethost::getLink() const {
	if(getLinks().size() == 0)
		return NULL;
	return getLinks()[0];
}

void nethost::setLink(netlink &link) {
	links.clear();
	addLink(link);
}


void nethost::receivePacket(double time, simulation &sim, 
	netflow &flow, packet &pkt) {
	if (pkt.getType() == ACK)
		nethost::receiveAckPacket(time, sim, flow, pkt);
	else if (pkt.getType() == FLOW)
		nethost::receiveFlowPacket(time, sim, flow, pkt);
}

void nethost::receiveAckPacket(double time, simulation &sim, 
	netflow &flow, packet &pkt) {

	// Check if sequence number is the same as the last one
	if (pkt.getSeq() == flow.getLastAck()) {
		// Increment number of duplicate acks and check if more than 
		// allowed number. If so, do fast retransmit.
		if (flow.incrDuplicateAcks() >= 3) {
			// TODO: use a constant for maximum no. allowed duplicates
			// instead of hardcoding it above.

			// Fast retransmit: instantly try to resend packet
			packet resent_pkt(FLOW, flow, pkt.getSeq());
			send_packet_event retransmit(time, sim, flow, resent_pkt);
			sim.addEvent(retransmit);
		}
	}

	else {
		// Update the last successfully received ack
		flow.updateLastAck(pkt.getSeq());

		/** Remove the timeout action event for this packet from the queue
		 * since the corresponding flow packet does not need to be resent */

		// Remove event from the flow's map of future timeout events
		timeout_event *t_event = flow.removeTimeoutEvent(pkt.getSeq() - 1);

		// Remove event from global queue
		sim.removeEvent((*t_event));

	}
}

void nethost::receiveFlowPacket(double time, simulation &sim,
		netflow &flow, packet &pkt) {

	/** TODO */

}

void nethost::printHelper(ostream &os) const {
	netnode::printHelper(os);
	os << " ---> [host. link: "
			<< (getLink() == NULL ? "NULL" : getLink()->getName()) << "]";
}

// ------------------------------ netrouter class -----------------------------

netrouter::netrouter (string name) : netnode(name) { }

netrouter::netrouter (string name, vector<netlink *> links) :
	netnode(name, links) { }

void netrouter::receivePacket(double time, simulation &sim, netflow &flow, packet &pkt) {
	if (pkt.getType() == ROUTING) {
		// TODO: Handle routing packets later
	}
	else
		netrouter::forwardPacket(time, sim, flow, pkt);
}

void netrouter::forwardPacket(double time, simulation &sim, netflow &flow, packet &pkt) {
	// TODO
}

void netrouter::printHelper(ostream &os) const {
	netnode::printHelper(os);
	bool first = true;
	os << " ---> [router. routing table: [ ";
	map<string, netlink *>::const_iterator itr;
	for (itr = rtable.begin(); itr != rtable.end(); itr++) {
		if (first) {
			os << itr->first << "-->" << itr->second;
			first = false;
			continue;
		}
		os << ", " << itr->first << "-->" << itr->second;
	}
	os << " ]";
}

// ------------------------------- netflow class ------------------------------

void netflow::constructorHelper (double start_time, float size_mb,
		nethost &source, nethost &destination, int num_total_packets,
		int last_received_ack_seqnum, int window_size,
		int num_duplicate_acks, double timeout_length) {
	this->start_time_sec = start_time;
	this->size_mb = size_mb;
	this->source = &source;
	this->destination = &destination;
	this->num_total_packets = num_total_packets;
	this->last_received_ack_seqnum = last_received_ack_seqnum;
	this->window_size = window_size;
	this->num_duplicate_acks = num_duplicate_acks;
	this->timeout_length_ms = timeout_length;
}

netflow::netflow (string name, float start_time, float size_mb,
			nethost &source, nethost &destination) :
				netelement(name) {
	constructorHelper(start_time, size_mb, source, destination,
			size_mb / PACKET_PAYLOAD_SIZE + 1, 0, 1, 0,
			DEFAULT_INITIAL_TIMEOUT);
}

nethost *netflow::getSource() const {
	return source;
}

float netflow::getStartTimeSec() const { return start_time_sec; }

float netflow::getStartTimeMs() const { return start_time_sec * MS_PER_SEC; }

nethost *netflow::getDestination() const { return destination; }

float netflow::getSizeMb() const { return size_mb; }

void netflow::setDestination(nethost &destination) {
	this->destination = &destination;
}

void netflow::setSource(nethost &source) {
	this->source = &source;
}

int netflow::incrDuplicateAcks() {
	return ++num_duplicate_acks;
}

int netflow::getLastAck() {
	return last_received_ack_seqnum;
}

void netflow::updateLastAck(int new_seqnum) {
	last_received_ack_seqnum = new_seqnum;
}

timeout_event* netflow::removeTimeoutEvent(int seq) {
	timeout_event *t = future_timeouts_events[seq];
	future_timeouts_events.erase(seq);
	return t;
}

void netflow::printHelper(ostream &os) const {
	netelement::printHelper(os);
	os << " ---> [flow. start time(sec): " <<
			start_time_sec << ", size(mb): " << size_mb
			<< ", source: "
			<< (source == NULL ? "NULL" : source->getName())
			<< ", destination: "
			<< (destination == NULL ? "NULL" : destination->getName())
			<< "]";
}

// ------------------------------- netlink class ------------------------------

void netlink::constructor_helper(double rate_mbps, int delay_ms, int buflen_kb,
		netelement *endpoint1, netelement *endpoint2) {
	this->rate_bpms = rate_mbps * BYTES_PER_MEGABIT / MS_PER_SEC;
	this->delay_ms = delay_ms;
	this->buflen_bytes = buflen_kb * BYTES_PER_KB;
	this->endpoint1 = endpoint1 == NULL ? NULL : endpoint1;
	this->endpoint2 = endpoint2 == NULL ? NULL : endpoint2;
	this->available_at_time_ms = 0;
	this->num_packets_waiting = 0;
	this->last_known_time = 0;
}

netlink::netlink(string name, double rate_mbps, int delay_ms, int buflen_kb,
		netelement &endpoint1, netelement &endpoint2) : netelement(name) {
	constructor_helper(
			rate_mbps, delay_ms, buflen_kb, &endpoint1, &endpoint2);
}

netlink::netlink (string name, double rate_mbps, int delay_ms, int buflen_kb) :
		 netelement(name) {
	constructor_helper(rate_mbps, delay_ms, buflen_kb, NULL, NULL);
}

long netlink::getBuflen() const { return buflen_bytes; }

long netlink::getBuflenKB() const { return buflen_bytes / BYTES_PER_KB; }

int netlink::getDelay() const { return delay_ms; }

netelement *netlink::getEndpoint1() const { return endpoint1; }

void netlink::setEndpoint1(netelement &endpoint1) {
	this->endpoint1 = &endpoint1;
}

netelement *netlink::getEndpoint2() const {
	return endpoint2;
}

void netlink::setEndpoint2(netelement &endpoint2) {
	this->endpoint2 = &endpoint2;
}

double netlink::getRateBytesPerSec() const { return rate_bpms * MS_PER_SEC; }

double netlink::getRateMbps() const {
	return ((float) rate_bpms) / BYTES_PER_MEGABIT * MS_PER_SEC;
}


long netlink::getBufferOccupancy(double current_time) const {
	assert(current_time >= last_known_time);
	return rate_bpms *
			((available_at_time_ms - current_time) -
					delay_ms * num_packets_waiting);
}

double netlink::sendPacket(const packet &pkt, double current_time) {

	assert(current_time >= last_known_time);
	last_known_time = current_time;

	// Check if the buffer has space. If it doesn't then return infinity.
	long pkt_size_bytes = (long)(pkt.getSizeMb() * BYTES_PER_MEGABIT);
	if (getBufferOccupancy(current_time) + pkt_size_bytes > buflen_bytes) {
		return PKT_DROPPED_SENTINEL;
	}

	num_packets_waiting++;
	available_at_time_ms += (pkt_size_bytes / rate_bpms + delay_ms);

	return available_at_time_ms;
}

void netlink::receivedPacket() { num_packets_waiting--; }

void netlink::printHelper(ostream &os) const {
	netelement::printHelper(os);
	os << " ---> [link. rate(mbps): " << getRateMbps() << ", delay: "
			<< delay_ms << ", buffer length(kB): " << getBuflenKB()
			<< ", endpoint 1: "
			<< (endpoint1 == NULL ? "NULL" : endpoint1->getName())
			<< ", endpoint 2: "
			<< (endpoint2 == NULL ? "NULL" : endpoint2->getName()) << "]";
}

// -------------------------------- packet class ------------------------------

void packet::constructorHelper(packet_type type, const string &source_ip,
			   const string &dest_ip, int seqnum,
			   netflow *parent_flow, float size) {
	this->type = type;
	this->source_ip = source_ip;
	this->dest_ip = dest_ip;
	this->seqnum = seqnum;
	this->parent_flow = parent_flow;
	this->size = size;
}

packet::packet(packet_type type, const string &source_ip,
		const string &dest_ip) : netelement("") {
	switch (type) {
	case ROUTING:
		constructorHelper(type, source_ip, dest_ip, SEQNUM_FOR_NONFLOWS,
				NULL, ((float)ROUTING_PACKET_SIZE) / BYTES_PER_MEGABIT);
		break;
	default:
		assert(type == ROUTING); // other types not allowed in this constructor
	}
}

packet::packet(packet_type type, netflow &parent_flow, int seqnum) :
		netelement("") {

	switch (type) {
	case FLOW:
		constructorHelper(type, parent_flow.getSource()->getName(),
				parent_flow.getDestination()->getName(), seqnum,
						&parent_flow,
						((float)FLOW_PACKET_SIZE) / BYTES_PER_MEGABIT);
		break;
	case ACK:
		constructorHelper(type, parent_flow.getDestination()->getName(),
				parent_flow.getSource()->getName(), SEQNUM_FOR_NONFLOWS,
						&parent_flow,
						((float)ACK_PACKET_SIZE) / BYTES_PER_MEGABIT);
		break;
	default:
		assert(type == FLOW || type == ACK); // no other packets types allowed
	}
}

string packet::getSource() const { return source_ip; }

string packet::getDestination() const { return dest_ip; }

int packet::getSeq() const { return seqnum; }

netflow *packet::getParentFlow() const { return parent_flow; }

packet_type packet::getType() const { return type; }

float packet::getSizeMb() const { return size; }

float packet::getSizeBytes() const { return size * BYTES_PER_MEGABIT;; }

void packet::printHelper(ostream &os) const {
	netelement::printHelper(os);
	os << " ---> [packet. src: " << source_ip << ", dst: " << dest_ip <<
			", type: " << type << ", seq_num: " << seqnum << ", size: " <<
			size << "]";
}
