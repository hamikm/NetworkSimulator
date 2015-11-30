/*
 * See header file for comments.
 */

// Custom headers.
#include "network.h"
#include "simulation.h"

// ------------------------------ netelement class ----------------------------

netelement::netelement() : name(""), nest_depth(0) { }

netelement::netelement(string name) : name(name), nest_depth(0) { }

netelement::~netelement() { }

const string &netelement::getName() const { return name; }

void netelement::setNestingDepth(int depth) { this->nest_depth = depth; }

string netelement::nestingPrefix(int delta) const {
	string rtn = "";
	for (int i = 0; i < nest_depth + delta; i++) {
		rtn += "  ";
	}
	return rtn;
}

void netelement::printHelper(std::ostream &os) const {
	os << "netelement. name: \"" << name << "\"";
}

// ------------------------------- netnode class ------------------------------

netnode::netnode () : netelement() {}

netnode::netnode (string name) : netelement(name) { }

netnode::netnode (string name, vector<netlink *> links) :
	netelement(name), links(links) { }

void netnode::addLink (netlink &link) { links.push_back(&link); }

bool netnode::isRoutingNode() const { return false; }

const vector<netlink *> &netnode::getLinks() const { return links; }

void netnode::printHelper(ostream &os) const { // TODO automate nesting
	netelement::printHelper(os);
	os << " <-- node. links: { " << endl << nestingPrefix(1) << "[";
	for (unsigned int i = 0;
			i < (links.size() == 0 ? 0 : links.size() - 1); i++) {
		os << links[i]->getName() << ", ";
	}
	if (links.size() > 0) {
		os << links[links.size() - 1]->getName();
	}
	os << "]" << endl << nestingPrefix(0) << "}";
}

// ------------------------------- nethost class ------------------------------

nethost::nethost (string name) : netnode(name) { }

nethost::nethost (string name, netlink &link) : netnode(name) {
	addLink(link);
}

bool nethost::isRoutingNode()  const { return false; }

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
	os << " <-- host";
}

// ------------------------------ netrouter class -----------------------------

netrouter::netrouter (string name) : netnode(name) { }

netrouter::netrouter (string name, vector<netlink *> links) :
	netnode(name, links) { }

bool netrouter::isRoutingNode() const { return true; }

map<netlink *, packet> netrouter::receivePacket(double time, simulation &sim,
		netflow &flow, packet &pkt) {
 
	map<netlink *, packet> link_pkt_map;	
	netlink *link_to_use = rtable.at(pkt.getDestination());
	link_pkt_map[link_to_use] = pkt;

	return link_pkt_map;
}

void netrouter::receiveRoutingPacket(double time, simulation &sim, netflow &flow, 
			packet &pkt, netlink &link) {

	// Update routing table as follows: For each destination,
	// if distance reported by the packet is less than the distance
	// stored in the router's rdistances map, update with the new 
	// distance and set the link_ptr in rtable to the link on
	// which the packet arrived.

	map<string, double> received_dist = pkt.getDistances();
	map<string, double>::iterator it = received_dist.begin();
	bool updated = false;
	double travel_time = time - pkt.getTransmitTimestamp(); 			

	while (it != received_dist.end()) {
		string key = it->first;

		if (it->second + travel_time < rdistances[key]) {
			updated = true;
		
			rdistances[key] = it->second + travel_time;

			// Set link_ptr in routing table to link this packet came from.
			rtable[key] = &link;

		}
		it++;
	}

	if (updated) {
		// Send routing packets to adjacent routers.

		vector<netlink *> adj_links = getLinks();
		for (unsigned i = 0; i < adj_links.size(); i++) {

			netnode *other_node;
			if (adj_links[i]->getEndpoint1()->getName().c_str() == 
									 this->getName().c_str()) {
				other_node = adj_links[i]->getEndpoint2();
			}
			else {
				other_node = adj_links[i]->getEndpoint1();
			}

			// Check if other_node points to router
			if (other_node->isRoutingNode()) {

				packet rpack = packet(ROUTING, this->getName(), other_node->getName());
				rpack.setDistances(rdistances); 
				rpack.setTransmitTimestamp(time); 			// Time that packet is sent

				// Check when link is free and queue up new packet
				send_packet_event *e = new send_packet_event(time, sim, flow,
					rpack, *adj_links[i], *this);
				sim.addEvent(e);

			}

		}
	}

}

