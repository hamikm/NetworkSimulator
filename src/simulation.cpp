/*
 * See header file for function comments.
 */

#include "simulation.h"

bool eventTimeSorter::operator() (const event &e1, const event &e2) {
	return e1.getTime() < e2.getTime();
}

simulation::simulation () {}

simulation::simulation (const char *inputfile) {

	// Read JSON file into a single string.
	string jsonstr;
	stringstream sstr;
	ifstream inputfilestream (inputfile);
	string line;
	if (inputfilestream.is_open()) {
		while (getline(inputfilestream, line)) {
			sstr << line << endl;
		}
		inputfilestream.close();
	}
	jsonstr = sstr.str();

	// Send the JSON string to a parser. Populate in-memory collections of
	// hosts, routers, links, and flows.
	parse_JSON_input (jsonstr);
}

simulation::~simulation () {
	free_network_devices();
}

void simulation::parse_JSON_input (string jsonstring) {

	// Parse JSON text into a document.
    Document document;
    char buffer[jsonstring.length() + 1];
    memcpy(buffer, jsonstring.c_str(), jsonstring.length());
    buffer[jsonstring.length()] = 0; // string has to be null-terminated
    assert(!document.ParseInsitu(buffer).HasParseError());

    // Load the hosts into memory.
    const Value& texthosts = document["hosts"];
	assert(texthosts.IsArray());
	for (SizeType i = 0; i < texthosts.Size(); i++) {
		string hostname(texthosts[i].GetString());
		nethost *curr_host = new nethost(hostname);
		hosts[hostname] = curr_host;
	}

	// Load the routers into memory
    const Value& textrouters = document["routers"];
	assert(textrouters.IsArray());
	for (SizeType i = 0; i < textrouters.Size(); i++) {
		string routername(textrouters[i].GetString());
		netrouter *curr_router = new netrouter(routername);
		routers[routername] = curr_router;
	}

	// Load the links into memory
    const Value& textlinks = document["links"];
	assert(textlinks.IsArray());
	for (SizeType i = 0; i < textlinks.Size(); i++) {
		assert(textlinks[i].IsObject());
		const Value& thislink = textlinks[i];
		string linkname = thislink["id"].GetString();

		string endpt1name = thislink["endpt_1"].GetString();
		string endpt2name = thislink["endpt_2"].GetString();

		netnode *endpoint1;
		bool endpt1IsHost = false;
		netnode *endpoint2;
		bool endpt2IsHost = false;

		// if this endpoint is a host
		if (hosts.find(endpt1name) != hosts.end()) {
			endpoint1 = hosts[endpt1name];
			endpt1IsHost = true;
		}
		// if this endpoint is a router
		else if (routers.find(endpt1name) != routers.end()) {
			endpoint1 = routers[endpt1name];
		}
		// if this endpoint is neither a router nor a host...
		else {
			// ...that should never happen!
			assert(false);
		}

		// if this endpoint is a host
		if (hosts.find(endpt2name) != hosts.end()) {
			endpoint2 = hosts[endpt2name];
			endpt2IsHost = true;
		}
		// if this endpoint is a router
		else if (routers.find(endpt2name) != routers.end()) {
			endpoint2 = routers[endpt2name];
		}
		// if this endpoint is neither a router nor a host...
		else {
			// ...that should never happen!
			assert(false);
		}

		netlink *curr_link =
				new netlink (linkname,
						(float) thislink["rate"].GetDouble(),
						(float) thislink["delay"].GetDouble(),
						(long) thislink["buf_len"].GetInt64(),
						*endpoint1, *endpoint2);

		// If this link is connected to a host, put reference to it in host.
		if (endpt1IsHost) {
			nethost *thishost = dynamic_cast<nethost *>(endpoint1);
			// each host must have exactly 1 link
			assert(thishost->getLink() == NULL);
			thishost->setLink(*curr_link);
		}
		if (endpt2IsHost) {
			nethost *thishost = dynamic_cast<nethost *>(endpoint2);
			// each host must have exactly 1 link
			assert(thishost->getLink() == NULL);
			thishost->setLink(*curr_link);
		}

		// If this link is connected to a router, add link to router.
		if (!endpt1IsHost) {
			netrouter *thisrouter = dynamic_cast<netrouter *>(endpoint1);
			thisrouter->addLink(*curr_link);
		}
		if (!endpt2IsHost) {
			netrouter *thisrouter = dynamic_cast<netrouter *>(endpoint2);
			thisrouter->addLink(*curr_link);
		}
		links[linkname] = curr_link;
	}

	// Load the flows into memory.
    const Value& textflows = document["flows"];
	assert(textflows.IsArray());
	for (SizeType i = 0; i < textflows.Size(); i++) {
		assert(textflows[i].IsObject());
		const Value& thisflow = textflows[i];
		string flowname = thisflow["id"].GetString();

		// Make sure that the source and destination of this flow are hosts.
		string srcname = thisflow["src"].GetString();
		string dstname = thisflow["dst"].GetString();

		bool srcIsHost = false;
		nethost *source_host;
		bool dstIsHost = false;
		nethost *destination_host;

		// if the source is a host, great, that's expected
		if (hosts.find(srcname) != hosts.end()) {
			srcIsHost = true;
			source_host = hosts[srcname];
		}
		// but flows can't start (or end) on anything else...
		else {
			assert(false);
		}

		// if the destination is a host, great, that's expected
		if (hosts.find(dstname) != hosts.end()) {
			dstIsHost = true;
			destination_host = hosts[dstname];
		}
		// but flows can't end (or start) on anything else...
		else {
			assert(false);
		}

		assert(srcIsHost && dstIsHost);

		netflow *curr_flow =
				new netflow (flowname,
						(float) thisflow["start"].GetDouble(),
						(float) thisflow["size"].GetDouble(),
						*source_host, *destination_host, *this);
		flows[flowname] = curr_flow;
	}
}

