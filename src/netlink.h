/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef NETLINK_H
#define NETLINK_H

// Standard includes
#include <iostream>
#include <cassert>
#include <string>
#include <map>

// Custom headers
#include "netelement.h"


using namespace std;

extern bool debug;
extern ostream &debug_os;

/**
 * Represents a link in a simple network. TODO add detail to this comment.
 */
class netlink : public netelement {

public:

	/**
	 * Input buffer size is in kilobytes, so this is used to convert it to
	 * bytes.
	 */
	static const int BYTES_PER_KB = 1 << 10;

	/**  Conversion factor between kilobytes and megabytes. */
	static const int KB_PER_MB = 1 << 10;

	/**  Conversion factor between bits and bytes. */
	static const int BITS_PER_BYTE = 1 << 3;

	/** Input rate is in mbps, so this is used to convert to bytes per sec. */
	static const int BYTES_PER_MEGABIT =
			BYTES_PER_KB * KB_PER_MB / BITS_PER_BYTE;

	/** Milliseconds per second. */
	static const int MS_PER_SEC = 1000;

private:

	/** This link's rate in bytes per millisecond. */
	double rate_bpms;

	/** Signal propagation delay for this link in ms. */
	int delay_ms;

	/** Buffer size in bytes. */
	int buflen_bytes;

	/** Pointer to one end of this link. */
	netelement *endpoint1;

	/** Pointer to the other end of this link. */
	netelement *endpoint2;

	/** The link will be free after this (absolute, not relative) time (ms). */
	double available_at_time_ms;

	/**
	 * Helper for the constructors. Converts the buffer length from kilobytes
	 * to bytes and the rate from megabits per second to bytes per second.
	 */
	void constructor_helper(float rate_mbps, int delay, int buflen_kb,
			netelement *endpoint1, netelement *endpoint2);

public:

	/**
	 * @param name of this link
	 * @param rate_mbps link rate in megabits per second
	 * @param delay link delay in milliseconds
	 * @param buflen_kb the size of the only buffer on this link in kilobytes.
	 * This is stored internally in bytes.
	 * @param endpoint1 the host or router on one side of this link
	 * @param endpoint2 the host or router on the other side of this link
	 */
	netlink(string name, float rate_mbps, int delay, int buflen_kb,
			netelement &endpoint1, netelement &endpoint2) : netelement(name) {
		constructor_helper(
				rate_mbps, delay, buflen_kb, &endpoint1, &endpoint2);
	}

	/**
	 * Sets the endpoints of this link to NULL.
	 * @param name of this link
	 * @param rate_mbps link rate in megabits per second
	 * @param delay link delay in milliseconds
	 * @param buflen_kb the size of the only buffer on this link in kilobytes
     * This is stored internally in bytes.
	 */
	netlink (string name, float rate_mbps, int delay, int buflen_kb) :
			 netelement(name) {
		constructor_helper(rate_mbps, delay, buflen_kb, NULL, NULL);
	}

	/** @return buffer length in bytes. */
	long getBuflen() const { return buflen_bytes; }

	/**  @return buffer length in kilobytes. */
	long getBuflenKB() const { return buflen_bytes / BYTES_PER_KB; }

	/** @return delay in milliseconds. */
	int getDelay() const { return delay_ms; }

	netelement *getEndpoint1() const { return endpoint1; }

	void setEndpoint1(netelement &endpoint1) { this->endpoint1 = &endpoint1; }

	netelement *getEndpoint2() const { return endpoint2; }

	void setEndpoint2(netelement &endpoint2) { this->endpoint2 = &endpoint2; }

	/** @return link rate in bytes per second. */
	long getRate() const { return rate_bpms * MS_PER_SEC; }

	/** @return link rate in megabits per second. */
	float getRateMbps() const {
		return ((float) rate_bpms) / BYTES_PER_MEGABIT * MS_PER_SEC;
	}

	/**
	 * Returns the buffer occupancy at a particular time.
	 * @param current_time time at which invoking event occurs
	 * @return number of bytes in use in the buffer (assuming a magically
	 * unfragmentable buffer)
	 */
	long getBufferOccupancy(double current_time) const {
		return (available_at_time_ms - current_time) - delay_ms * rate_bpms;
	}

	/**
	 * If the link buffer has space the link's future availability time
	 * @c available_at_time is updated, as is the buffer occupancy. If the
	 * buffer can't accommodate this packet then neither the
	 * @c aavailable_at_time variable nor the buffer occupancy
	 * @c buf_occupancy is updated, which models the behavior of packet
	 * dropping.
	 * @param pkt the packet to send
	 * @param current_time the simulation's global current time
	 */
	//double sendPacket(packet pkt, double current_time); TODO

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const {
		netelement::printHelper(os);
		os << " ---> [link. rate(mbps): " << getRateMbps() << ", delay: "
				<< delay_ms << ", buffer length(kB): " << getBuflenKB()
				<< ", endpoint 1: "
				<< (endpoint1 == NULL ? "NULL" : endpoint1->getName())
				<< ", endpoint 2: "
				<< (endpoint2 == NULL ? "NULL" : endpoint2->getName()) << "]";
	}
};

#endif // NETLINK_H