void netrouter::printHelper(ostream &os) const {
	netnode::printHelper(os);
	os << " <-- router. routing table: ";
	if (rtable.size() == 0) {
		os << "{ }";
		return;
	}
	os << "{ " << endl;
	map<string, netlink *>::const_iterator itr;
	for (itr = rtable.begin(); itr != rtable.end(); itr++) {
		os << nestingPrefix(1) << "(" <<
				itr->first << "<--" << itr->second << ")";
	}
	os << nestingPrefix(0) << "}";
}

// ------------------------------- netflow class ------------------------------

void netflow::constructorHelper (double start_time, double size_mb,
		nethost &source, nethost &destination, int num_total_packets,
		double window_size, double timeout_length_ms, simulation &sim) {

	this->start_time_sec = start_time;
	this->size_mb = size_mb;
	this->source = &source;
	this->destination = &destination;

	this->pktTally = 0;
	this->leftTime = start_time;
	this->rightTime = start_time + RATE_INTERVAL;

	this->amt_sent_mb = 0;
	this->highest_received_ack_seqnum = 1;
	this->highest_sent_flow_seqnum = 0;
	this->highest_received_flow_seqnum = 0;
	this->window_size = window_size;
	this->window_start = 1;
	this->num_duplicate_acks = 0;
	this->timeout_length_ms = timeout_length_ms;
	this->lin_growth_winsize_threshold = -1;
	this->avg_RTT = -1;
	this->std_RTT = -1;
	this->pkt_RRT = -1;

	this->sim = &sim;
}

netflow::netflow (string name, double start_time, double size_mb,
			nethost &source, nethost &destination, simulation &sim) :
				netelement(name) {
	constructorHelper(start_time, size_mb, source, destination,
			size_mb / packet::FLOW_PACKET_SIZE + 1, 1,
			DEFAULT_INITIAL_TIMEOUT, sim);
}


double netflow::getStartTimeSec() const { return start_time_sec; }

double netflow::getStartTimeMs() const { return start_time_sec * MS_PER_SEC; }

double netflow::getSizeMb() const { return size_mb; }

int netflow::getNumTotalPackets() const {
	long size_in_bytes = size_mb * BYTES_PER_MEGABIT;
	if (size_in_bytes % FLOW_PACKET_SIZE == 0)
		return size_in_bytes / FLOW_PACKET_SIZE;
	return size_in_bytes / FLOW_PACKET_SIZE + 1;
}

nethost *netflow::getDestination() const { return destination; }

void netflow::setDestination(nethost &destination) {
	this->destination = &destination;
}

nethost *netflow::getSource() const {
	return source;
}

void netflow::setSource(nethost &source) {
	this->source = &source;
}

int netflow::getPktTally() const { return pktTally; }

void netflow::incPktTally() {
	pktTally++;
}

void netflow::resetPktTally() {
	pktTally = 0;
}
	
double netflow::getLeftTime() const { return leftTime; }

void netflow::setLeftTime(double newTime) {
	leftTime = newTime;
}
	
double netflow::getRightTime() const { return rightTime; }

void netflow::setRightTime(double newTime) {
	rightTime = newTime;
}

int netflow::getLastAck() const {
	return highest_received_ack_seqnum;
}

void netflow::setLastACKNum(int new_seqnum) {
	highest_received_ack_seqnum = new_seqnum;
}

int netflow::getHighestAckSeqnum() const {
	return highest_received_ack_seqnum;
}

