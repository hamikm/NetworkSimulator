/*
 * See header file for comments.
 */

#include "packet.h"

packet::packet(packet_type type, string source_ip,
	   string dest_ip, int seqnum, netflow &parent_flow) :
			netelement (""), t(type),
			source_ip(source_ip), dest_ip(dest_ip) {

	switch (type) {
	case FLOW:
		this->seqnum = seqnum;
		this->size = FLOW_PACKET_SIZE;
		this->parent_flow = &parent_flow;
		break;
	case ACK:
		this->seqnum = NO_FLOW_SEQNUM;
		this->size = ACK_PACKET_SIZE;
		this->parent_flow = &parent_flow;
		break;
	case ROUTING:
		this->seqnum = NO_FLOW_SEQNUM;
		this->size = ROUTING_PACKET_SIZE;
		this->parent_flow = NULL;
		break;
	default:
		assert(false); // no other packets types
	}
}
