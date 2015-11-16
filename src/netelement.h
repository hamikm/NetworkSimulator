/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef NETDEVICE_H
#define NETDEVICE_H

#include <iostream>
#include <cassert>
#include <string>
#include <map>
#include <ostream>

using namespace std;

extern bool debug;
extern ostream &debug_os;

/**
 * Superclass of all network elements including routers, hosts, links, packets,
 * flows. "Device" is a bit of a misnomer for the latter but they all share a
 * superclass so they can share an output operator overload and so they can
 * co-exist in the same STL collection if necessary.
 */
class netelement {

private:

	/** E.g. a router or host name like "H1". */
	string name;

	// TODO reference to simulation (or some data logger object?) for data...

public:

	netelement (string name) : name(name) { }

	virtual ~netelement() { }

	const string &getName() const {
		return name;
	}

	/**
	 * Print helper function. Derived classes should (partially) override this.
	 * @param os The output stream to which to write netdevice information.
	 */
	virtual void printHelper(std::ostream &os) const {
		os << "[device. name: " << name << "]";
	}
};

/**
 * Output operator override for printing contents of the given netelement
 * to an output stream. Uses the printHelper function, which is virtual
 * because derived classes will want to modify or enhance printing behavior.
 * @param os The output stream to which to write.
 * @param device The @c netdevice object to write.
 * @return The same output stream for operator chaining.
 */
inline ostream & operator<<(ostream &os, const netelement &device) {
	device.printHelper(os);
	return os;
}

#endif // NETDEVICE_H
