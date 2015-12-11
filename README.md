# Caltech CS/EE 143 Final Technical Report

Simulates a user-specified network of hosts, links, routers, and flows. Outputs graphs so network behavior can be analyzed easily. Written to satisfy the group project requirement for Caltech's introductory networking class, CS/EE 143.

### Authors: Jessica Li, Hamik Mukelyan, Jingwen Wang (alphabetical order)

### Getting started

Pull the repository into a Linux machine then `make`. The simulation probably won't compile on OSX and definitely won't compile in Windows; we have been using Ubuntu VMs. The Makefile generates several binaries: one of them belongs to the unit testing library and can be ignored, the other is a suite of unit tests called `tests`, and the other is the actual simulation and is called `netsim`. The unit tests were used in the earlier stages of development so they are behind relative to the project's architecture and use cases. They were nevertheless important early in development; we abandoned them at some point to meet deadlines and because it was easier to debug `netsim` directly. 

Run `netsim` without args to see a usage message. I suggest `./netsim -d input_files/test_case_0_tahoe`. Kill it if it takes too long to terminate, *MANUALLY REMOVE* `plot/test_case_0_tahoe_log.json`, then run it without the debug flag `-d`. Run `python plotNetSimData.py plot/test_case_0_tahoe_log.json` to see the output graphs. 





### Architecture

Mention docs

### Test cases

graphs, analysis, analytical expectations




