/*
 * See header file for comments.
 */

#include "netlink.h"

void netlink::constructor_helper(float rate_mbps, int delay, int buflen_kb,
		netelement *endpoint1, netelement *endpoint2) {
	this->rate = (long)(rate_mbps * BYTES_PER_MEGABIT);
	this->delay = delay;
	this->buflen = buflen_kb * BYTES_PER_KB;
	this->endpoint1 = endpoint1 == NULL ? NULL : endpoint1;
	this->endpoint2 = endpoint2 == NULL ? NULL : endpoint2;
	this->buf_occupancy = 0;
	this->available_at_time = 0;
}
