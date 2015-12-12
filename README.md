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

Hosts partition data flows into packets, which are enqueued onto links, which pass them to routers, which forward them to the flow's destination. As such error correction techniques, such as parity bits and checksums, are not simulated. Destinations receive packets and acknowledge them. Users specify with which protocol to transfer flows in the input file. This simulation supports TCP Tahoe and TCP-FAST. Routers periodically update their routing tables by running the Bellman-Ford algorithm on the network. Routing packets must wait to be transferred through the links along with flow and acknowledgement traffic. For distributed Bellman Ford, the routing messages are sent until the graph becomes stable. The update process is terminated (i.e., no more routing packets are sent from a particular router) when the router does not need to update any more distances in its routing table.


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
- *buffer occupancy*
    - stored as state variable
    - calculated as terms of KB rather than packets because all packets get queued, but do not all have the same size
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
Since Test Case 0 has only 1 flow transferring over 1 link, a 20 MB flow transferring at full link capacity should take about 16 seconds. However, given congestion control is implemented and that TCP Tahoe is the slowest of the congestion controls, we expect the transmission time to take a little longer. The actual simulation takes ~28 seconds to complete, which is reasonable.

The plot of window sizes also appears to be correct. Slow start is entered in the beginning. Afterwards, every time duplicate acknowledgements are registered (corresponding to spikes in packet delay), the window size is reset to 1. Slow start is entered again before entering linear growth. This can be seen in the plot of window sizes in the slight gap between each fin. 

