#!/usr/bin/python

'''
In Network Simulation project for Caltech CS 143, the following metrics are
measured and then graphed as a time trace and overall average:
    For each link:
        -- link rate (Mbps)
        -- buffer occupancy (is this one per link?)
        -- packet loss
    For each flow:
        -- flow rate
        -- window size
        -- packet delay
The simulator records data into a .json file, from which data is extracted
and graphed.
'''

import json
import sys
import matplotlib.pyplot as plt 

def loadAllData(filename):
	'''
	Given the .json data file, sorts all data into their appropriate arrays
	for later graphing. If a data point is 'null' it is skipped.
	'''
	with open(filename) as dataFile:
		# create array of events
		events = json.load(dataFile)["Simulation Event Metrics"]
	
	# initialize arrays and dicts that hold data	
	times = []
	linksData = {}
	flowsData = {}

	if events[0]["Time"] == 0:
		# initialize data storage arrays
		times.append(0)
		# initialize data storage arrays for links
		for link in events[0]["LinkData"]:
			linksData[link["LinkID"]] = {"Link Rate (Mbps)" : [link["LinkRate"]],
			                              "Buffer Occupancy (pkts)" : [link["BuffOcc"]],
			                              "Packet Loss (pkts)" : [link["PktLoss"]]}
		for flow in events[0]["FlowData"]:
			flowsData[flow["FlowID"]] = {"Flow Rate (Mbps)" : [flow["FlowRate"]],
				                       "Window Size (pkts)" : [flow["WinSize"]],
				                       "Packet Delay (ms)" : [flow["PktDelay"]]}

	# continue sorting data from every event
	for event in events[1:]:
		# get time
		times.append(event["Time"])
		# get and store link data
		for link in event["LinkData"]:
			linksData[link["LinkID"]]["Link Rate (Mbps)"].append(link["LinkRate"])
			linksData[link["LinkID"]]["Buffer Occupancy (pkts)"].append(link["BuffOcc"])
			linksData[link["LinkID"]]["Packet Loss (pkts)"].append(link["PktLoss"])
			
		# get and store flow data
		for flow in event["FlowData"]:
			flowsData[flow["FlowID"]]["Flow Rate (Mbps)"].append(flow["FlowRate"])
			flowsData[flow["FlowID"]]["Window Size (pkts)"].append(flow["WinSize"])
			flowsData[flow["FlowID"]]["Packet Delay (ms)"].append(flow["PktDelay"])

	return (times, linksData, flowsData)


def plotLinkData(time, linksData):
	'''
	Given array of time data and dictionary containg metrics for all links,
	generate three plots: 1 showing each metric for all links.
	'''

	f, (lrates, buffocc, pktloss) = plt.subplots(3, 1, sharex=True)

	# set window title
	f.canvas.set_window_title("Flow Metrics Graph")
	
	# plot link rates, buff occup, packet loss for all links
	# note: 'o' is to ensure scatterplot
	for link in linksData.keys():
		lrates.plot(time, linksData[link]["Link Rate (Mbps)"], 'o', label=str(link))
		buffocc.plot(time, linksData[link]["Buffer Occupancy (pkts)"], 'o', label=str(link))
		pktloss.plot(time, linksData[link]["Packet Loss (pkts)"], 'o', label=str(link))
	
	# add titles and labels and legend to plots
	lrates.set_xlabel('Time (ms)')
	lrates.set_ylabel('Link Rate (Mbps)')
	lrates.legend()
	
	buffocc.set_xlabel('Time (ms)')
	buffocc.set_ylabel('Buffer Occupancy (pkts)')
	buffocc.legend()
	
	pktloss.set_xlabel('Time (ms)')
	pktloss.set_ylabel('Packet Loss (pkts)')
	pktloss.legend()

	# set window title
	f.canvas.set_window_title("Link Metrics Graph")


def plotFlowData(time, flowsData):
	'''
	Given array of time data and dictionary containg metrics for all flows,
	generate three plots: 1 showing each metric for all flows.
	'''
	f, (frates, winsize, pktdelay) = plt.subplots(3, 1, sharex=True)
	
	# plot flow rates, buff occup, packet loss for all links
	# note: 'o' is to ensure scatterplot
	for flow in flowsData.keys():
		frates.plot(time, flowsData[flow]["Flow Rate (Mbps)"], 'o',label=str(flow))
		winsize.plot(time, flowsData[flow]["Window Size (pkts)"], 'o', label=str(flow))
		pktdelay.plot(time, flowsData[flow]["Packet Delay (ms)"], 'o', label=str(flow))
	
	# add titles and labels to plots
	frates.set_xlabel('Time (ms)')
	frates.set_ylabel('Flow Rate (Mbps)')
	frates.legend()
	
	winsize.set_xlabel('Time (ms)')
	winsize.set_ylabel('Window Size (pkts)')
	winsize.legend()
	
	pktdelay.set_xlabel('Time (ms)')
	pktdelay.set_ylabel('Packet Delay (ms)')
	pktdelay.legend()

	# set window title
	# don't really understand why this isn't working
	f.canvas.set_window_title("Flow Metrics Graph")

def plotAll(time, linksData, flowsData):
	''' Generates plots for flow and link data'''
	# plot flows
	plotLinkData(time, linksData)
	# plot links
	plotFlowData(time, flowsData)

	plt.show()

if __name__ =='__main__':
	try:
		filename = sys.argv[1]
		
		# make sure it's a json file
		if filename.split('.')[1] != "json":
			raise IOError()
		
		time, links, flows = loadAllData(filename)
		plotAll(time, links, flows)
	
	except IndexError:
		print >> sys.stderr, "usage: python plotNetSimGraphs *.json"
		sys.exit(1)

	except IOError:
		print >> sys.stderr, "usage: python plotNetSimGraphs *.json"
		print >> sys.stderr, "    input must be valid *.json file format"
		sys.exit(1)