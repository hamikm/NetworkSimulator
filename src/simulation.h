/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef SIMULATION_H
#define SIMULATION_H

// Standard headers.
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <cassert>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <queue>

// Libraries.
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

// Forward declarations.
class event;
class eventTimeSorter;
class nethost;
class netrouter;
class netlink;
class netflow;

// Custom headers.
#include "events.h"

using namespace std;
using namespace rapidjson;

extern bool debug;
extern ostream &debug_os;

/**
 * Comparison functor for use in container templates like @c priority_queue.
 * Sorts on events' times.
 */
class eventTimeSorter {

public:

	/**
	 * Comparison function; compares on times.
	 * @param e1
	 * @param e2
	 * @return true if time of @c e1 is less than time of @c e2.
	 */
	bool operator() (const event &e1, const event &e2);
};

/**
 * Represents the simulation. TODO add details.
 */
class simulation {

private:

	/** All hosts in network. */
	map<string, nethost *> hosts;

	/** All routers in network. */
	map<string, netrouter *> routers;

	/** All links in network. */
	map<string, netlink *> links;

	/** All flows in network. */
	map<string, netflow *> flows;

	/** Event queue (implemented with a multimap which is sorted by key).
	 * Keys represent time in milliseconds. */
	multimap<double, event> events;

	/**
	 * Helper for the destructor.
	 *
	 * TODO make sure to update this function if other stuff is stuff is
	 * dynamically allocated.
	 */
	void free_network_devices ();

public:

	/**
	 * Parses the JSON file stored at @c inputfile and populates in-memory
	 * hosts, routers, links, and flows.
	 * @param inputfile JSON filename. Points to description of network.
	 */
	simulation (const char *inputfile);

	/**
	 * Default constructor which does nothing. Might be used in tests.
	 * @param inputfile JSON filename. Points to description of network.
	 */
	simulation ();

	/**
	 * Deletes all the dynamically allocated network objects like hosts,
	 * routers, and so forth.
	 */
	~simulation ();

	/**
	 * Takes the full JSON string passed in, parses it, and fills the in-memory
	 * collections of hosts, routers, links, and flows.
	 * @param jsonstring full JSON string consisting of network description
	 * @post STL collections of hosts, routers, links, and flows are filled in
	 * @warning routing tables ARE NOT INITIALIZED HERE.
	 */
	void parse_JSON_input (string jsonstring);

	/**
	 * Prints hosts, routers, links, and flows to given output stream.
	 * @param os the output stream to which to print.
	 */
	void print_network(ostream &os) const;

	/**
	 * Runs the simulation by loading some initial events into the @c events
	 * queue then starts a loop over queue events, calling the @c runEvent
	 * function on each of them, and continues until the @c events queue
	 * empties. Note that each event can (1) modify the host, flow, router,
	 * or link data structures in this simulation object, (2) add new events
	 * to this simulation object's event queue, or (3) log data into this
	 * simulation object's data logger object. This function is not responsible
	 * for writing the data logger's data to disk--the caller is.
	 */
	void runSimulation();

	/**
	 * Adds an event to the simulation's event queue. Event objects have a
	 * reference to this simulation so they can add events they need to
	 * generate by invoking this function.
	 * @param e event to add to the simulation's @c events queue
	 */
	void addEvent(event &e);

	/**
	 * Removes the given event from the simulation's event map.
	 * @param e event to remove
	 */
	void removeEvent(event &e);
};

#endif // SIMULATION_H
