#include "reset.h"

#include "http_noncore.h"

esp_err_t http_reset_handler(httpd_req_t *req) {
    const char resp[] = "reset done";
    reset_do();
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t http_reset = {
	.uri = "/reset",
	.method = HTTP_GET,
	.handler = http_reset_handler,
	.user_ctx = NULL
};


void install_noncore_handlers(httpd_handle_t httpd) {
	httpd_register_uri_handler(httpd, &http_reset);
}
