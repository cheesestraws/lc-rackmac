#include <stdlib.h>

#include <esp_err.h>
#include <esp_http_server.h>

#include "tunables.h"
#include "stats.h"

stats_t stats;

char* stats_fmt =
	"#TYPE fan_speed_rpm gauge\n"
	"fan_speed_rpm %d\n"
	"#TYPE hd_operation_count counter\n"
	"hd_operation_count %d\n"
	"#TYPE bufferpool_panic_count counter\n"
	"bufferpool_panic_count %d\n"
	"#TYPE tashtalk_llap_rx_count counter\n"
	"tashtalk_llap_rx_count %d\n"
	"#TYPE tashtalk_llap_too_long_count counter\n"
	"tashtalk_llap_too_long_count %d\n"
	"#TYPE tashtalk_waiting_for_buffer_count counter\n"
	"tashtalk_waiting_for_buffer_count %d\n"
	"#TYPE tashtalk_crc_fail_count counter\n"
	"tashtalk_crc_fail_count %d\n"
	"#TYPE tashtalk_framing_error_count counter\n"
	"tashtalk_framing_error_count %d\n"
	"#TYPE tashtalk_frame_abort_count counter\n"
	"tashtalk_frame_abort_count %d\n"
	"#TYPE temperature gauge\n"
	"temperature %f\n"
	
	#ifdef ONOFF
	"#TYPE power_status gauge\n"
	"power_status %d\n"
	#endif
	"\n";


esp_err_t stats_to_prometheus(httpd_req_t *req) {
	size_t len = strlen(stats_fmt) * 2; // I'm not proud of this but it'll do
	char* buf = malloc(len);
	
	snprintf(buf, len, stats_fmt,
		stats.fanrpm,
		stats.hdled_count,
		stats.bp_freakout_count,
		stats.tashtalk_llap_rx_count,
		stats.tashtalk_llap_too_long_count,
		stats.tashtalk_waiting_for_buffer_count,
		stats.tashtalk_crc_fail_count,
		stats.tashtalk_framing_error_count,
		stats.tashtalk_frame_abort_count,
		stats.temperature
		#ifdef ONOFF
		,stats.relay_status
		#endif
		);
		
	httpd_resp_set_type(req, "text/plain");
	httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);

	free(buf);
	
	return ESP_OK;
}