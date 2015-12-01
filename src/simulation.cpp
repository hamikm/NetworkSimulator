/*
 * See header file for function comments.
 */

#include "simulation.h"

bool eventTimeSorter::operator() (const event &e1, const event &e2) {
	return e1.getTime() == e2.getTime() ? e1.getId() < e2.getId() :
			e1.getTime() < e2.getTime();
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
		os << *(hitr->second) << endl << endl;
	}

	// Print all the routers
	for (ritr = routers.begin(); ritr != routers.end(); ritr++) {
		os << *(ritr->second) << endl << endl;
	}

	// Print all the links
	for (litr = links.begin(); litr != links.end(); litr++) {
		os << *(litr->second) << endl << endl;
	}

	// Print all the flows
	for (fitr = flows.begin(); fitr != flows.end(); fitr++) {
		os << *(fitr->second) << endl << endl;
	}
}

map<string, nethost *> simulation::getHosts() const { return hosts; }

map<string, netrouter *> simulation::getRouters() const { return routers; }

void simulation::runSimulation() {

	// Initialize routing tables
	for (map<string, netrouter*>::iterator it_rt = routers.begin();
		 it_rt != routers.end(); it_rt++) {
		it_rt->second->initializeTables(hosts, routers);

		if (debug) {
			it_rt->second->printHelper(cout);
			cout << endl;
		}
	}

	// At regular time intervals, push router discovery events onto events
	// queue. 
	// TODO: determine proper time interval. For now just try once:
	router_discovery_event *r_event = new router_discovery_event(0, *this);
	addEvent(r_event);

	/*
	// Loop over the flows, making a start flow event for each and adding
	// it to the events queue
	for (map<string, netflow *>::iterator itr = flows.begin();
			itr != flows.end(); itr++) {
		netflow *flow = itr->second;
		start_flow_event *fevent = new
				start_flow_event(flow->getStartTimeMs(), *this, *flow);
		addEvent(fevent);
	}
	*/

	// Loop over the events in the events queue, running the one with the
	// smallest start time.
	while (!events.empty()) {
		multimap<double, event *>::iterator it = events.begin();
		event *curr_event = (*it).second;
		events.erase(it);
		curr_event->runEvent();

		if (debug) {
			debug_os << endl;
			if (detail) {
				string dummy;
				cerr << "Waiting... enter to continue." << endl;
				cin.ignore();
			}
		}
	}

	// Check resulting routing tables
	if (debug) {
		for (map<string, netrouter*>::iterator it_rt = routers.begin();
			 it_rt != routers.end(); it_rt++) {

			
			it_rt->second->printHelper(cout);
			cout << endl;
		}
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

/********** SIMULATION LOGGER RELATED FUNCTIONS **********/

int simulation::getEvtCount() const {
	return eventCount;
}

string simulation::getLogName() const {
	return logName;
}

int simulation::initializeLog(string filename) {
	ofstream logger;
	
	// store name of logger file
	logName = filename;
	// opening file with intent of appending to EOF
    logger.open(logName, ios::out |ios::app);
    // write in first line
    string firstLine = "{ \"Simulation Event Metrics\" : [\n";
    logger << firstLine ;
    
    logger.close();

    return 0;
}

int simulation::closeLog() {
	ofstream logger;
	
	// opening file with intent of appending to EOF
    logger.open(logName, ios::out |ios::app);

    // add last line and close json file
    string lastLine = "] }";
    logger << lastLine;

    logger.close();

    return 0;
}

int simulation::logEvent(double currTime) {
	// initialize json holders
    json allLinks;
    json allFlows;
    json currEvent;

    // initalize iterators
    map<string, netlink *>::const_iterator litr;
    map<string, netflow *>::const_iterator fitr;

    // get and format link data
    for (litr = links.begin(); litr != links.end(); litr++) {
        // get and format metrics
        json linkMetric = logLinkMetric(*(litr->second), currTime);

        // append to link metric array
        allLinks.push_back(linkMetric);
    }

    // get and format flow data
    for (fitr = flows.begin(); fitr != flows.end(); fitr++) {
        // get and format metrics
        json flowMetric = logFlowMetric(*(fitr->second), currTime);
        
        // append to flow metric array
        allFlows.push_back(flowMetric);
    }

    // format event metric as json
    json event = 
    {
        {"Time" , currTime},
        {"LinkData" , allLinks},
        {"FlowData" , allFlows}
    };

    // write to file
    ofstream logger;
    logger.open(logName, ios::out |ios::app);

    appendEventMetric(event, logger, eventCount);

    // update eventCount
    eventCount++;

    return 0;
}

void simulation::appendEventMetric(json event, ofstream& logger, int eventNum) {
	if (eventNum != 0) {
        logger << ',' <<'\n';
    }

    // 4 space indentation
    logger << std::setw(4) << event << '\n';
}

json simulation::logLinkMetric(netlink link, double currTime) {

	// retreive link metrics
    string name = link.getName();
    double rate = link.getRateMbps();
    long occ = link.getBufferOccupancy();
    int loss = link.getPktLoss();

    // format into json
    json linkMetric =
    {
        {"LinkID" , name},
        {"LinkRate" , rate},
        {"BuffOcc" , occ},
        {"PktLoss" , loss},
    };

    return linkMetric;
}

json simulation::logFlowMetric(netflow flow, double currTime) {

	// retrieve flow metrics
    string name = flow.getName();
    double rate = flow.getRateMbps();
    int window = flow.getWindowSize();
    double delay = flow.getPktDelay(currTime);

    // format into json
    json flowMetric =
    {
        {"FlowID" , name},
        {"FlowRate" , rate},
        {"WinSize" , window},
        {"PktDelay" , delay},
    };

    return flowMetric;
}
