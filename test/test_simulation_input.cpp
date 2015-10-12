/**
 * @file
 * @author Hamik Mukelyan
 *
 * This test case doesn't correspond with a source file or class in the
 * simulation; it just contains code that can be copy-pasted into the
 * simulation driver's JSON file input functions. I wanted to test JSON file
 * input before putting it into the driver, which is why all this is here.
 */

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
#include "netdevice.h"
#include "nethost.h"
#include "netrouter.h"
#include "netlink.h"
#include "netflow.h"

using namespace std;
using namespace rapidjson;

#ifndef TEST_SIMULATION_INPUT_CPP
#define TEST_SIMULATION_INPUT_CPP

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

    map<string, netdevice *> devices;

    // Load the hosts into memory.
    const Value& texthosts = document["hosts"];
	ASSERT_TRUE(texthosts.IsArray());
	for (SizeType i = 0; i < texthosts.Size(); i++) {
		string hostname(texthosts[i].GetString());
		nethost *curr_host = new nethost(hostname);
		devices[hostname] = curr_host;
	}

	// Load the routers into memory
    const Value& textrouters = document["routers"];
	ASSERT_TRUE(textrouters.IsArray());
	for (SizeType i = 0; i < textrouters.Size(); i++) {
		string routername(textrouters[i].GetString());
		netrouter *curr_router = new netrouter(routername);
		devices[routername] = curr_router;
	}

	// Check that the router and host names were read correctly.
	map<string, netdevice *>::iterator itr;
	const char *correct_names[] =
		{ "R1", "R2", "R3", "R4", "S1", "S2", "S3", "T1", "T2", "T3"};
	int i;
	for (itr = devices.begin(), i = 0; itr != devices.end(); itr++, i++) {
		ASSERT_STREQ(correct_names[i], itr->first.c_str());
	}

	// Load the links into memory
    const Value& textlinks = document["links"];
	ASSERT_TRUE(textlinks.IsArray());
	for (SizeType i = 0; i < textlinks.Size(); i++) {
		ASSERT_TRUE(textlinks[i].IsObject());
		const Value& thislink = textlinks[i];
		string linkname = thislink["id"].GetString();
		netdevice *endpoint1 = devices[thislink["endpt_1"].GetString()];
		netdevice *endpoint2 = devices[thislink["endpt_2"].GetString()];
		netlink *curr_link =
				new netlink (linkname,
						(float) thislink["rate"].GetDouble(),
						(float) thislink["delay"].GetDouble(),
						(long) thislink["buf_len"].GetInt64(),
						*endpoint1, *endpoint2);

		// If this link is connected to a host, put reference to it in host.
		if (endpoint1->isHost()) {
			nethost *thishost = dynamic_cast<nethost *>(endpoint1);
			ASSERT_TRUE(thishost->getLink() == NULL);
			thishost->setLink(*curr_link);
		}
		if (endpoint2->isHost()) {
			nethost *thishost = dynamic_cast<nethost *>(endpoint2);
			ASSERT_TRUE(thishost->getLink() == NULL);
			thishost->setLink(*curr_link);
		}
		if (endpoint1->isRouter()) {
			netrouter *thisrouter = dynamic_cast<netrouter *>(endpoint1);
			thisrouter->addLink(*curr_link);
		}
		if (endpoint2->isRouter()) {
			netrouter *thisrouter = dynamic_cast<netrouter *>(endpoint2);
			thisrouter->addLink(*curr_link);
		}
		devices[linkname] = curr_link;
	}

	// Load the flows into memory.
    const Value& textflows = document["flows"];
	ASSERT_TRUE(textflows.IsArray());
	for (SizeType i = 0; i < textflows.Size(); i++) {
		ASSERT_TRUE(textflows[i].IsObject());
		const Value& thisflow = textflows[i];
		string flowname = thisflow["id"].GetString();

		// Make sure that the source and destination of this flow are hosts.
		ASSERT_TRUE(devices[thisflow["src"].GetString()]->isHost());
		ASSERT_TRUE(devices[thisflow["dst"].GetString()]->isHost());

		nethost *host1 = dynamic_cast<nethost *>(
				devices[thisflow["src"].GetString()]);
		nethost *host2 = dynamic_cast<nethost *>(
						devices[thisflow["dst"].GetString()]);

		netflow *curr_flow =
				new netflow (flowname,
						(float) thisflow["start"].GetDouble(),
						(float) thisflow["size"].GetDouble(),
						*host1, *host2);
		devices[flowname] = curr_flow;
	}

	// Print all the routers, links, etc. to stdout.
	for (itr = devices.begin(); itr != devices.end(); itr++) {
		cout << *(itr->second) << endl;
	}


	// Remove all the dynamically allocated objects from memory. TODO
}

#endif // TEST_SIMULATION_INPUT_CPP
