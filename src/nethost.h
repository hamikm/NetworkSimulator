/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#include <iostream>
#include <cassert>
#include <string>
#include "netlink.h"
#include "netdevice.h"

using namespace std;

#ifndef NETHOST_H
#define NETHOST_H

/**
 * Represents a host in a simple network. TODO add detail to this comment.
 */
class nethost : public netdevice {

private:

	netlink *link;

	// TODO add reference to parent simulation

public:

	nethost (string name) : netdevice(name) {
		link = NULL;
	}

	nethost (string name, netlink &link) : netdevice(name), link(&link) { }

	virtual bool isHost() {
		return true;
	}

	virtual bool isRouter() {
		return false;
	}

	void setLink(netlink &link) {
		this->link = &link;
	}

	netlink *getLink() const {
		return link;
	}

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const {
		netdevice::printHelper(os);
		os << " ---> [host. link: "
				<< (link == NULL ? "NULL" : link->getName()) << "]";
	}
};

#endif // NETHOST_H