![test-case-0-tahoe-flow](https://github.com/hamikm/cit_cs143_network_sim/blob/smart_gbn/report_graphs/tc0_tahoe_flow_metrics_graph.png)
![test-case-0-tahoe-link](https://github.com/hamikm/cit_cs143_network_sim/blob/smart_gbn/report_graphs/tc0_tahoe_link_metrics_graph.png)

##### Test Case 1
This test case is intended to test the dynamic routing. Link cost is determined by the amount of time it takes for packet to traverse the link. This metric includes both static cost (length) and dynamic cost (due to congestion) since traversal time depends on both. Distance to self and adjacent hosts are treated as 0, since each host has only one outgoing link and the next hop will never change. The first router discovery event occurs at time 0, and terminates before any flows start. In subsequent router discovery events, each router’s distance table is reset (although next-hop links are not) and routing packets are sent from each router to its neighboring routers.

We expect flow packets to alternate between L1 and L2, and L3 and L4. It is more difficult to see in the throughput graphs, but easy to see by examing the link buffers. One can clearly see the switching between links, indicating the dynamic routing is working.

Different from Test Case 0, Test Case 1 has routers. We expect the total time to be far longer because the flow running with TCP Tahoe is traveling over many links, and routing table update events are happening while the simulation timer is running.

![test-case-1-tahoe-flow](https://github.com/hamikm/cit_cs143_network_sim/blob/smart_gbn/report_graphs/tc1_tahoe_flow_metrics_graph.png)
![test-case-1-tahoe-link](https://github.com/hamikm/cit_cs143_network_sim/blob/smart_gbn/report_graphs/tc1_tahoe_link_metrics_graph.png)

##### Test Case 2

Since Test Case 2 contains multiple routers and multiple flows, we expect it to take the longest and exhibit the most complex behavior behavior of the three cases. Looking at plot of flow throughput, we may get a sense for how long each flow took. Flow 1 took the longest to transmit, which is reasonable since it had the longest distance to travel. 

The window size plot is as roughly as expected. There is a spike in window size whenever each plow begins to send due to slow start. Afterwards, each flow's window size is adjusted according to TCP Tahoe. Flow 1's window size continues to readjust throughout the entire simulation. However, Flow 2's and Flow 3's window sizes flatline at approximately 150 seconds and 310 seconds, respectively. These times are also when Flow 2 and Flow 3 finish transmitting. The flatline in the plot is due to the fact that the the simulation continues to log metrics from all flows, until the simulation terminates.

![test-case-2-tahoe-flow](https://github.com/hamikm/cit_cs143_network_sim/blob/smart_gbn/report_graphs/tc2_tahoe_flow_metrics_graph.png)
![test-case-2-tahoe-link](https://github.com/hamikm/cit_cs143_network_sim/blob/smart_gbn/report_graphs/tc2_tahoe_link_metrics_graph.png)

#### TCP-FAST

We used gamma = 1 and alpha = 20. Window size is updated every 20 milliseconds. Although the window sizes do quickly converge to a steady-state, steady-state window size is lower than expected, causing the run-time (in simulation seconds) to longer than expected. We suspect that this is true because our acknowledgement packets and routing packets both incur link delays, which can be exacerbated by the fact that we have implemented half-duplex links.

##### Analytical Results

As we pointed out above our FAST results are impossible to compare with our analytical results because we're using half-duplex links and because window resizing under FAST doesn't quite work, but here's our analysis anyway for test case 2. Note that our buffer occupancy metric is recorded in bytes rather than in packets.

(Zero to ten seconds) In this time interval	the first flow goes from S1 to T1 using each of the links L1, L2, and L3. Since there is just one flow and since all link capacities and propagation delays are the same the throughput of the first flow is just 2500 packets/s. The queuing delay is then q<sub>1</sub><sup>\*</sup> = &#945; / x<sub>1</sub><sup>\*</sup> = 50 / 2500 = 0.02 s/packet. Since 2500 packets are sent per second we get a queue length of (link capacity)\*(queuing delay) = 2500 \* 0.02 = 50 packets, though packets will only need to queue up at link 1 since the steady-state send-rate keeps packets from needing to queue at links 2 or 3.

(Ten to twenty seconds) x<sub>1</sub><sup>\*</sup> = &#945; / q<sub>1</sub><sup>\*</sup>, where q<sub>1</sub><sup>\*</sup> is the new queuing delay for link 1. However the second flow must account for the original 0-10s delay q<sub>1'</sub><sup>\*</sup>, so x<sub>2</sub><sup>\*</sup> = &#945; / (q<sub>1</sub><sup>\*</sup> + q<sub>1'</sub><sup>\*</sup>)  = &#945; / (q<sub>1</sub><sup>\*</sup> - 0.02). These throughputs must sum to 2500 packets/s, so we have &#945; / (q<sub>1</sub><sup>\*</sup> - 0.02) + &#945; / q<sub>1</sub><sup>\*</sup> = 50 / (q<sub>1</sub><sup>\*</sup> - 0.02) + 50 / q<sub>1</sub><sup>\*</sup> = 2500 if and only if q<sub>1</sub><sup>\*</sup> = 1/100 \* (3 &#177; &#8730;5) (from Wolfram Alpha). Discarding the smaller solution as "too small" we get the new queueing delay is q<sub>1</sub><sup>\*</sup> = .05236s, which generates a queue length of 2500 \* .05236 = 131 packets. We also have x<sub>1</sub><sup>\*</sup> = &#945; / q<sub>1</sub><sup>\*</sup> = 50 / .05236 = 955 packets/s as the throughput of flow 1 and x<sub>2</sub><sup>\*</sup> = &#945; / (q<sub>1</sub><sup>\*</sup> + q<sub>1'</sub><sup>\*</sup>) = 50 / (.05236 - .02) = 1545 packets/s as the throughput of flow 2. Note that again only flow 1 travels through links 2 and 3, and since its throughput is less than the capacities of those links there are no queueing delays or queues at those links.

(Twenty seconds onward) Instead of explaining again I'm just going to write down the equations, since the reasoning is analogous to what I've already written. We have x<sub>1</sub><sup>\*</sup> = &#945; / (q<sub>1</sub><sup>\*</sup> + q<sub>3</sub><sup>\*</sup>), x<sub>2</sub><sup>\*</sup> = &#945; / (q<sub>1</sub><sup>\*</sup> - q<sub>1'</sub><sup>\*</sup>), and x<sub>3</sub><sup>\*</sup> = &#945; / q<sub>3</sub><sup>\*</sup>. The sum of the throughputs for flows 1 and 2 equals 2500 packets/s, as does the sum of the throughputs for flows 1 and 3. The equations are 2500 = 50 / (q<sub>1</sub><sup>\*</sup> + q<sub>3</sub><sup>\*</sup>) + 50 / (q<sub>1</sub><sup>\*</sup> - .02) and  2500 = 50 / (q<sub>1</sub><sup>\*</sup> + q<sub>3</sub><sup>\*</sup>) + 50 / (q<sub>3</sub><sup>\*</sup>). Note that since only flow 2 goes through link 2 and its still rate-limited by link 1 there's no queueing delay q<sub>2</sub><sup>\*</sup> and no corresponding queue on link 2. Solving these in Wolfram Alpha and taking the physical solutions we get q<sub>1</sub><sup>\*</sup> = 1/100 \* (3 + &#8730;3) = .047s and q<sub>3</sub><sup>\*</sup> = 1/100 \* (1 + &#8730;3) = .027s, which generate queue lengths of 118 and 68 packets on links 1 and 3, respectively. Then x<sub>1</sub><sup>\*</sup> = 50 / (q<sub>1</sub><sup>\*</sup> + q<sub>3</sub><sup>\*</sup>) = 50 / (.027 + .047) = 670 packets/s, x<sub>2</sub><sup>\*</sup> = &#945; / (q<sub>1</sub><sup>\*</sup> - q<sub>1'</sub><sup>\*</sup>) = 50 / (.047 - .02) = 1831 packets/s, and x<sub>3</sub><sup>\*</sup> =  &#945; / q<sub>3</sub><sup>\*</sup> = 50 / .027 = 1831 packets/s.

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
