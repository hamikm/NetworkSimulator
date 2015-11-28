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

	if (pkt.getType() == ROUTING) {
		// TODO handle routing packets later. See the else below or
		// netflow::popOutstandingPackets for an example.
	}
	else {
		netlink *link_to_use = rtable.at(pkt.getDestination());
		link_pkt_map[link_to_use] = pkt;
	}

	return link_pkt_map;
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

	this->num_total_packets =num_total_packets;
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

	this->sim = &sim;

	// Initialize vector of received packets.
	this->received_flow_packets.push_back(true);	// For easy indexing.
	for (int i = 1; i <= num_total_packets; i++)
		this->received_flow_packets.push_back(false);

}

netflow::netflow (string name, double start_time, double size_mb,
			nethost &source, nethost &destination, simulation &sim) :
				netelement(name) {
	constructorHelper(start_time, size_mb, source, destination,
			1e6 * size_mb / packet::FLOW_PACKET_SIZE + 1, 1,
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

const map<int, timeout_event *>& netflow::getFutureTimeoutsEvents() const {
	return future_timeouts_events;
}

const map<int, ack_event *>& netflow::getFutureSendAckEvents() const {
	return future_send_ack_events;
}

timeout_event *netflow::cancelTimeoutAction(int seq) {
	if (future_timeouts_events.find(seq) == future_timeouts_events.end()) {
		return NULL;
	}

	timeout_event *t = future_timeouts_events[seq];
	future_timeouts_events.erase(seq);
	sim->removeEvent(t); // this frees the memory
	return t;
}

void netflow::registerTimeoutAction(int seq, double time) {
	packet p = packet(FLOW, *this, seq);
	timeout_event *e = new timeout_event(time, *sim, *this, p);
	sim->addEvent(e);
	future_timeouts_events[seq] = e;
}

ack_event *netflow::cancelSendDuplicateAckAction(int seq) {

	if(future_send_ack_events.find(seq) == future_send_ack_events.end())
		return NULL;

	ack_event *e = future_send_ack_events.at(seq);
	future_send_ack_events.erase(seq);
	sim->removeEvent(e); // this frees the memory
	return e;
}

void netflow::registerSendDuplicateAckAction(int seq, double time) {
	packet p = packet(ACK, *this, seq);
	ack_event *e = new ack_event(time, *sim, *this, p);
	future_send_ack_events[seq] = e;
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

	
	cerr << "highest sent flow seqnum: " << highest_sent_flow_seqnum << endl;
	cerr << "window start: " << window_start << endl;
	cerr << "window end: " << window_start + window_size << endl;
	cerr << "num outstanding packets: " << outstanding_pkts.size() << endl;
	

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
	int i = 0;
	while(it != outstanding_pkts.end()) {

		// Store start time.
		rtts[it->getSeq()] = -start_time;

		// Make a timeout_event for this packet, store it in local map and
		// push it onto the simulation's event queue.
		registerTimeoutAction(it->getSeq(),
				linkFreeAt + timeout_length_ms + TIME_EPSILON * i++);
		it++;
	}

	highest_sent_flow_seqnum += outstanding_pkts.size();
	amt_sent_mb += ((double)packet::FLOW_PACKET_SIZE) / BYTES_PER_MEGABIT *
			outstanding_pkts.size();

	return outstanding_pkts;
}

void netflow::updateTimeouts(double end_time_ms, int flow_seqnum) {

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
}

void netflow::receivedAck(packet &pkt, double end_time_ms,
		double linkFreeAtTime) {

	assert(pkt.getType() == ACK);
	//assert(pkt.getSeq() >= highest_received_ack_seqnum);
	if (pkt.getSeq() < highest_received_ack_seqnum) {
		
		cerr << "Ack seq smaller than highest received seq " << endl;
		cerr << "Received ack: " << pkt.getSeq() << endl;
		cerr << "Highest: " << highest_received_ack_seqnum << endl;
		//exit(0);
	}

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
			int old_highest_sent = highest_sent_flow_seqnum;
			highest_sent_flow_seqnum = pkt.getSeq()-1;
			window_start = pkt.getSeq();

			cerr << "Window size (before): " << window_size << endl;
			window_size = window_size / 2 > 1 ? window_size / 2 : 1;
			cerr << "Window size (after): " << window_size << endl;

			lin_growth_winsize_threshold = window_size;
			num_duplicate_acks = 0;

			cancelTimeoutAction(pkt.getSeq() + 1);

			// TODO: revise timeout handling.
			
			// new timeout_event will be created when popOutstandingPackets
			// is called.
		}

		// Got a duplicate ACK but don't have enough of them to do a fast
		// retransmit. Push back the timeout event by canceling the old one
		// and registering a new one.
		// TODO: revise timeout handling

		else {
			cancelTimeoutAction(pkt.getSeq() + 1);
			registerTimeoutAction(pkt.getSeq() + 1,
					linkFreeAtTime + timeout_length_ms);
		}
	}

	// Otherwise if the ACK number is greater than the highest ACK we've seen we
	// had a successful transmission, so slide and grow the window, adjust the
	// average and std of RTTs so the timeout length can be set, and remove
	// the corresponding timeout events from the flow.
	else if (pkt.getSeq() > highest_received_ack_seqnum) {
		// Remove the timeout events for all packets between the last 
		// highest_received_ack_seqnum and the ack just received.	
		int prev_highest = highest_received_ack_seqnum;	

		for (int ack_seqnum = highest_received_ack_seqnum;
			 ack_seqnum < pkt.getSeq(); ack_seqnum++) {

			// Adjust avg and std of RTTs
			updateTimeouts(end_time_ms, ack_seqnum);

			// Remove events from the flow's map of future timeout events
			// and from the simulation's event queue
			cancelTimeoutAction(ack_seqnum);
		}

		// Update the last successfully received ack
		highest_received_ack_seqnum = pkt.getSeq();

		// Now adjust the window start and size. Since we just received
		// an ACK for a successfully received packet we can slide the window
		// to begin at its sequence number, and we can adjust the window size
		// by whether we're in exponential or linear growth mode.
		window_start = highest_received_ack_seqnum;

		increaseWindowSize(pkt.getSeq() - prev_highest);

	}
	
	// If the ACK number is less than the highest received ACK, it is the
	// result of leftover repeated-ACKs still being dequeued.
	// Drop them for now...
	else {

		/*
		// The corresponding FLOW packet had sequence number one less
		int flow_seqnum = pkt.getSeq() - 1;

		cancelAllTimeouts();

		updateTimeouts(end_time_ms, flow_seqnum);

		// Assume there was a dropped packet.
		window_size = (window_size / 2) > 1 ? (window_size / 2)  : 1;
		highest_sent_flow_seqnum = highest_received_ack_seqnum - 1;
		window_start = highest_sent_flow_seqnum + 1;
		*/
		return;
	}
	
}

