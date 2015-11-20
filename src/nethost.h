/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef NETHOST_H
#define NETHOST_H

#include <iostream>
#include <cassert>
#include <string>

#include "netnode.h"

using namespace std;

extern bool debug;
extern ostream &debug_os;

/**
 * Represents a host in a simple network. TODO add detail to this comment.
 */
class nethost : public netnode {

private:

public:

	nethost (string name) : netnode(name) { }

	nethost (string name, netlink &link) : netnode(name) {
		addLink(link);
	}

	/**
	 * Gets the first (and only, since this is a host) link.
	 * @return link attached to this host
	 */
	netlink *getLink() const {
		if(getLinks().size() == 0)
			return NULL;
		return getLinks()[0];
	}

	/**
	 * Deletes all links then adds the given one.
	 * @param link link to add after deleteing the others
	 */
	void setLink(netlink &link) {
		links.clear();
		addLink(link);
	}

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const {
		netnode::printHelper(os);
		os << " ---> [host. link: "
				<< (getLink() == NULL ? "NULL" : getLink()->getName()) << "]";
	}
};

#endif // NETHOST_H
