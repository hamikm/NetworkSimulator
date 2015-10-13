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
#include "netdevice.h"

using namespace std;

/**
 * Represents a link in a simple network. TODO add detail to this comment.
 */
class netlink : public netdevice {

private:

	/** This link's rate in mbps. */
	float rate;

	/** Signal propagation delay for this link in ms. */
	float delay;

	/** Buffer size in kb. */
	long buflen;

	/** Pointer to one end of this link. */
	netdevice *endpoint1;

	/** Pointer to the other end of this link. */
	netdevice *endpoint2;

	// TODO add reference to parent simulation

public:

	netlink (string name, float rate, float delay, long buflen,
			netdevice &endpoint1, netdevice &endpoint2) :
				netdevice(name), rate(rate), delay(delay), buflen(buflen),
				endpoint1(&endpoint1), endpoint2(&endpoint2) { }

	netlink (string name, float rate, float delay, long buflen) :
			netdevice(name), rate(rate), delay(delay), buflen(buflen) {
		endpoint1 = NULL;
		endpoint2 = NULL;
	}

	long getBuflen() const {
		return buflen;
	}

	float getDelay() const {
		return delay;
	}

	netdevice *getEndpoint1() const {
		return endpoint1;
	}

	void setEndpoint1(netdevice &endpoint1) {
		this->endpoint1 = &endpoint1;
	}

	netdevice *getEndpoint2() const {
		return endpoint2;
	}

	void setEndpoint2(netdevice &endpoint2) {
		this->endpoint2 = &endpoint2;
	}

	float getRate() const {
		return rate;
	}

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const {
		netdevice::printHelper(os);
		os << " ---> [link. rate: " << rate << ", delay: " << delay
				<< ", buffer length: " << buflen << ", endpoint 1: "
				<< (endpoint1 == NULL ? "NULL" : endpoint1->getName())
				<< ", endpoint 2: "
				<< (endpoint2 == NULL ? "NULL" : endpoint2->getName()) << "]";
	}
};

#endif // NETLINK_H