void netflow::receivedFlowPacket(packet &pkt, double arrival_time) {

	assert(pkt.getType() == FLOW);
	cerr << "Received flow packet seq: " << pkt.getSeq() << endl;

	// Set the highest_received_flow_seqnum and make/send the corresponding
	// ACK packet. This should handle out-of-order FLOW packets.

	// Update the vector of received flow packets.
	received_flow_packets[pkt.getSeq()] = true;
	
	// Find the latest flow packet received IN ORDER. Start iterating at
	// the current highest received flow packet, until we reach something
	// not received or the end of the flow.
	int update_last_received = highest_received_flow_seqnum + 1;

	while (update_last_received < num_total_packets &&
		   received_flow_packets[update_last_received]) {
		highest_received_flow_seqnum = update_last_received;
		update_last_received++;
	}

	cerr << "Received highest consec. flow packet: " << highest_received_flow_seqnum << endl;
	// Send the corresponding acknowledgement.
	packet ackpack = packet(ACK, *this, highest_received_flow_seqnum + 1);

	// Make and queue (locally and on the simulation event queue) an immediate
	// duplicate_ack_event. When that event is run it will queue another,
	// future duplicate_ack_event in case the next FLOW packet never arrives.
	if (amt_sent_mb < size_mb) {
		registerSendDuplicateAckAction(ackpack.getSeq(), arrival_time);
	}

	// Remove the previous packet's corresponding duplicate ACK
	// send_packet_event from the local map and global events queue
	if (cancelSendDuplicateAckAction(ackpack.getSeq() - 1) == NULL && debug) {
		debug_os << "No pending duplicate_ack_event to delete." << endl;
	}
}

double netflow::getTimeoutLengthMs() const { return timeout_length_ms; }

void netflow::cancelAllTimeouts() {
	map<int, timeout_event *>::iterator it;
	for (it = future_timeouts_events.begin();
			it != future_timeouts_events.end(); it++) {
		sim->removeEvent(it->second);
	}
	future_timeouts_events.clear();
}

void netflow::timeoutOccurred(const packet &to_pkt) {
	lin_growth_winsize_threshold = window_size / 2;
	window_size = 1;
	window_start = to_pkt.getSeq();
	num_duplicate_acks = 0;
	rtts.clear();
	highest_sent_flow_seqnum = to_pkt.getSeq() - 1;
	highest_received_ack_seqnum = to_pkt.getSeq();
	cancelAllTimeouts();
}

void netflow::increaseWindowSize(int num_acks_received) {

	if (lin_growth_winsize_threshold < 0) { // hasn't been initialized, so
										    // just do exponential growth
		window_size += num_acks_received;
	}
	else if (window_size < lin_growth_winsize_threshold) {
		// Increase exponentially until linear growth threshold is reached.
		// Then increase linearly.
		if (window_size + num_acks_received < lin_growth_winsize_threshold)
			window_size += num_acks_received;
		else {
			int linear_growth = window_size + num_acks_received - 
				lin_growth_winsize_threshold;
			window_size = lin_growth_winsize_threshold;
			for (int i = 0; i < linear_growth; i++) {
				window_size += 1 / window_size;
			}
		}

	}
	else { // linear growth part
		for (int i = 0; i < num_acks_received; i++) {
			window_size += 1 / window_size;
		}
	}
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

double netlink::getArrivalTime(const packet &pkt, bool useDelay, double time) {
	return (useDelay ? getDelay() : 0) + getTransmissionTimeMs(pkt) +
			(buffer.size() > 0 ? buffer.rbegin()->first : time);
}

bool netlink::sendPacket(const packet &pkt, bool useDelay, double time) {

	// Check if the buffer has space. If it doesn't then return false.
	if (getBufferOccupancy() + pkt.getSizeBytes() > buffer_capacity) {
		return false;
	}
	buffer[getArrivalTime(pkt, useDelay, time)] = pkt;
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
