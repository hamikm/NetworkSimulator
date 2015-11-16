/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef NETHOST_H
#define NETHOST_H

#include <iostream>
#include <cassert>
#include <string>

#include "netelement.h"
#include "netlink.h"

using namespace std;

extern bool debug;
extern ostream &debug_os;

/**
 * Represents a host in a simple network. TODO add detail to this comment.
 */
class nethost : public netelement {

private:

	netlink *link;

public:

	nethost (string name) : netelement(name) {
		link = NULL;
	}

	nethost (string name, netlink &link) : netelement(name), link(&link) { }

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
		netelement::printHelper(os);
		os << " ---> [host. link: "
				<< (link == NULL ? "NULL" : link->getName()) << "]";
	}
};

#endif // NETHOST_H
