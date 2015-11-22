/**
 * @file
 * @author Hamik Mukelyan
 *
 * This test case doesn't correspond with a source file or class in the
 * simulation; it just contains code that can be copy-pasted into the
 * simulation driver's JSON file input functions. I wanted to test JSON file
 * input before putting it into the driver, which is why all this is here.
 */

#ifndef TEST_SIMULATION_INPUT_CPP
#define TEST_SIMULATION_INPUT_CPP

// Standard includes.
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include <string.h>
#include <sstream>
#include <fstream>

using namespace std;
using namespace rapidjson;

/*
 * This is a "test fixture" that sets up things we need in the actual unit
 * tests below. Note that an object of this class is created before
 * each test case begins and is torn down when each test case ends.
 */
class simulationInputTest : public ::testing::Test {
protected:

	/* This string will contain json data read in from the file below. */
	string jsonstr;

	virtual void SetUp() {

		// Read the data into an intermediate string, tack that onto a
		// string buffer, then after file ends copy whole json string.
		stringstream sstr;
		ifstream inputfile ("input_files/test_case_2");
		string line;
		if (inputfile.is_open()) {
			while (getline(inputfile, line)) {
				sstr << line << endl;
			}
			inputfile.close();
		}
		jsonstr = sstr.str();
	}

	virtual void TearDown() { }
};

/*
 * Reads JSON from a file and populates hosts, links, routers, flows.
 */
TEST_F(simulationInputTest, JsonFileInput) {

	// Parse JSON text into a document.
    Document document;
    char buffer[jsonstr.length() + 1];
    memcpy(buffer, jsonstr.c_str(), jsonstr.length());
    buffer[jsonstr.length()] = 0; // string has to be null-terminated
    ASSERT_FALSE(document.ParseInsitu(buffer).HasParseError());

    map<string, nethost *> hosts;
    map<string, netrouter *> routers;
    map<string, netlink *> links;
    map<string, netflow *> flows;

    // Load the hosts into memory.
    const Value& texthosts = document["hosts"];
	ASSERT_TRUE(texthosts.IsArray());
	for (SizeType i = 0; i < texthosts.Size(); i++) {
		string hostname(texthosts[i].GetString());
		nethost *curr_host = new nethost(hostname);
		hosts[hostname] = curr_host;
	}

	// Load the routers into memory
    const Value& textrouters = document["routers"];
	ASSERT_TRUE(textrouters.IsArray());
	for (SizeType i = 0; i < textrouters.Size(); i++) {
		string routername(textrouters[i].GetString());
		netrouter *curr_router = new netrouter(routername);
		routers[routername] = curr_router;
	}

	// Check that the router and host names were read correctly.
	map<string, nethost *>::iterator hitr;
	const char *correct_hnames[] =
		{ "S1", "S2", "S3", "T1", "T2", "T3"};
	map<string, netrouter *>::iterator ritr;
	const char *correct_rnames[] = { "R1", "R2", "R3", "R4" };
	int i;
	for (hitr = hosts.begin(), i = 0; hitr != hosts.end(); hitr++, i++) {
		ASSERT_STREQ(correct_hnames[i], hitr->first.c_str());
	}
	for (ritr = routers.begin(), i = 0; ritr != routers.end(); ritr++, i++) {
		ASSERT_STREQ(correct_rnames[i], ritr->first.c_str());
	}

	// Load the links into memory
    const Value& textlinks = document["links"];
	ASSERT_TRUE(textlinks.IsArray());
	for (SizeType i = 0; i < textlinks.Size(); i++) {
		ASSERT_TRUE(textlinks[i].IsObject());
		const Value& thislink = textlinks[i];
		string linkname = thislink["id"].GetString();

		string endpt1name = thislink["endpt_1"].GetString();
		string endpt2name = thislink["endpt_2"].GetString();

		netelement *endpoint1;
		bool endpt1IsHost = false;
		netelement *endpoint2;
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
		// if this endpoing is neither a router nor a host...
		else {
			// ...that should never happen!
			ASSERT_TRUE(false);
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
		// if this endpoing is neither a router nor a host...
		else {
			// ...that should never happen!
			ASSERT_TRUE(false);
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
			ASSERT_TRUE(thishost->getLink() == NULL);
			thishost->setLink(*curr_link);
		}
		if (endpt2IsHost) {
			nethost *thishost = dynamic_cast<nethost *>(endpoint2);
			// each host must have exactly 1 link
			ASSERT_TRUE(thishost->getLink() == NULL);
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
	ASSERT_TRUE(textflows.IsArray());
	for (SizeType i = 0; i < textflows.Size(); i++) {
		ASSERT_TRUE(textflows[i].IsObject());
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
			ASSERT_TRUE(false);
		}

		// if the destination is a host, great, that's expected
		if (hosts.find(dstname) != hosts.end()) {
			dstIsHost = true;
			destination_host = hosts[dstname];
		}
		// but flows can't end (or start) on anything else...
		else {
			ASSERT_TRUE(false);
		}

		ASSERT_TRUE(srcIsHost && dstIsHost);

		netflow *curr_flow =
				new netflow (flowname,
						(float) thisflow["start"].GetDouble(),
						(float) thisflow["size"].GetDouble(),
						*source_host, *destination_host);
		flows[flowname] = curr_flow;
	}

	// Remove all the dynamically allocated objects from memory.
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

#endif // TEST_SIMULATION_INPUT_CPP
