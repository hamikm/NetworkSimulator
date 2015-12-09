/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 *
 * Contains some constants used in the simulation.
 */

#ifndef UTIL_H
#define UTIL_H

/** Size of a flow packet in bytes. */
static const long FLOW_PACKET_SIZE = 1024;

/** Size of an ACK packet in bytes. */
static const long ACK_PACKET_SIZE = 64;

/** Size of a routing packet in bytes. */
static const long ROUTING_PACKET_SIZE = 64;

const int BYTES_PER_KB = 1 << 10;

/**  Conversion factor between kilobytes and megabytes. */
const int KB_PER_MB = 1 << 10;

/**  Conversion factor between bits and bytes. */
const int BITS_PER_BYTE = 1 << 3;

/** Input rate is in mbps, so this is used to convert to bytes per sec. */
const int BYTES_PER_MEGABIT =
		BYTES_PER_KB * KB_PER_MB / BITS_PER_BYTE;

/** Milliseconds per second. */
const int MS_PER_SEC = 1000;

/** Types of packets in this simulation. */
enum packet_type {
	FLOW,
	ACK,
	ROUTING
};

/** A sentinel used for the sequence numbers of routing packets. */
const int SEQNUM_FOR_NONFLOWS = -1;

/** Time interval in ms over which to compute flow rate */
const int RATE_INTERVAL = 1000;

/** Send routing packets until this time (in milliseconds) is reached. */
const int UPPER_TIME_ROUTING_LIMIT = 400000;

/** Print information about packets every (this many) packets. */
const int PRINT_PACKET_INFO_MILESTONE = 5000;

/** Log every (this many) events. */
const int LOG_FREQUENCY = 10;

#endif // UTIL_H
