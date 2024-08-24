#pragma once

#include <stdatomic.h>

typedef struct stats {
	_Atomic int fanrpm;
	_Atomic int hdled_count;
	_Atomic int bp_freakout;
} stats_t;

extern stats_t stats;