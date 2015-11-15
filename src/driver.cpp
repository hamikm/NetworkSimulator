/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 *
 * This file reads a network description from a JSON file then starts a
 * simulation of the network. It logs data as the simulation progresses then
 * dumps it to to disk so scripts can generate graphs.
 *
 * TODO decide how the data is dumped... to stdout so it can be redirected to
 * a single file? To multiple files in the 'data_dump' directory?
 */

// ---------------------------- Includes --------------------------------------

// Standard headers.
#include <iostream>
#include <string>
#include <cstdlib>
#include "simulation.h"

using namespace std;

// --------------------------- Prototypes -------------------------------------

/**
 * Prints a usage statement to stderr.
 * @param progname name of this program.
 */
void print_usage_statement (char *progname);

// ------------------------ Global variables ----------------------------------



// ------------------------------ Main ----------------------------------------

/**
 * Reads a JSON file from disk, populates in-memory collections of hosts,
 * routers, links, and flows, starts a simulation, then logs data.
 *
 * TODO describe how we log data to disk. Multiple files? To stdout intending
 * that it will get redirected to a single file?
 */
int main (int argc, char **argv) {

	// See print_usage_statement function for expected console arguments.
	if (argc != 2) {
		print_usage_statement(argv[0]);
		exit (1);
	}

	// TODO make data logger object (not in its own thread...)

	// Load hosts, routers, links, and flows from the JSON input file.
	// TODO pass in the data logger object
	simulation sim(argv[1]);

	// Invoke the simulation loop, which should terminate when all events
	// have been processed.
	sim.runSimulation();

	// TODO flush the data logger's data to disk

	return 0;
}

// ---------------------------- Functions  ------------------------------------

void print_usage_statement (char *progname) {
	cerr << "Usage: " << progname << " <JSON input file>" << endl;
	// TODO update when we decide where to send output.
}
