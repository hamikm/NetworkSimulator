# Caltech CS/EE 143 Final Technical Report

Simulates a user-specified network of hosts, half-duplex links, routers, and flows. Outputs graphs so network behavior can be analyzed easily. Written to satisfy the group project requirement for Caltech's introductory networking class, CS/EE 143.

### Authors: Jessica Li, Hamik Mukelyan, Jingwen Wang (alphabetical order)

### Quick start

Pull the repository into a Linux machine then `make`. The simulation probably won't compile on OSX and definitely won't compile in Windows; we have been using Ubuntu VMs. The Makefile generates several binaries: one of them belongs to the unit testing library and can be ignored, the other is a suite of unit tests called `tests`, and the other is the actual simulation and is called `netsim`. The unit tests were used early in development so they are behind relative to the project's architecture and use cases. They were nevertheless important early on and I encourage you to run them as `./tests`.

The run the simulation itself type `./netsim` without args to see a usage message then try `./netsim -d input_files/test_case_0_tahoe`. Kill it if it takes too long to terminate then run it without the debug flag `-d`. Run `python plot/plotNetSimData.py plot/test_case_0_tahoe_log.json` to see the output graphs. Note that you can scale the graphs and turn specific lines on and off by clicking on their respective colored lines in the keys. 

We encourage you to use our Doxygen-generated HTML documentation as you acquaint yourself with the codebase. Here's the [class documentation](http://users.cms.caltech.edu/~hamik/docs/html/annotated.html) and here's the [file documentation](http://users.cms.caltech.edu/~hamik/docs/html/files.html). 

### Purpose and scope

A full description of the project can be found at `NetworkSimGuidelines-2015.pdf` in the base directory of this project. The intent of the simulation was to help the authors better learn TCP through coding and through analysis of simulation-generated graphs of flow and link metrics. An ancillary goal for the authors was to not fail their class.

This is a simulation of a network consisting of a few simple components: hosts which send data flows to other hosts, optional routers, and half-duplex links which connect hosts and routers. We tried to mirror reality closely; hosts partition data flows into packets which are used to send data down links to routers which forward them through the best known link towards the destination, but packets contain no payloads. As such error correction techniques like parity bits and checksums are not simulated. Destinations receive packets and acknowledge them with the TCP ACK scheme. Packets are sent and received under a user-chosen TCP protocol--either TCP Tahoe or TCP FAST. Routers periodically run the Bellman-Ford algorithms on the network--i.e, they send and receive routing packets to judge network congestion and distances to other nodes--to populate their routing tables. 

### Architecture

We use a discrete event-driven simulation. What this means is that 

The network topology and other network parameters like the sizes, start times, and TCP protocols of flows are all specified by the user 

#### Logging


### Test cases and analysis




