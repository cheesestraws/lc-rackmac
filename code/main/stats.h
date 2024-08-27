#pragma once

#include <esp_err.h>
#include <stdatomic.h>

#include <esp_http_server.h>


typedef struct stats {
	_Atomic int fanrpm;
	_Atomic int hdled_count;
	_Atomic int bp_freakout_count;
	_Atomic int tashtalk_llap_rx_count;
	_Atomic int tashtalk_llap_too_long_count;
	_Atomic int tashtalk_waiting_for_buffer_count;
	_Atomic int tashtalk_crc_fail_count;
	_Atomic int tashtalk_framing_error_count;
	_Atomic int tashtalk_frame_abort_count;
} stats_t;

extern stats_t stats;

esp_err_t stats_to_prometheus(httpd_req_t *req);