#include "reset.h"
#include "stats.h"
#include "html_fragments.h"

#include "http_noncore.h"

esp_err_t http_reset_handler(httpd_req_t *req) {
    reset_do();
	httpd_resp_send_chunk(req, HTML_TOP, HTTPD_RESP_USE_STRLEN);
	httpd_resp_send_chunk(req, "Reset done", HTTPD_RESP_USE_STRLEN);
	httpd_resp_send_chunk(req, OK_BTN, HTTPD_RESP_USE_STRLEN);
	httpd_resp_send_chunk(req, HTML_BOTTOM, HTTPD_RESP_USE_STRLEN);
    httpd_resp_sendstr_chunk(req, NULL);

    return ESP_OK;
}

httpd_uri_t http_reset = {
	.uri = "/reset",
	.method = HTTP_GET,
	.handler = http_reset_handler,
	.user_ctx = NULL
};

esp_err_t http_on_handler(httpd_req_t *req) {
    reset_close();
    
	httpd_resp_send_chunk(req, HTML_TOP, HTTPD_RESP_USE_STRLEN);
	httpd_resp_send_chunk(req, "Powered on", HTTPD_RESP_USE_STRLEN);
	httpd_resp_send_chunk(req, OK_BTN, HTTPD_RESP_USE_STRLEN);
	httpd_resp_send_chunk(req, HTML_BOTTOM, HTTPD_RESP_USE_STRLEN);
    httpd_resp_sendstr_chunk(req, NULL);

    return ESP_OK;
}

httpd_uri_t http_on = {
	.uri = "/on",
	.method = HTTP_GET,
	.handler = http_on_handler,
	.user_ctx = NULL
};

esp_err_t http_off_handler(httpd_req_t *req) {
    reset_open();

	httpd_resp_send_chunk(req, HTML_TOP, HTTPD_RESP_USE_STRLEN);
	httpd_resp_send_chunk(req, "Powered off", HTTPD_RESP_USE_STRLEN);
	httpd_resp_send_chunk(req, OK_BTN, HTTPD_RESP_USE_STRLEN);
	httpd_resp_send_chunk(req, HTML_BOTTOM, HTTPD_RESP_USE_STRLEN);
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

httpd_uri_t http_off = {
	.uri = "/off",
	.method = HTTP_GET,
	.handler = http_off_handler,
	.user_ctx = NULL
};

httpd_uri_t http_metrics = {
	.uri = "/metrics",
	.method = HTTP_GET,
	.handler = stats_to_prometheus, // in stats.c
	.user_ctx = NULL
};


void install_noncore_handlers(httpd_handle_t httpd) {
#ifdef ONOFF
	httpd_register_uri_handler(httpd, &http_on);
	httpd_register_uri_handler(httpd, &http_off);
#endif
	httpd_register_uri_handler(httpd, &http_reset);
	httpd_register_uri_handler(httpd, &http_metrics);
}
