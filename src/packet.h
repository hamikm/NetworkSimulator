/**
 * @file
 * @author Jessica Li, Jingwen Wang, Hamik Mukelyan
 */

#ifndef PACKET_H
#define PACKET_H

#include <iostream>
#include <cassert>
#include <string>
#include "nethost.h"

using namespace std;

class packet {
private:

	/* Type of packet: either flow (payload), acknowledgement, or
	routing. */
	packetType t;

	/* Packet size in mb (to be consistent with flow) */
	float size;

	/* Sequence number of packet */
	int seq;

	/* IP address of source host */
	string source_ip;

	/* IP address of destination host */
	string dest_ip;

public:
	enum packetType {
		FLOW,
		ACK,
		ROUTING
	};

	packet(packetType type, float size, string source_ip, 
		string dest_ip, int seq) {
		t = type;
		this->size = size;
		this->source_ip = source_ip;
		this->dest_ip = dest_ip;
		this->seq = seq;
	}

	string getSource() { return source; }

	string getDestination() { return destination; }

	int getSeq() { return seq; }

	bool isAck() { return packetType == ACK; }

};

#endif // PACKET_H