#pragma once

#include <stdint.h>

// actually represents an LLAP packet + the FCS
typedef struct {
	int length;
	unsigned char packet[605]; /* 603 max length of LLAP packet: Inside 
	                              Appletalk2nd. Ed. p. 1-6) + 2 bytes FCS. */
} llap_packet;
