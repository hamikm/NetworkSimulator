/*
 * See header file for comments.
 */

#include "netlink.h"

void netlink::constructor_helper(float rate_mbps, int delay_ms, int buflen_kb,
		netelement *endpoint1, netelement *endpoint2) {
	this->rate_bpms = rate_mbps * BYTES_PER_MEGABIT / MS_PER_SEC;
	this->delay_ms = delay_ms;
	this->buflen_bytes = buflen_kb * BYTES_PER_KB;
	this->endpoint1 = endpoint1 == NULL ? NULL : endpoint1;
	this->endpoint2 = endpoint2 == NULL ? NULL : endpoint2;
	this->available_at_time_ms = 0;
}

/*
double sendPacket(packet pkt, double current_time) {




	// TODO







	return 0;
}
*/
