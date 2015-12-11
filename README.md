# Caltech CS/EE 143 Final Technical Report

Simulates TCP on a user-specified network of hosts, half-duplex links, routers, and flows. Outputs graphs so network behavior can be analyzed easily. Written to satisfy the group project requirement for Caltech's introductory networking class, CS/EE 143.

### Authors (alphabetically): Jessica Li, Hamik Mukelyan, Jingwen Wang

### Quick Start

Pull the repository--it should pull `smart_gbn`, which is the default branch---into a Linux machine then `make`. The simulation probably won't compile on OSX and definitely won't compile in Windows; we have been using Ubuntu VMs. The Makefile generates several binaries: one of them belongs to the unit testing library and can be ignored, the other is a suite of unit tests called `tests`, and the other is the actual simulation and is called `netsim`. The unit tests were used early in development so they are behind relative to the project's architecture and use cases. They were nevertheless important early on and we encourage you to run them as `./tests`.

To run the simulation itself type `./netsim` without args to see a usage message then try `./netsim -d input_files/test_case_0_tahoe plot/nameofLogger.json`. Kill it if it takes too long to terminate then run it without the debug flag `-d`.

To generate plots of simulation metrics, run `python plot/plotNetSimData.py plot/test_case_0_tahoe_log.json` to see the output graphs. To plot the graphs in the background, add an ` & ` to the end of that command. Two windows should appear, one for flow related metrics and one for link related metrics. Graphs can be scaled by resizing the windows or using the magnify tool in the bottom left. To faciliate comparisons, data sets plotted in each subplot can be made visible or removed from view by clicking the respective icon in the subplot's legend.

We encourage you to use our Doxygen-generated HTML documentation as you acquaint yourself with the codebase. Here's the [class documentation](http://users.cms.caltech.edu/~hamik/docs/html/annotated.html) and here's the [file documentation](http://users.cms.caltech.edu/~hamik/docs/html/files.html). Type `make clean` to clean up, `make docs` to generate potentially newer documentation locally--open `docs/html/index.html` in a browser to see them--and although it shouldn't be necessary you can run `make depend` to see the Makefile automatically generate its own dependencies.

### Purpose and Scope

A full description of the project can be found at `NetworkSimGuidelines-2015.pdf` in the base directory of this project. The intent of the simulation was to help the authors better learn TCP through coding a network simulator and analyzing the results for the given test cases.

This networks in this simulation consist of:
* *routers*, which dynamically update routing tables
* *hosts*, which send data flows to other hosts through routers
* half-duplex *links* connecting hosts and routers
* *flows*, which represents data transfers
* *packets*, which do not have a payload size, but not an actual payload

Hosts partition data flows into packets, which are enqueued onto links, which pass them to routers, which forward them to the flow's destination. As such error correction techniques, such as parity bits and checksums, are not simulated. Destinations receive packets and acknowledge them. Users choose which protocol to transfer flows with. This simulation supports TCP Tahoe and TCP-FAST. Routers periodically update their routing table by running the Bellman-Ford algorithmon the network. Routing table update occur within the simulation. In other works, routing packets use to find routers must wait to be transfered as would flow and ack packets.

### Architecture

Event driven simulation
We will follow the flow of the `netsim` program from start to finish and will omit a discussion of the unit tests.

#### Program Input

The network topology and other network parameters like the sizes, start times, and TCP protocols of flows are all specified by the user in JSON files that live in the `input_files` directory. Each `.json` file corresponds to one test case. The format is as follows:

```json
{
    "hosts": [ "H1", "H2", "more hosts here" , "H<n>" ],
    "routers": [ "R1", "R2", "more routers here" , "R<m>" ],
    "links": [ 
        { "id": "L1", 
          "rate": rate_in_mbps, 
          "delay": signal_propagation_delay_in_ms,
          "buf_len": buffer_size_in_kb,
          "endpt_1": "host or router name",
          "endpt_2": "host or router name" },
        { "more links here" } ],
    "flows": [
        { "id": "F1",
          "src": "host string",
          "dst": "host string",
          "size": data_transmission_size_in_mb,
          "start": flow_start_time_in_sec },
        { "more flows here" } ]
}
```

#### Driver
Main file that parses console arguments to generate a network from the JSON input file, create a simulation object and log file, and begin simulation. 

#### Simulation

#### Logging
JSON for Modern C++ is the C++ JSON parsing module used to write the logger in JSON format. A logger file is created every time a simluation is run, thus all logger related functions are stored under a simulation object. Data is logged every time an event is run. In order to speed up graphing and reduce the size of the log file, 1 in every 10 events is actually logged.

This simulation logs the following:

Link Metrics
- *throughput*
    - calculated by binning the number of packets received by the network node at either end of the link (since every link is half duplex) every RATE_INTERVAL, currently set to 1 second.
- *buffer capacity*
    - stored as state variable
- *packet loss*
    - computed as number of packets continously dropped from a full buffer, reset every time buffer is not full

Flow Metrics
- *flow throughput*
    - calculated by binning number of flow packets received by the flow's destination host every RATE_INTERVAL, currently set to 1 second
- *window size*
    - stored as state variable
    - updated according to flow's TCP
- *packet delay*
    - calculated per pakcet as time elapsed since packet was sent and respective acknowledgement was received


### Analysis of Simulation of TCP on Given Test Cases

Three test cases were provided:
* **Test Case 0** contains two hosts, a single flow, and no routers, and was useful for verifying our simluation architecture.
* **Test Case 1** contains two hosts, a single flow, and four routers, and was useful for testing dynamic routing.
* **Test Case 2** contains six hosts, three flows, and four routers, and was useful for testing our simulation on multiple flows.

#### TCP Tahoe

##### Test Case 0
##### Test Case 1
##### Test Case 2

#### TCP-FAST

##### Test Case 0
##### Test Case 1
##### Test Case 2

### Division of Labor

#### Jingwen

TODO

#### Jessica 

#### Hamik

* Decided the JSON input file format, wrote the test cases, and chose the RapidJSON library
* Chose the Doxygen documentation generator and uploaded the final docs to his website
* Wrote the rather long Makefile
* Decided an inheritance hierarchy and wrote skeleton flow, host, router, link, packet, simulation, and event classes. He also wrote the `driver.cpp` file as the entry point for the simulation and tested the skeleton simulation against some unit tests written under the Google C++ unit testing framework.
* Played a support role for Jessica and Jingwen after the project was bootstrapped, which included helping with debugging, occasionally writing new code or bug fixes, and clarifying C++ concepts since they were less familiar with C++ than he was.
* Merged a lot of the code from other branches into the default branch, since his editor (Eclipse) was best at it.
