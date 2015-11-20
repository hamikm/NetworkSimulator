/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef NETNODE_H
#define NETNODE_H

#include <iostream>
#include <cassert>
#include <string>
#include <map>
#include <iterator>
#include <vector>

#include "netelement.h"
#include "netlink.h"

using namespace std;

extern bool debug;
extern ostream &debug_os;

/**
 * Represents a node, which is either a node or a host, in a simple network.
 * TODO add detail to this comment.
 */
class netnode : public netelement {

protected:

	/** Pointers to all the links attached to this router. */
	vector<netlink *> links;

public:

	netnode (string name) : netelement(name) { }

	netnode (string name, vector<netlink *> links) :
		netelement(name), links(links) { }

	virtual void addLink (netlink &link) { links.push_back(&link); }

	const vector<netlink *> &getLinks() const { return links; }

	/**
	 * Print helper function. Partially overrides superclass's.
	 * @param os The output stream to which to write netdevice information.
	 */
	virtual void printHelper(std::ostream &os) const;
};

#endif // NETNODE_H