int netflow::getHighestSentSeqnum() const {
	return highest_sent_flow_seqnum;
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

double netflow::getRateBytesPerSec() const {
	return window_size * FLOW_PACKET_SIZE / avg_RTT;
}

double netflow::getRateMbps() const {
	// time trace instanenous flow rate
	return window_size * FLOW_PACKET_SIZE / avg_RTT;
}

double netflow::getPktDelay(double currTime) const {
	return pkt_RRT;
}


timeout_event *netflow::delayFlowTimeout(double new_time) {
	// Find and remove the current timeout_event in the global event queue.
	this->sim->removeEvent(this->flow_timeout);

	// Queue up new event scheduled for input time
	timeout_event *e = new timeout_event(new_time, *sim, *this);
	this->sim->addEvent(e);

	// Make sure the flow has a pointer to this event for future updates.
	this->flow_timeout = e;
	return e;
}

void netflow::registerSendDuplicateAckAction(int seq, double time) {
	// Update and queue up new ack_event
	packet p = packet(ACK, *this, seq);
	ack_event *e = new ack_event(time, *sim, *this, p);
	sim->addEvent(e);
}

vector<packet> netflow::peekOutstandingPackets() {

	// If we're done sending this flow's data then return nothing.
	if (amt_sent_mb >= size_mb) {
		return vector<packet>();
	}

	vector<packet> outstanding_pkts;

	// Iterate over the sequence numbers that haven't had corresponding
	// packets sent. Make packets for each and collect them into a vector.
	for (int i = highest_sent_flow_seqnum + 1;
			i < window_start + window_size; i++) {
		outstanding_pkts.push_back(packet(FLOW, *this, i));
	}

	cerr << "outstanding pkts: " << outstanding_pkts.size() << endl;
	return outstanding_pkts;
}

vector<packet> netflow::popOutstandingPackets(double start_time,
		double linkFreeAt) {

	// If we're done sending this flow's data then return nothing.
	if (amt_sent_mb >= size_mb) {
		return vector<packet>();
	}

	vector<packet> outstanding_pkts = peekOutstandingPackets();

	// Iterate over the packets about to be sent, keeping their start times
	// so round_trip_times can be computed later. Also making a timeout_event
	// for each one, putting them on the out parameter and into the local map.
	vector<packet>::iterator it = outstanding_pkts.begin();
	//int i = 0;
	while(it != outstanding_pkts.end()) {

		// Store start time.
		rtts[it->getSeq()] = -start_time;

		/*
		// Make a timeout_event for this packet, store it in local map and
		// push it onto the simulation's event queue.
		registerTimeoutAction(it->getSeq(),
				linkFreeAt + timeout_length_ms + TIME_EPSILON * i++);
				*/
		it++;

	}

	highest_sent_flow_seqnum += outstanding_pkts.size();
	amt_sent_mb += ((double)packet::FLOW_PACKET_SIZE) / BYTES_PER_MEGABIT *
			outstanding_pkts.size();

	return outstanding_pkts;
}

void netflow::updateTimeoutLength(double end_time_ms, int flow_seqnum) {

	// Make sure we have a departure time for this ACK's corresponding FLOW
	// packet.
	assert (rtts.find(flow_seqnum) != rtts.end());

	// Make sure the departure time was stored as a negative number.
	assert (rtts[flow_seqnum] < 0);

	// If we do then get the round-trip time for this packet then delete
	// its entry from the RTT table.
	double rtt = rtts[flow_seqnum] + end_time_ms;
	rtts.erase(flow_seqnum);
	
	// update state variable
	pkt_RRT = rtt;
	
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
}

void netflow::receivedAck(packet &pkt, double end_time_ms,
		double linkFreeAtTime) {

	assert(pkt.getType() == ACK);
	assert(pkt.getSeq() >= highest_received_ack_seqnum);

	cerr << "received ack" << endl;

	/*
	 * Check if sequence number is the same as the last one (i.e., duplicate
	 * ACK); if so then update duplicate ACKs field and potentially get ready
	 * to fast retransmit. Also push back the local and global timeout event
	 * for this packet if we're not doing fast retransmit; if we are then
	 * just cancel the pending timeout, since popOutstandingPackets will
	 * create one later.
	 */
	if (pkt.getSeq() == highest_received_ack_seqnum) {

		// Increment number of duplicate acks and check if more than
		// allowed number. If so, do fast retransmit by changing last
		// seen packet to current sequence number and halving window size
		if (++num_duplicate_acks >=
				FAST_RETRANSMIT_DUPLICATE_ACK_THRESHOLD) {
			highest_sent_flow_seqnum = pkt.getSeq()-1;
			window_start = pkt.getSeq();

			window_size = window_size / 2 > 1 ? window_size / 2 : 1;
			lin_growth_winsize_threshold = window_size;
			num_duplicate_acks = 0;

			delayFlowTimeout(end_time_ms + timeout_length_ms);
		}

		// Got a duplicate ACK but don't have enough of them to do a fast
		// retransmit. Push back the timeout event by canceling the old one
		// and registering a new one.
		else {
			// Don't push back the timeout (for now)
			// TODO: make sure interval between duplicate ACK retransmit
			// is less than timeout length.
		}
	}

	// Otherwise if the ACK number is one more than highest ACK we've seen we
	// had a successful transmission, so slide and grow the window, adjust the
	// average and std of RTTs so the timeout length can be set, and remove
	// the corresponding timeout event from the flow.
	else if (pkt.getSeq() == highest_received_ack_seqnum + 1) {
		// Update the last successfully received ack
		highest_received_ack_seqnum = pkt.getSeq();

		// The corresponding FLOW packet had sequence number one less
		int flow_seqnum = pkt.getSeq() - 1;

		updateTimeoutLength(end_time_ms, flow_seqnum);

		// Now adjust the window start and size. Since we just received
		// an ACK for a successfully received packet we can slide the window
		// start by one to the right, and we can adjust the window size
		// by whether we're in exponential or linear growth mode.
		window_start++;
		if (lin_growth_winsize_threshold < 0) { // hasn't been initialized, so
											    // just do exponential growth
			window_size++;
		}
		else if (window_size < lin_growth_winsize_threshold) { // exp. part
			window_size++;
		}
		else { // linear growth part
			window_size += 1 / window_size;
		}

		// Successfully received an ACK, so we push back the timeout.
		delayFlowTimeout(end_time_ms + timeout_length_ms);
	}

	cerr << " --------------- " << endl;
	cerr << "highest_received_ack_seqnum" << highest_received_ack_seqnum << endl;
	cerr << "highest_sent_flow_seqnum" << highest_sent_flow_seqnum << endl;
	cerr << "window start" << window_start << endl;
	cerr << "window size" << window_size << endl;
	cerr << " --------------- " << endl;

}

void netflow::receivedFlowPacket(packet &pkt, double arrival_time) {

	assert(pkt.getType() == FLOW);

	// Make the corresponding ACK packet and set the highest received flow
	// sequence number.

	if (pkt.getSeq() ==  highest_received_flow_seqnum + 1) {
		highest_received_flow_seqnum++;
		// TODO: confirm whether timeouts are delayed upon receipt
		// of correct ACKs
		//delayFlowTimeout(arrival_time + timeout_length_ms);
	}


	// Make and queue (locally and on the simulation event queue) an immediate
	// duplicate_ack_event. When that event is run it will queue another,
	// future duplicate_ack_event in case the next FLOW packet never arrives.
	packet ackpack = packet(ACK, *this, highest_received_flow_seqnum + 1);
	if (amt_sent_mb < size_mb) {
		registerSendDuplicateAckAction(ackpack.getSeq(), arrival_time);
	}

}

double netflow::getTimeoutLengthMs() const { return timeout_length_ms; }

timeout_event* netflow::initFlowTimeout() {
	timeout_event *e = new timeout_event(getStartTimeMs() + timeout_length_ms,
	 									 *sim, *this);
	this->flow_timeout = e;
	this->sim->addEvent(e);
	return e;
}

timeout_event* netflow::setFlowTimeout(timeout_event *e) {
	this->flow_timeout = e;
	return e;
}

void netflow::timeoutOccurred() {
	lin_growth_winsize_threshold = window_size / 2;
	window_size = 1;
	window_start = highest_received_ack_seqnum;
	num_duplicate_acks = 0;
	rtts.clear();
	highest_sent_flow_seqnum = window_start - 1;
	//cancelAllTimeouts();
}

void netflow::printHelper(ostream &os) const {
	netelement::printHelper(os);
	os << " <-- flow. {" << endl
			<< nestingPrefix(1) << "start: " <<
				start_time_sec << " secs," << endl
			<< nestingPrefix(1) << "size: " <<
				size_mb << " megabits," << endl
			<< nestingPrefix(1) << "source: \"" <<
				(source == NULL ? "NULL" : source->getName()) << "\"," << endl
			<< nestingPrefix(1) << "destination: \"" <<
				(destination == NULL ? "NULL" : destination->getName()) <<
				"\"," << endl
			<< nestingPrefix(1) << "data sent: " <<
				amt_sent_mb << " megabits," << endl
			<< nestingPrefix(1) << "linear growth threshold: " <<
				lin_growth_winsize_threshold << " packets," << endl
			<< nestingPrefix(1) << "timeout length: " <<
				timeout_length_ms << " ms," << endl
			<< nestingPrefix(1) << "window start: " <<
				window_start << "-th packet," << endl
			<< nestingPrefix(1) << "window size: " <<
				window_size << " packets," << endl
			<< nestingPrefix(1) << "last seqnum sent: " <<
				highest_sent_flow_seqnum << "-th packet," << endl
			<< nestingPrefix(1) << "last ACK seen: " <<
				highest_received_ack_seqnum << "-th ACK," << endl
			<< nestingPrefix(0) << "}";
}

// ------------------------------- netlink class ------------------------------

void netlink::constructor_helper(double rate_mbps, int delay_ms, int buflen_kb,
		netnode *endpoint1, netnode *endpoint2) {
	this->rate_bpms = rate_mbps * BYTES_PER_MEGABIT / MS_PER_SEC;
	this->delay_ms = delay_ms;
	this->buffer_capacity = buflen_kb * BYTES_PER_KB;
	this->endpoint1 = endpoint1 == NULL ? NULL : endpoint1;
	this->endpoint2 = endpoint2 == NULL ? NULL : endpoint2;
}

netlink::netlink(string name, double rate_mbps, int delay_ms, int buflen_kb,
		netnode &endpoint1, netnode &endpoint2) : netelement(name) {
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

netnode *netlink::getEndpoint1() const { return endpoint1; }

void netlink::setEndpoint1(netnode &endpoint1) {
	this->endpoint1 = &endpoint1;
}

netnode *netlink::getEndpoint2() const {
	return endpoint2;
}

void netlink::setEndpoint2(netnode &endpoint2) {
	this->endpoint2 = &endpoint2;
}

double netlink::getRateBytesPerSec() const { return rate_bpms * MS_PER_SEC; }

double netlink::getRateMbps() const {
	return ((double) rate_bpms) / BYTES_PER_MEGABIT * MS_PER_SEC;
}

double netlink::getTransmissionTimeMs(const packet &pkt) const {
	return pkt.getSizeBytes() / rate_bpms;
}

double netlink::getLinkFreeAtTime() const {
	if (buffer.size() == 0) {
		return 0;
	}
	return buffer.rbegin()->first;
}

void netlink::printBuffer(ostream &os) {
	os << nestingPrefix(0) << "Link buffer: " << endl;
	map<double, packet>::iterator it;
	for(it = buffer.begin(); it != buffer.end(); it++) {
		os << nestingPrefix(1) << "(arrival time: " << it->first
				<< ", " << it->second.getTypeString() << " packet #: "
				<< it->second.getSeq() << ")" << endl;
	}
	os << nestingPrefix(0) << "buffer size: " << getBufferOccupancy() << endl;
	os << nestingPrefix(0) << "free at: " << getLinkFreeAtTime() << endl;
}

long netlink::getBufferOccupancy() const {
	long buffer_occupancy = 0;
	map<double, packet>::const_iterator it;
	for(it = buffer.begin(); it != buffer.end(); it++) {
		buffer_occupancy += it->second.getSizeBytes();
	}
	return buffer_occupancy;
}

int netlink::getPktLoss() const {
	return packets_dropped;
}

double netlink::getArrivalTime(const packet &pkt, bool useDelay, double time) {
	return (useDelay ? getDelay() : 0) + getTransmissionTimeMs(pkt) +
			(buffer.size() > 0 ? buffer.rbegin()->first : time);
}

bool netlink::sendPacket(const packet &pkt, bool useDelay, double time) {

	// Check if the buffer has space. If it doesn't then return false.
	if (getBufferOccupancy() + pkt.getSizeBytes() > buffer_capacity) {
		packets_dropped++;
		return false;
	}
	buffer[getArrivalTime(pkt, useDelay, time)] = pkt;
	packets_dropped = 0;
	return true;
}

bool netlink::receivedPacket(long pkt_id) {
	if(buffer.size() == 0 || buffer.begin()->second.getId() != pkt_id) {
		return false;
	}
	buffer.erase(buffer.begin());
	return true;
}

void netlink::printHelper(ostream &os) const {
	netelement::printHelper(os);
	os << " <-- link. {" << endl
			<< nestingPrefix(1) << "rate: " <<
				getRateMbps() << " megabits/second," << endl
			<< nestingPrefix(1) << "delay: " <<
				delay_ms << " ms," << endl
			<< nestingPrefix(1) << "buffer length: " <<
				getBuflenKB() << " kilobytes," << endl
			<< nestingPrefix(1) << "endpoint 1: \"" <<
				(endpoint1 == NULL ? "NULL" : endpoint1->getName()) <<
				"\"," << endl
			<< nestingPrefix(1) << "endpoint 2: \"" <<
				(endpoint2 == NULL ? "NULL" : endpoint2->getName()) <<
				"\"," << endl
			<< nestingPrefix(1) << "number of packets in buffer: " <<
				buffer.size() << " packets," << endl
			<< nestingPrefix(1) << "occupancy: " <<
				getBufferOccupancy() << " bytes" << endl
			<< nestingPrefix(1) << "free at: " <<
				getLinkFreeAtTime() << " ms" << endl
			<< nestingPrefix(0) << "}";
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

bool packet::isNullPacket() const { return pkt_id == 0 ? true : false; }

string packet::getSource() const { return source_ip; }

string packet::getDestination() const { return dest_ip; }

int packet::getSeq() const { return seqnum; }

map<string, double> packet::getDistances() const { return distance_vec; }

void packet::setDistances(map<string, double> distances) {
	this->distance_vec = distances;
}

netflow *packet::getParentFlow() const { return parent_flow; }

long packet::getId() const { return pkt_id; }

packet_type packet::getType() const { return type; }

double packet::getSizeMb() const { return size; }

long packet::getSizeBytes() const { return size * BYTES_PER_MEGABIT; }

string packet::getTypeString() const {
	switch(type) {
	case ACK:
		return "ACK";
		break;
	case FLOW:
		return"FLOW";
		break;
	case ROUTING:
		return "ROUTING";
		break;
	default:
		return "ERROR_TYPE";
		break;
	}
}

double packet::getTransmitTimestamp() const { return transmit_timestamp; }

void packet::setTransmitTimestamp(double time) { transmit_timestamp = time; }

void packet::printHelper(ostream &os) const {
	netelement::printHelper(os);

	os << " <-- packet. {" << endl
			<< nestingPrefix(1) << "source: \"" << source_ip << "\"," << endl
			<< nestingPrefix(1) << "destination: \"" <<
				dest_ip << "\"," << endl
			<< nestingPrefix(1) << "type: " << getTypeString() << "," << endl
			<< nestingPrefix(1) << "size: " << getSizeBytes() << endl
			<< nestingPrefix(1) << "sequence number: " << getSeq() << endl
			<< nestingPrefix(0) << "}";
}