void simulation::free_network_devices () {

	map<string, nethost *>::iterator hitr;
	map<string, netrouter *>::iterator ritr;
	map<string, netlink *>::iterator litr;
	map<string, netflow *>::iterator fitr;

	for (hitr = hosts.begin(); hitr != hosts.end(); hitr++) {
		delete hitr->second;
	}
	for (ritr = routers.begin(); ritr != routers.end(); ritr++) {
		delete ritr->second;
	}
	for (litr = links.begin(); litr != links.end(); litr++) {
		delete litr->second;
	}
	for (fitr = flows.begin(); fitr != flows.end(); fitr++) {
		delete fitr->second;
	}
}

void simulation::print_network(ostream &os) const {

	map<string, nethost *>::const_iterator hitr;
	map<string, netrouter *>::const_iterator ritr;
	map<string, netlink *>::const_iterator litr;
	map<string, netflow *>::const_iterator fitr;

	// Print all the hosts
	for (hitr = hosts.begin(); hitr != hosts.end(); hitr++) {
		os << *(hitr->second) << endl;
	}

	// Print all the routers
	for (ritr = routers.begin(); ritr != routers.end(); ritr++) {
		os << *(ritr->second) << endl;
	}

	// Print all the links
	for (litr = links.begin(); litr != links.end(); litr++) {
		os << *(litr->second) << endl;
	}

	// Print all the flows
	for (fitr = flows.begin(); fitr != flows.end(); fitr++) {
		os << *(fitr->second) << endl;
	}
}

void simulation::runSimulation() {

	// Loop over the flows, making a start flow event for each and adding
	// it to the events queue
	for (map<string, netflow *>::iterator itr = flows.begin();
			itr != flows.end(); itr++) {
		netflow *flow = itr->second;
		start_flow_event *fevent = new
				start_flow_event(flow->getStartTimeSec(), *this, *flow);
		addEvent(fevent);
	}

	// Loop over the events in the events queue, running the one with the
	// smallest start time.
	while (!events.empty()) {
		multimap<double, event *>::iterator it = events.begin();
		event *curr_event = (*it).second;
		events.erase(it);

		if (debug) {
			debug_os << endl << "In runSimulation loop. Current event ID: " <<
					curr_event->getId() << endl;
		}

		curr_event->runEvent();
	}
}

void simulation::addEvent(event *e) {
	events.insert( pair<double, event *> (e->getTime(), e) );
}

void simulation::removeEvent(event *e) {
		
	// Find the first occurrence of the input time. Since map is sorted
	// by time, all simultaneous events (if any) will be lumped together
	multimap<double, event *>::iterator it = events.find(e->getTime());

	while ((*it).first == e->getTime()) {

		// Check that the event we are removing has the same id as the
		// input.
		if ((*it).second->getId() == e->getId()) {
			events.erase(it);
			delete e;
		}
		it++;
	}
}
