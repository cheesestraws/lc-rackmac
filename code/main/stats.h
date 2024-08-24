#pragma once

#include <stdatomic.h>

typedef struct stats {
	_Atomic int fanrpm;
	_Atomic int hdled_count;
} stats_t;

extern stats_t stats;