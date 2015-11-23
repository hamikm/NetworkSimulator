/*
 * See header file for comments.
 */

// Custom headers.
#include "network.h"
#include "simulation.h"

// ------------------------------ netelement class ----------------------------

netelement::netelement() : name("") { }

netelement::netelement(string name) : name(name) { }

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

void nethost::printHelper(ostream &os) const {
	netnode::printHelper(os);
	os << " ---> [host. link: "
			<< (getLink() == NULL ? "NULL" : getLink()->getName()) << "]";
}

// ------------------------------ netrouter class -----------------------------

netrouter::netrouter (string name) : netnode(name) { }

netrouter::netrouter (string name, vector<netlink *> links) :
	netnode(name, links) { }

void netrouter::receivePacket(double time, simulation &sim,
		netflow &flow, packet &pkt) {
	if (pkt.getType() == ROUTING) {
		// TODO Handle routing packets later
	}
	else
		netrouter::forwardPacket(time, sim, flow, pkt);
}

void netrouter::forwardPacket(double time, simulation &sim,
		netflow &flow, packet &pkt) {
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

void netflow::constructorHelper (double start_time, double size_mb,
		nethost &source, nethost &destination, int num_total_packets,
		double window_size, double timeout_length_ms, simulation &sim) {

	this->start_time_sec = start_time;
	this->size_mb = size_mb;
	this->source = &source;
	this->destination = &destination;

	this->highest_ack_seqnum = 1;
	this->highest_sent_seqnum = 0;
	this->window_size = window_size;
	this->window_start = 1;
	this->num_duplicate_acks = 0;
	this->timeout_length_ms = timeout_length_ms;
	this->lin_growth_winsize_threshold = -1;
	this->avg_RTT = -1;
	this->std_RTT = -1;

	this->sim = &sim;
}

netflow::netflow (string name, double start_time, double size_mb,
			nethost &source, nethost &destination, simulation &sim) :
				netelement(name) {
	constructorHelper(start_time, size_mb, source, destination,
			size_mb / packet::FLOW_PACKET_SIZE + 1, 1,
			DEFAULT_INITIAL_TIMEOUT, sim);
}

nethost *netflow::getSource() const {
	return source;
}

double netflow::getStartTimeSec() const { return start_time_sec; }

double netflow::getStartTimeMs() const { return start_time_sec * MS_PER_SEC; }

nethost *netflow::getDestination() const { return destination; }

double netflow::getSizeMb() const { return size_mb; }

int netflow::getNumTotalPackets() const {
	long size_in_bytes = size_mb * BYTES_PER_MEGABIT;
	if (size_in_bytes % FLOW_PACKET_SIZE == 0)
		return size_in_bytes / FLOW_PACKET_SIZE;
	return size_in_bytes / FLOW_PACKET_SIZE + 1;
}

void netflow::setDestination(nethost &destination) {
	this->destination = &destination;
}

void netflow::setSource(nethost &source) {
	this->source = &source;
}

int netflow::incrDuplicateAcks() {
	return ++num_duplicate_acks;
}

int netflow::getLastAck() const {
	return highest_ack_seqnum;
}

void netflow::setLastACKNum(int new_seqnum) {
	highest_ack_seqnum = new_seqnum;
}

int netflow::getHighestAckSeqnum() const {
	return highest_ack_seqnum;
}

int netflow::getHighestSentSeqnum() const {
	return highest_sent_seqnum;
}

int netflow::getNumDuplicateAcks() const {
	return num_duplicate_acks;
}

const map<int, double>& netflow::getRoundTripTimes() const {
	return rtts;
}

double netflow::getWindowSize() const {
	return window_size;
}

int netflow::getWindowStart() const {
	return window_start;
}

double netflow::getLinGrowthWinsizeThreshold() const {
	return lin_growth_winsize_threshold;
}

const map<int, timeout_event>& netflow::getFutureTimeoutsEvents() const {
	return future_timeouts_events;
}

const map<int, send_packet_event>& netflow::getFutureSendAckEvents() const {
	return future_send_ack_events;
}

timeout_event netflow::cancelTimeoutAction(int seq) {
	timeout_event t = future_timeouts_events[seq];
	future_timeouts_events.erase(seq);
	return t;
}

vector<packet> netflow::peekOutstandingPackets() {

	vector<packet> outstanding_pkts;

	// Iterate over the sequence numbers that haven't had corresponding
	// packets sent. Make packets for each and collect them into a vector.
	for (int i = highest_sent_seqnum + 1;
			i < window_start + window_size; i++) {
		outstanding_pkts.push_back(packet(FLOW, *this, i));
	}

	return outstanding_pkts;
}

vector<packet> netflow::popOutstandingPackets(double start_time_ms) {

	vector<packet> outstanding_pkts = peekOutstandingPackets();

	// Iterate over the packets about to be sent, keeping their start times
	// so round_trip_times can be computed later. Also making a timeout_event
	// for each one, putting them on the out parameter and into the local map.
	vector<packet>::iterator it = outstanding_pkts.begin();
	int i = 0;
	while(it != outstanding_pkts.end()) {

		// Store start time.
		rtts[it->getSeq()] = -start_time_ms;

		// Make a timeout_event for this packet, store it on out param
		// and in local map.
		timeout_event e(start_time_ms + timeout_length_ms +
				TIMEOUT_DELTA * i++, *sim, *this);
		sim->addEvent(e);
		future_timeouts_events[it->getSeq()] = e;
		it++;
	}

	highest_sent_seqnum += outstanding_pkts.size();
	return outstanding_pkts;
}

void netflow::receivedAck(packet &pkt, double end_time_ms) {

	assert(pkt.getSeq() >= highest_ack_seqnum);

	// Check if sequence number is the same as the last one; if so then
	// update duplicate ACKs field and potentially get ready to fast retransmit
	if (pkt.getSeq() == highest_ack_seqnum) {

		// Increment number of duplicate acks and check if more than
		// allowed number. If so, do fast retransmit by changing last
		// seen packet to current sequence number and halving window size
		if (++num_duplicate_acks >=
				FAST_RETRANSMIT_DUPLICATE_ACK_THRESHOLD) {
			highest_sent_seqnum = pkt.getSeq();
			window_size = window_size / 2 > 1 ? window_size / 2 : 1;
			lin_growth_winsize_threshold = window_size;
			num_duplicate_acks = 0;
		}
	}

	// Otherwise we had a successful transmission, so slide and grow the
	// window, adjust the average and std of RTTs so the timeout length
	// can be set, and remove the corresponding timeout event from the flow.
	else {
		// Update the last successfully received ack
		highest_ack_seqnum = pkt.getSeq();

		// The corresponding FLOW packet had sequence number one less
		int flow_seqnum = pkt.getSeq() - 1;

		// Make sure we have a departure time for this ACK's corresponding FLOW
		// packet.
		assert (rtts.find(flow_seqnum) != rtts.end());

		// Make sure the departure time was stored as a negative number.
		assert (rtts[flow_seqnum] < 0);

		// If we do then get the round-trip time for this packet then delete
		// its entry from the RTT table.
		double rtt = rtts[flow_seqnum] + end_time_ms;
		rtts.erase(flow_seqnum);

		// If it's the first ACK we don't have an average or deviation for the
		// round-trip times, so initialize them to the first RTT
		if (avg_RTT < 0 && std_RTT < 0) {
			avg_RTT = rtt;
			std_RTT = rtt;
		}
		else { // If we do have initial avg and std values, then adjust them
			avg_RTT = (1 - B_TIMEOUT_CALC) * avg_RTT + B_TIMEOUT_CALC * rtt;
			std_RTT = (1 - B_TIMEOUT_CALC) * std_RTT +
					B_TIMEOUT_CALC * abs(rtt - avg_RTT);
		}

		// Now update the timeout length
		timeout_length_ms = avg_RTT + 4 * std_RTT;

		// Now adjust the window start and size. Since we got just received
		// an ACK for a successfully received packet we can slide the window
		// start by one to the right, and we can adjust the window size
		// by whether we're in exponential or linear growth mode.
		window_start++;
		if (lin_growth_winsize_threshold < 0) { // hasn't been initialized, so
											    // just do exponential growth
			window_size++;
		}
		else if (window_size < lin_growth_winsize_threshold) { // exponential
			window_size++;
		}
		else { // linear
			window_size += 1 / window_size;
		}

		// Remove event from the flow's map of future timeout events
		// and from the simulation's event queue
		timeout_event to = cancelTimeoutAction(pkt.getSeq() - 1);
		sim->removeEvent(to);
	}
}

double netflow::getTimeoutLengthMs() const { return timeout_length_ms; }

void netflow::timeoutOccurred() {
	lin_growth_winsize_threshold = window_size / 2;
	window_size = 1;
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
	this->buffer_capacity = buflen_kb * BYTES_PER_KB;
	this->endpoint1 = endpoint1 == NULL ? NULL : endpoint1;
	this->endpoint2 = endpoint2 == NULL ? NULL : endpoint2;
	this->wait_time = 0;
	this->buffer_occupancy = 0;
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

long netlink::getBuflen() const { return buffer_capacity; }

long netlink::getBuflenKB() const { return buffer_capacity / BYTES_PER_KB; }

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
	return ((double) rate_bpms) / BYTES_PER_MEGABIT * MS_PER_SEC;
}

double netlink::getTransmissionTimeMs(const packet &pkt) const {
	return pkt.getSizeBytes() / rate_bpms + delay_ms;
}

double netlink::getWaitTimeIntervalMs() const {
	return wait_time;
}

int netlink::getBufferOccupancy() const { return buffer_occupancy; }

bool netlink::sendPacket(const packet &pkt) {

	// Check if the buffer has space. If it doesn't then return false.
	if (getBufferOccupancy() + pkt.getSizeBytes() > buffer_capacity) {
		return false;
	}

	buffer.push(pkt);
	wait_time += getTransmissionTimeMs(pkt);
	buffer_occupancy += pkt.getSizeBytes();
	return true;
}

bool netlink::receivedPacket(long pkt_id) {
	if(buffer.size() == 0 || buffer.front().getId() != pkt_id)
		return false;
	wait_time -= getTransmissionTimeMs(buffer.front());
	buffer_occupancy -= buffer.front().getSizeBytes();
	buffer.pop();
	return true;
}

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

long packet::id_gen = 1;

void packet::constructorHelper(packet_type type, const string &source_ip,
			   const string &dest_ip, int seqnum,
			   netflow *parent_flow, double size) {
	this->type = type;
	this->source_ip = source_ip;
	this->dest_ip = dest_ip;
	this->seqnum = seqnum;
	this->parent_flow = parent_flow;
	this->size = size;
	this->pkt_id = id_gen++;
}

packet::packet() :
		netelement(), pkt_id(0), type(FLOW), source_ip(""), dest_ip(""),
		parent_flow(NULL), size(FLOW_PACKET_SIZE), seqnum(0) { }

packet::packet(packet_type type, const string &source_ip,
		const string &dest_ip) : netelement("") {
	switch (type) {
	case ROUTING:
		constructorHelper(type, source_ip, dest_ip, SEQNUM_FOR_NONFLOWS,
				NULL, ((double)ROUTING_PACKET_SIZE) / BYTES_PER_MEGABIT);
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
						((double)FLOW_PACKET_SIZE) / BYTES_PER_MEGABIT);
		break;
	case ACK:
		constructorHelper(type, parent_flow.getDestination()->getName(),
				parent_flow.getSource()->getName(), seqnum,
						&parent_flow,
						((double)ACK_PACKET_SIZE) / BYTES_PER_MEGABIT);
		break;
	default:
		assert(type == FLOW || type == ACK); // no other packets types allowed
	}
}

string packet::getSource() const { return source_ip; }

string packet::getDestination() const { return dest_ip; }

int packet::getSeq() const { return seqnum; }

netflow *packet::getParentFlow() const { return parent_flow; }

long packet::getId() const { return pkt_id; }

packet_type packet::getType() const { return type; }

double packet::getSizeMb() const { return size; }

long packet::getSizeBytes() const { return size * BYTES_PER_MEGABIT; }

void packet::printHelper(ostream &os) const {
	netelement::printHelper(os);
	os << " ---> [packet. src: " << source_ip << ", dst: " << dest_ip <<
			", type: " << type << ", seq_num: " << seqnum << ", size: " <<
			size << "]";
}
