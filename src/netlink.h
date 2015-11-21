/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef NETLINK_H
#define NETLINK_H

#include <iostream>
#include <cassert>
#include <string>
#include <map>

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

private:

	/** This link's rate in bytes per second. */
	long rate;

	/** Signal propagation delay for this link in ms. */
	int delay;

	/** Buffer size in bytes. */
	int buflen;

	/** Pointer to one end of this link. */
	netelement *endpoint1;

	/** Pointer to the other end of this link. */
	netelement *endpoint2;

	/** Number of bytes in use in the buffer. */
	long buf_occupancy;

	/** The link will be free after this time. */
	double available_at_time;

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
	long getBuflen() const { return buflen; }

	/**  @return buffer length in kilobytes. */
	long getBuflenKB() const { return buflen / BYTES_PER_KB; }

	/** @return delay in milliseconds. */
	int getDelay() const { return delay; }

	netelement *getEndpoint1() const { return endpoint1; }

	void setEndpoint1(netelement &endpoint1) { this->endpoint1 = &endpoint1; }

	netelement *getEndpoint2() const { return endpoint2; }

	void setEndpoint2(netelement &endpoint2) { this->endpoint2 = &endpoint2; }

	/** @return link rate in bytes per second. */
	long getRate() const { return rate; }

	/** @return link rate in megabits per second. */
	float getRateMbps() const { return ((float) rate) / BYTES_PER_MEGABIT; }

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const {
		netelement::printHelper(os);
		os << " ---> [link. rate(mbps): " << getRateMbps() << ", delay: "
				<< delay << ", buffer length(kB): " << getBuflenKB()
				<< ", endpoint 1: "
				<< (endpoint1 == NULL ? "NULL" : endpoint1->getName())
				<< ", endpoint 2: "
				<< (endpoint2 == NULL ? "NULL" : endpoint2->getName()) << "]";
	}
};

#endif // NETLINK_H
