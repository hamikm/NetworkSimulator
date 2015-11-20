/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef NETROUTER_H
#define NETROUTER_H

#include <iostream>
#include <cassert>
#include <string>
#include <map>
#include <iterator>
#include <vector>

#include "netnode.h"

using namespace std;

extern bool debug;
extern ostream &debug_os;

/**
 * Represents a router in a simple network. TODO add detail to this comment.
 */
class netrouter : public netnode {

private:

	/** Routing table implemented as map from destination names to links. */
	map<string, netlink *> rtable;

public:

	netrouter (string name) : netnode(name) { }

	netrouter (string name, vector<netlink *> links) :
		netnode(name, links) { }

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const {
		netnode::printHelper(os);
		bool first = true;
		os << " ---> [router. routing table: [ ";
		map<string, netlink *>::const_iterator itr;
		for (itr = rtable.begin(); itr != rtable.end(); itr++) {
			if (first) {
				os << itr->first << "-->" << itr->second;
				first = false;
				continue;
			}
			os << ", " << itr->first << "-->" << itr->second;
		}
		os << " ]";
	}
};

#endif // NETROUTER_H
