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
#include "netlink.h"
#include "netdevice.h"

using namespace std;

/**
 * Represents a router in a simple network. TODO add detail to this comment.
 */
class netrouter : public netdevice {

private:

	// TODO add reference to parent simulation

	/** Pointers to all the links attached to this router. */
	vector<netlink *> links;

	/** Routing table implemented as map from destination names to links. */
	map<string, netlink *> rtable;

public:

	netrouter (string name) : netdevice(name) { }

	netrouter (string name, vector<netlink *> links) :
		netdevice(name), links(links) { }

	void addLink (netlink &link) {
		links.push_back(&link);
	}

	const vector<netlink *> &getLinks() const {
		return links;
	}

	/**
	 * Print helper function which partially overrides the one in @c netdevice.
	 * @param os The output stream to which to write.
	 */
	virtual void printHelper(ostream &os) const {
		netdevice::printHelper(os);
		os << " ---> [router. links: [ ";
		for (unsigned int i = 0;
				i < (links.size() == 0 ? 0 : links.size() - 1); i++) {
			os << links[i]->getName() << ", ";
		}
		if (links.size() > 0) {
			os << links[links.size() - 1]->getName();
		}
		os << " ], routing table: [ ";

		bool first = true;
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
