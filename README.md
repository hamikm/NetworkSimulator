# Caltech CS/EE 143 Final Technical Report

Simulates TCP on a user-specified network of hosts, half-duplex links, routers, and flows. Outputs graphs so network behavior can be analyzed easily. Written to satisfy the group project requirement for Caltech's introductory networking class, CS/EE 143.

### Authors (alphabetically): Jessica Li, Hamik Mukelyan, Jingwen Wang

### Quick Start

Pull the repository--it should pull `smart_gbn`, which is the default branch--into a Linux machine then `make`. The simulation probably won't compile on OSX and definitely won't compile in Windows; we have been using Ubuntu VMs. The Makefile generates several binaries: one of them belongs to the unit testing library and can be ignored, the other is a suite of unit tests called `tests`, and the other is the actual simulation and is called `netsim`. The unit tests were used early in development so they are behind relative to the project's architecture and use cases. They were nevertheless important early on and we encourage you to run them as `./tests`.

To run the simulation itself type `./netsim` without args to see a usage message then try `./netsim -d input_files/test_case_0_tahoe plot/nameofLogger.json`. Kill it if it takes too long to terminate then run it without the debug flag `-d`.

To generate plots of simulation metrics, run `python plot/plotNetSimData.py plot/test_case_0_tahoe_log.json` to see the output graphs. To plot the graphs in the background, add an ` & ` to the end of that command. Two windows should appear, one for flow related metrics and one for link related metrics. Graphs can be scaled by resizing the windows or using the magnify tool in the bottom left. To faciliate comparisons, data sets plotted in each subplot can be made visible or removed from view by clicking the respective icon in the subplot's legend.

We encourage you to use our Doxygen-generated HTML documentation as you acquaint yourself with the codebase. Here's the [class documentation](http://users.cms.caltech.edu/~hamik/docs/html/annotated.html) and here's the [file documentation](http://users.cms.caltech.edu/~hamik/docs/html/files.html). Type `make clean` to clean up, `make docs` to generate potentially newer documentation locally--open `docs/html/index.html` in a browser to see them--and although it shouldn't be necessary you can run `make depend` to see the Makefile automatically generate its own dependencies.

### Purpose and Scope

A full description of the project can be found at `NetworkSimGuidelines-2015.pdf` in the root directory of this project. The intent of the simulation was to help the authors better learn TCP through coding a network simulator and analyzing the results for the given test cases.

This networks in this simulation consist of:
* *routers*, which dynamically update routing tables
* *hosts*, which send data flows to other hosts through routers
* half-duplex *links* connecting hosts and routers
* *flows*, which represents data transfers
* *packets*, which do not have a payload size, but not an actual payload

Hosts partition data flows into packets, which are enqueued onto links, which pass them to routers, which forward them to the flow's destination. As such error correction techniques, such as parity bits and checksums, are not simulated. Destinations receive packets and acknowledge them. Users choose which protocol to transfer flows with. This simulation supports TCP Tahoe and TCP-FAST. Routers periodically update their routing table by running the Bellman-Ford algorithmon the network. Routing table update occur within the simulation. In other works, routing packets use to find routers must wait to be transfered as would flow and ack packets.

### Architecture

Our simulation is event-driven, which means that it initially seeds an event queue with starter events--`start_flow_event` instances in our case--then enters a loop in which it dequeues an event, calls its `runEvent` function, then logs some data before iterating. Each event occurs at a particular time, which was determined at the time the event was created and which is used to order the events in the events queue, and each event can in turn queue more events. For example, a `send_packet_event` might enqueue a `receive_packet_event` after determining the time at which its packet will be received at the other end of the link. That process is explained in detail in the documentation in the corresponding class. 

As we explain the architecture in more detail we will follow the flow of the `netsim` program from start to finish. The unit test architecture is not discussed.

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

We have written up the three provided test cases in this format, but the simulation will in principle handle others.

#### Driver File and Simulation Class

Our `main` function lives in `driver.cpp`. This file handles console arguments and initializes a `simulation` object, whose job is to parse input files, populate in-memory flow, router, host, and link collections, and queue some initial events. The driver then instructs the simulation object to enter its main loop, where events are dequeued and run. 

#### Logging

JSON for Modern C++ is the C++ JSON parsing module used to write the log file in JSON format. A logger file is created every time a simluation is run, thus all logger related functions are stored under a simulation object. Data is logged every time an event is run. In order to speed up graphing and reduce the size of the log file, 1 in every 10 events is actually logged. See `sampleDataFile.json` for example of log file format.

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
    - calculated per packet as time elapsed since packet was sent and respective acknowledgement was received


### Analysis of Simulation of TCP on Given Test Cases

See `NetworkSimTestCases-2015.pdf` in the root directory for an explanation 

#### TCP Tahoe

##### Test Case 0
##### Test Case 1
##### Test Case 2

#### TCP-FAST

Although the window sizes do converge to a steady-state, the run-time (in simulation seconds) is longer than expected and the steady-state window size is lower than expected because our acknowledgement packets and routing packets incur link delays. These delays can be exacerbated by the fact that we have implemented half-duplex links.

##### Test Case 0
##### Test Case 1
##### Test Case 2

### Division of Labor
*A lot of the commits that appear to have originated from Hamik's account are actually mostly Jingwen's or Jessica's, since Hamik just performed the merges.*

#### Jingwen

* Helped design event handling
* Designed and implemented dynamic routing and associated events
* Implemented FAST TCP and associated events
* Found fixes for bugs in sending/receiving packets, duplicate and selective ack handling, window resizing, and incurring half-duplex link costs
*Note: A few early commits were made when my git.config email was set incorrectly, so the contributions are not registered to my account. The commits are still visible under my name if you scroll back through the commit feed.*

#### Jessica 
* implemented logging from simulation into JSON formatted log file using JSON for Modern C++ instead of RapidJSON
* implemented plotting of simulation metrics by parsing JSON logger
    * implemented interactive plot 
* created presentations
* implemented selective acknowledgement scheme

#### Hamik

* Decided the JSON input file format, wrote the test cases, and chose the RapidJSON library
* Chose the Doxygen documentation generator and uploaded the final docs to his website
* Wrote the long Makefile
* Decided an inheritance hierarchy and wrote skeleton flow, host, router, link, packet, simulation, and event classes. He also wrote the `driver.cpp` file as the entry point for the simulation and tested the skeleton simulation against some unit tests written under the Google C++ unit testing framework.
* Played a support role for Jessica and Jingwen after the project was bootstrapped, which included helping with debugging, occasionally writing new code or bug fixes, and clarifying C++ concepts since they were less familiar with C++ than he was.
* Merged a lot of the code from other branches into the default branch, since his editor (Eclipse) was best at it. 
