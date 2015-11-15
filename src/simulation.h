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

// Libraries.
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

// Custom headers.
#include "netelement.h"
#include "nethost.h"
#include "netrouter.h"
#include "netlink.h"
#include "netflow.h"
#include "event.h"
#include <vector>
#include <queue>

using namespace std;
using namespace rapidjson;

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

	/** Event queue. */
	priority_queue<event, vector<event>, eventTimeSorter> events;

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
};

#endif // SIMULATION_H
