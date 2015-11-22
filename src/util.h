/**
 * @file
 * @auther Jessica Li, Jingwen Wang, Hamik Mukelyan
 *
 * Contains some constants used in the simulation.
 */

#ifndef UTIL_H
#define UTIL_H

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

/**
 * Some negative number is used at the return value from functions which
 * normally return the arrival time of a packet; the client should check
 * if the returned value was negative rather than using equality comparison
 * with this sentinel value.
 */
const double PKT_DROPPED_SENTINEL = -1;

/** Types of packets in this simulation. */
enum packet_type {
	FLOW,
	ACK,
	ROUTING
};

/** Size of a flow packet in bytes. */
static const long FLOW_PACKET_SIZE = 1024;

/** Size of an ACK packet in bytes. */
static const long ACK_PACKET_SIZE = 64;

/** Size of a routing packet in bytes. */
static const long ROUTING_PACKET_SIZE = 64;

/** A sentinel used for the sequence numbers of routing and ACK packets. */
static const int SEQNUM_FOR_NONFLOWS = -1;

#endif // UTIL_H
