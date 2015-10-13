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

/**
 * Superclass of all network elements including routers, hosts, links, and
 * flows. "Device" is a bit of a misnomer for the latter but they all share a
 * superclass so they can share an output operator overload and so they can
 * co-exist in the same STL collection if necessary. Ideally each type of
 * netdevice would exist in its own generic-using collection though.
 */
class netdevice {

private:

	string name;

public:

	netdevice (string name) : name(name) { }

	virtual ~netdevice() { }

	const string &getName() const {
		return name;
	}

	/**
	 * Print helper function. Derived classes should (partially) override this.
	 * @param os The output stream to which to write scene object information.
	 */
	virtual void printHelper(std::ostream &os) const {
		os << "[device. name: " << name << "]";
	}
};

/**
 * Output operator override for printing contents of the given object
 * to an output stream. Uses the printHelper function, which is virtual
 * because derived classes will want to modify or enhance printing behavior.
 * @param os The output stream to which to write.
 * @param device The @c netdevice object to write.
 * @return The same output stream for operator chaining.
 */
inline ostream & operator<<(ostream &os, const netdevice &device) {
	device.printHelper(os);
	return os;
}

#endif // NETDEVICE_H
