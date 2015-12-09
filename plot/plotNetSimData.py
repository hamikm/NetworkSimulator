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

For each subplot in each window (Link Metric, Flow Metric), the viewer can
choose which data set to view by clicking on the associated line 'symbol'
in the subplot's legend. This feature is most helpful when comparing link
flow rates, since the data sets all overlap and become hard to make sense of.
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

	# use events[0] to declare and initialize arrays
	# initialize data storage arrays
	times.append(events[0]["Time"])
	# initialize data storage arrays for links
	for link in events[0]["LinkData"]:
		linksData[link["LinkID"]] = {"Link Rate (Mbps)" : [link["LinkRate"]],
		                              "Buffer Occupancy (KB)" : [link["BuffOcc"]],
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
			linksData[link["LinkID"]]["Buffer Occupancy (KB)"].append(link["BuffOcc"])
			linksData[link["LinkID"]]["Packet Loss (pkts)"].append(link["PktLoss"])
			
		# get and store flow data
		for flow in event["FlowData"]:
			flowsData[flow["FlowID"]]["Flow Rate (Mbps)"].append(flow["FlowRate"])
			flowsData[flow["FlowID"]]["Window Size (pkts)"].append(flow["WinSize"])
			flowsData[flow["FlowID"]]["Packet Delay (ms)"].append(flow["PktDelay"])

	return (times, linksData, flowsData)

def makeLegDict(legend, plottedLines):
	'''
	Helper function that helps create a dictionary that connects a plotted line
	to its reference in the legent itself.
	Inputs should be a legend handle and an array of handles to plotted lines
	'''
	linker = {}
	for legLine, plotLine in zip(legend.get_lines(), plottedLines):
		legLine.set_picker(5)    # set tolerance
		linker[legLine] = plotLine

	return linker

def plotLinkData(time, linksData):
	'''
	Given array of time data and dictionary containg metrics for all links,
	generate three plots: 1 showing each metric for all links.
	'''

	f, (lrates, buffocc, pktloss) = plt.subplots(3, 1, sharex=True)

	# For the purpose of linking each plotted Data Set to its line in the
	# legend so that user can interactively select which data to view
	linkLines = []
	bufocLines = []
	pktloLines = []
	
	# plot link rates, buff occup, packet loss for all links
	# note: 'o' is to ensure scatterplot
	for link in linksData.keys():
		lhandle, = lrates.plot(time, linksData[link]["Link Rate (Mbps)"], '-',
				label=str(link),
				markersize=2, markeredgewidth=0.0)
		bohandle, = buffocc.plot(time, linksData[link]["Buffer Occupancy (KB)"], '-',
				label=str(link),
				markersize=2, markeredgewidth=0.0)
		plhandle, = pktloss.plot(time, linksData[link]["Packet Loss (pkts)"], '-',
				label=str(link),
				markersize=2, markeredgewidth=0.0)
	
		# update array of handles to plotted lines
		linkLines.append(lhandle)
		bufocLines.append(bohandle)
		pktloLines.append(plhandle)

	# add titles, labels and legend; set lower bound of y axis
	lrates.set_xlabel('Time (ms)')
	lrates.set_ylabel('Link Rate (Mbps)')
	lleg = lrates.legend(loc='best')
	lrates.set_ylim(ymin=0)
	
	buffocc.set_xlabel('Time (ms)')
	buffocc.set_ylabel('Buffer Occupancy (KB)')
	boleg = buffocc.legend(loc='best')
	buffocc.set_ylim(ymin=0)
	
	pktloss.set_xlabel('Time (ms)')
	pktloss.set_ylabel('Packet Loss (pkts)')
	pleg = pktloss.legend(loc='best')
	pktloss.set_ylim(ymin=0)

	# set window title
	f.canvas.set_window_title("Link Metrics Graph")

	# create dictionary connecting line in legend to plot
	linkDict = makeLegDict(lleg, linkLines)
	bfDict = makeLegDict(boleg, bufocLines)
	plDict = makeLegDict(pleg, pktloLines)

	def onpick(event):
	    # on the pick event, find the orig line corresponding to the
	    # legend proxy line, and toggle the visibility
	    legline = event.artist
	    origline = linkDict[legline]
	    vis = not origline.get_visible()
	    origline.set_visible(vis)
	    # Change the alpha on the line in the legend so we can see what lines
	    # have been toggled
	    if vis:
	        legline.set_alpha(1.0)
	    else:
	        legline.set_alpha(0.2)
	    f.canvas.draw()

	f.canvas.mpl_connect('pick_event', onpick)

def plotFlowData(time, flowsData):
	'''
	Given array of time data and dictionary containg metrics for all flows,
	generate three plots: 1 showing each metric for all flows.
	'''
	f, (frates, winsize, pktdelay) = plt.subplots(3, 1, sharex=True)

	# For the purpose of linking each plotted Data Set to its line in the
	# legend so that user can interactively select which data to view
	flowLines = []
	winLines = []
	pktdLines = []
	
	# plot flow rates, buff occup, packet loss for all links
	# note: 'o' is to ensure scatterplot
	for flow in flowsData.keys():
		# note: the commas after assigning plotted line handle are necessary
		fhandle, = frates.plot(time, flowsData[flow]["Flow Rate (Mbps)"], '',
				label=str(flow),
				markersize=2, markeredgewidth=0.0)
		whandle, = winsize.plot(time, flowsData[flow]["Window Size (pkts)"], '-',
				label=str(flow),
				markersize=2, markeredgewidth=0.0)
		pdhandle, = pktdelay.plot(time, flowsData[flow]["Packet Delay (ms)"], '-',
				label=str(flow),
				markersize=2, markeredgewidth=0.0)

		# update array of handles to plotted lines
		flowLines.append(fhandle)
		winLines.append(whandle)
		pktdLines.append(pdhandle)
	
	# add titles, labels, and legends to plots; set lower bound of y axis
	frates.set_xlabel('Time (ms)')
	#frates.set_ylabel('Flow Rate (Mbps)')
	frates.set_ylabel('Flow Percentage (Mbps)') # useful for debugging purposes
	fleg = frates.legend(loc='best')
	frates.set_ylim(ymin=0)
	
	winsize.set_xlabel('Time (ms)')
	winsize.set_ylabel('Window Size (pkts)')
	winleg = winsize.legend(loc='best')
	winsize.set_ylim(ymin=0)
	
	pktdelay.set_xlabel('Time (ms)')
	pktdelay.set_ylabel('Packet Delay (ms)')
	pktleg = pktdelay.legend(loc='best')
	pktdelay.set_ylim(ymin=0)

	# set window title
	f.canvas.set_window_title("Flow Metrics Graph")

	FlowDict = makeLegDict(fleg, flowLines)
	wxDict = makeLegDict(winleg, winLines)
	pdDict = makeLegDict(pktleg, pktdLines)

	def onpick(event):
	    # on the pick event, find the orig line corresponding to the
	    # legend proxy line, and toggle the visibility
	    legline = event.artist
	    origline = FlowDict[legline]
	    vis = not origline.get_visible()
	    origline.set_visible(vis)
	    # Change the alpha on the line in the legend so we can see what lines
	    # have been toggled
	    if vis:
	        legline.set_alpha(1.0)
	    else:
	        legline.set_alpha(0.2)
	    f.canvas.draw()

	f.canvas.mpl_connect('pick_event', onpick)

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
