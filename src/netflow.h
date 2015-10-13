/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef NETFLOW_H
#define NETFLOW_H

#include <iostream>
#include <cassert>
#include <string>
#include <map>
#include "nethost.h"
#include "netdevice.h"

using namespace std;

/**
 * Represents a flow in a simple network. TODO add detail to this comment.
 */
class netflow : public netdevice {

private:

	/** Start time in seconds from beginning of simulation. */
	float start_time;

	/** Transmission size in mb. */
	float size;

	/** Pointer to one end of this link. */
	nethost *source;

	/** Pointer to the other end of this link. */
	nethost *destination;

	// TODO add reference to parent simulation

	// TODO add last successfully sent packet number

	// TODO add total number of packets

public:

	netflow (string name, float start_time, float size,
			nethost &source, nethost &destination) :
				netdevice(name), start_time(start_time), size(size),
				source(&source), destination(&destination) { }

	netflow (string name, float start_time, float size) :
			netdevice(name), start_time(start_time), size(size) {
		source = NULL;
		destination = NULL;
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
		netdevice::printHelper(os);
		os << " ---> [flow. start time: " << start_time << ", size: " << size
				<< ", source: "
				<< (source == NULL ? "NULL" : source->getName())
				<< ", destination: "
				<< (destination == NULL ? "NULL" : destination->getName())
				<< "]";
	}
};

#endif // NETFLOW_H
