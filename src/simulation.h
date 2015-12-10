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
/* NOTE: rapidjson is used for parsing input while
 *       "JSON for Modern C++" (found here: https://github.com/nlohmann/json)
 *       is used to parse and format .json logger file.
 */
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "json.hpp"

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
using namespace nlohmann;     // necessary to use "json.h"

extern bool debug;
extern bool detail;
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
 * Represents the simulation.
 * Sets up network based on .json input file and runs network simulation. TCP protocol
 * to use indicated as flow parameter in .json input file.
 * Each simulation object has an associated logger file to which simulation metrics
 * are written.
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

	/**
	 * Event queue (implemented with a multimap which is sorted by key).
	 * Keys represent time in milliseconds.
	 */
	multimap<double, event *> events;

	/** Name of file to which simulation metrics are logged */
	string logName;

	/** Keeps track of how many events have been executed; Used in logging */
	int eventCount = 0;

	/** Helper for the destructor. */
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
	 * Getter for the string to host-pointer map.
	 * @return hosts
	 */
	map<string, nethost *> getHosts() const;

	/**
	 * Getter for the string to router-pointer map.
	 * @return router
	 */
	map<string, netrouter *> getRouters() const;

	/**
	 * Runs the simulation by loading some initial events into the @c events
	 * queue then starts a loop over the events, calling the @c runEvent
	 * function on each of them, and continues until the @c events queue
	 * empties. Note that each event can (1) modify the host, flow, router,
	 * or link data structures in this simulation object, (2) add new events
	 * to this simulation object's event queue, or (3) log data into this
	 * simulation object's related log file. This function is not responsible
	 * for writing the data logger's data to disk--the caller (individual event)
	 * is.
	 */
	void runSimulation();

	/**
	 * Adds an event to the simulation's event queue. Event objects have a
	 * reference to this simulation so they can add events they need to
	 * generate by invoking this function.
	 * @param e event to add to the simulation's @c events queue
	 */
	void addEvent(event *e);

	/**
	 * Removes the given event from the simulation's event map.
	 * @param e event to remove
	 */
	void removeEvent(event *e);

	// SIMULATION LOGGER RELATED FUNCTIONS
	
	/**
	 * Helper function for logging once every LOG_FREQUENCY.
	 * @return eventCount counter for number of events logged so far
	 */
	int getEvtCount() const;

	/**
	 * Getter for the logger filename
	 * @return logName name of log file
	 */
	string getLogName() const;

	/**
	 * Initializes and sets up a data log for the simulation object
	 * by creating new file called "filename.json" and adds the first line.
	 * @param filename name to give to log file
	 * @return 0 returned if successful
	 */
	int initializeLog(string filename);

	/**
	 * After simulation has finished i.e. all events have logged data
	 * adds the last line to make the logger a valid JSON file.
	 * @return 0 returned if successful
	 */
	int closeLog();
	
	/**
	 * Called everytime an event is run/"popped".
	 * Sweeps for all relevant metrics of time currTime stored in event object
	 * and then writes to file.
	 * @param currTime occurrance time of event currently being logged
	 * @return 0 returned if successful
	 */
	int logEvent(double currTime);

	/**
	 * Helper function to logEvent.
	 * Appends a ',' before appending a .json formatted event metric 
	 * to a logger file, except for the first event logged.
	 * @param event current event being logged
	 * @param logger file to log data into
	 */
	void appendEventMetric(json event, ofstream& logger);

	/**
	 * Helper function to logEvent.
	 * Retrieves link ID, link rate, link buffer occupancy, and packet loss of a
	 * single link and formats metrics into json.
	 * @param link
	 * @param currTime occurrance time of event currently being logged
	 * @param returns metrics for input link in JSON format
	 */
	json logLinkMetric(netlink link, double currTime);

	/**
	 * Helper function to logEvent.
	 * Retrieves flow ID, flow rate, window size, and packet delay of a single
	 * flow, and formats metrics into json.
	 * @param flow
	 * @param currTime occurrance time of event currently being logged
	 * @param returns metrics for input flow in JSON format
	 */
	json logFlowMetric(netflow flow, double currTime);
};

#endif // SIMULATION_H
