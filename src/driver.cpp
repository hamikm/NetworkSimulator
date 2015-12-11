/**
 * @file
 * This file reads a network description from a JSON file then starts a
 * simulation of the network. It logs data as the simulation progresses then
 * dumps it to to disk so scripts can generate graphs.
 */

// ---------------------------- Includes --------------------------------------

// Standard headers.
#include <iostream>
#include <string>
#include <cstdlib>
#include <string.h>
#include <csignal>

// Custom headers.
#include "simulation.h"

using namespace std;

// --------------------------- Prototypes -------------------------------------

/**
 * Prints a usage statement to stderr.
 * @param progname name of this program.
 */
void print_usage_statement (char *progname);

/**
 * Processes console arguments and sets the global variables @c infile and
 * @c outfile.
 * @param argc number of console arguments
 * @param argv console arguments
 * @post sets the debug flag and infile and outfile global variables.
 * @warning exits from this process after printing a usage statement if the
 * console arguments don't conform to the usage statement
 */
void process_console_args(int argc, char **argv);

/**
 * Called when the program terminates unexpectedly to append some crucial
 * characters to the output JSON file.
 * @param signal
 */
void term_sig_handler(int signal);

// ------------------------ Global variables ----------------------------------

/** If true lots of debugging output is shown. */
bool debug = false;

/**
 * If true even more debugging output is shown and the output pauses between
 * events for analysis.
 */
bool detail = false;

/**
 * Output stream to which to write debugging statements. Might be customized
 * to be a file output stream, stderr, or something else.
 */
ostream &debug_os = cout;

/**
 * The simulation object is a global variable so that the signal handler can
 * see it when it tries to clean up the output file.
 */
simulation *sim;

/** Input filename */
char *infile;

/** Output filename */
char *outfile;

// ------------------------------ Main ----------------------------------------

/**
 * Reads a JSON file from disk, populates in-memory collections of hosts,
 * routers, links, and flows, starts a simulation, then logs data.
 */
int main (int argc, char **argv) {

	signal(SIGINT, term_sig_handler);
	signal(SIGSEGV, term_sig_handler);
	signal(SIGTERM, term_sig_handler);

	process_console_args(argc, argv);

	// Load hosts, routers, links, and flows from the JSON input file.
	sim = new simulation(infile);

	// Invoke the simulation loop, which should terminate when all events
	// have been processed. Every time an event is executed, network sim
	// metrics are logged.
	sim->initializeLog(outfile);
	sim->runSimulation();
	sim->closeLog();

	delete sim;

	return 0;
}

// ---------------------------- Functions  ------------------------------------

void term_sig_handler(int signal) {
	if (sim != NULL)
		sim->closeLog();
	exit(1);
	return;
}

void print_usage_statement (char *progname) {
	cerr << endl << "Usage: " << progname << " <JSON input file> "
			"<JSON output file> [-d|-dd]" << endl;
	cerr << "  -d to print debugging statements to stdout." << endl;
	cerr << "  -dd to print detailed, pausing debugging statements to stdout."
			<< endl << endl << "Note that the debug flags must come after "
			"the two required filenames." << endl << endl;
}

void process_console_args(int argc, char **argv) {

	// See print_usage_statement function for expected console arguments.
	if (argc != 3 && argc != 4) {
		print_usage_statement(argv[0]);
		exit (1);
	}

	// No flags
	if (argc == 3) {
		debug = false;
		detail = false;
	}

	// One debug flag
	if (argc == 4) {
		debug = true;
		if (strcmp(argv[3], "-dd") == 0) {
			detail = true;
		}
	}

	infile = argv[1];
	outfile = argv[2];
}
