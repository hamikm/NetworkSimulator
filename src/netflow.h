/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef NETFLOW_H
#define NETFLOW_H

// Standard includes
#include <iostream>
#include <cassert>
#include <string>
#include <map>

// Custom headers
#include "netelement.h"
#include "nethost.h"

// Forward declarations
class timeout_event;
class send_ack_event;

using namespace std;

extern bool debug;
extern ostream &debug_os;

/**
 * Represents a flow in a simple network. TODO add detail to this comment.
 */
class netflow : public netelement {

private:

	/** Start time in seconds from beginning of simulation. */
	double start_time;

	/** Transmission size in megabits. */
	float size;

	/** Pointer to one end of this link. */
	nethost *source;

	/** Pointer to the other end of this link. */
	nethost *destination;

	/**
	 * Just the flow size divided by the size of each packet.
	 */
	int num_total_packets;

	/**
	 * Sequence number of the last ACK received by the source host.
	 */
	int last_received_ack_seqnum;

	/**
	 * The TCP protocol's window size.
	 */
	int window_size;

	/**
	 * Number of duplicate ACKs seen.
	 */
	int num_duplicate_acks;

	/**
	 * This is a map from sequence numbers of packets to their future,
	 * corresponding timeout events. Each time a send_packet_event is pushed
	 * onto the global events queue a corresponding future timeoutout_event is
	 * also pushed. If an ACK for the packet sent through the send_packet_event
	 * is received before that packet's timeout_event has been processed
	 * then it's the receive_ack_event's job to remove the corresponding
	 * timeout_event from the global events queue by looking up the
	 * timeout_event in this map then using the pointer to delete from events.
	 */
	map<int, timeout_event *> future_timeouts_events;

	/**
	 * Every time a send_ack_event is pushed onto the global events queue we
	 * also push another send_ack_event (for a later time) onto the events
	 * queue. The second event is for duplicate acks. If a future
	 * receive_packet_event runs before the duplicate send_ack_event for
	 * the previous packet, the duplicate send_ack_event must be looked
	 * up in this map then deleted from the global events queue.
	 */
	map<int, send_ack_event *> future_send_ack_events;

	/**
	 * This is the maximum length of time in milliseconds that can pass between
	 * a send_packet_event and its corresponding receive_ack_event before
	 * a timeout_event will run. This timeout length is be dynamically
	 * readjusted with the (recursively determined) average RTT and
	 * deviation of RTT.
	 */
	double timeout_length;

	void constructorHelper (double start_time, float size, nethost &source,
			nethost &destination, int num_total_packets,
			int last_received_ack_seqnum, int window_size,
			int num_duplicate_acks, double timeout_length) {
		this->start_time = start_time;
		this->size = size;
		this->source = &source;
		this->destination = &destination;
		this->num_total_packets = num_total_packets;
		this->last_received_ack_seqnum = last_received_ack_seqnum;
		this->window_size = window_size;
		this->num_duplicate_acks = num_duplicate_acks;
		this->timeout_length = timeout_length;
	}

public:

	/**
	 * Size of each packet's payload in bytes.
	 */
	static const int PACKET_PAYLOAD_SIZE = 1024;

	/**
	 * The timeout value for the very first packet sent in a flow; the
	 * timeout value is later seeded with the first packet's RTT and then
	 * changed with recursive average and deviation algorithms.
	 */
	static const double DEFAULT_INITIAL_TIMEOUT = 1000.0;

	netflow (string name, float start_time, float size,
			nethost &source, nethost &destination) :
				netelement(name) {
		constructorHelper(start_time, size, source, destination,
				size / PACKET_PAYLOAD_SIZE + 1, 0, 1, 0,
				DEFAULT_INITIAL_TIMEOUT);
	}

	nethost *getDestination() const {
		return destination;
	}

	void setDestination(nethost &destination) {
		this->destination = &destination;
	}

	float getSize() const {
		return size;
	}

	nethost *getSource() const {
		return source;
	}

	void setSource(nethost &source) {
		this->source = &source;
	}

	float getStartTime() const {
		return start_time;
	}

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 *
	 * TODO if more instance variables are added might want to add them to this
	 * printer for debugging help.
	 */
	virtual void printHelper(ostream &os) const {
		netelement::printHelper(os);
		os << " ---> [flow. start time: " << start_time << ", size: " << size
				<< ", source: "
				<< (source == NULL ? "NULL" : source->getName())
				<< ", destination: "
				<< (destination == NULL ? "NULL" : destination->getName())
				<< "]";
	}
};

#endif // NETFLOW_H
