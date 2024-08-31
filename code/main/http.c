#include <stdbool.h>
#include <ctype.h>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include "ota.h"
#include "html_fragments.h"

#include "http.h"

static const char* TAG = "HTTP";


void urlndecode(char *dst, const char *src, size_t len)
{
	char a, b;
	size_t cnt = 0;
	while (*src) {
		if ((*src == '%') &&
			((a = src[1]) && (b = src[2])) &&
			(isxdigit(a) && isxdigit(b))) {
				if (a >= 'a')
						a -= 'a'-'A';
				if (a >= 'A')
						a -= ('A' - 10);
				else
						a -= '0';
				if (b >= 'a')
						b -= 'a'-'A';
				if (b >= 'A')
						b -= ('A' - 10);
				else
						b -= '0';
				*dst++ = 16*a+b;
				src+=3;
		} else if (*src == '+') {
				*dst++ = ' ';
				src++;
		} else {
				*dst++ = *src++;
		}
		cnt++;
		if (cnt >= (len - 1)) {
			break;
		}
	}
	*dst++ = '\0';
}

/* root */

esp_err_t http_root_handler(httpd_req_t *req) {
    const char resp[] = 
    HTML_TOP
    "hello world"
    HTML_BOTTOM;
    
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t http_root = {
	.uri = "/",
	.method = HTTP_GET,
	.handler = http_root_handler,
	.user_ctx = NULL
};

/* ota handlers */

esp_err_t http_ota_status_handler(httpd_req_t *req) {
	httpd_resp_send_chunk(req, HTML_TOP, HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, get_ota_status(), HTTPD_RESP_USE_STRLEN);
	httpd_resp_send_chunk(req, GO_BACK_BTN, HTTPD_RESP_USE_STRLEN);
	httpd_resp_send_chunk(req, HTML_BOTTOM, HTTPD_RESP_USE_STRLEN);
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

httpd_uri_t http_ota_status = {
	.uri = "/ota/status",
	.method = HTTP_GET,
	.handler = http_ota_status_handler,
	.user_ctx = NULL
};

esp_err_t http_ota_upgrade_handler(httpd_req_t *req) {
	char url[256] = { 0 };
	char encoded_url[256] = { 0 };
	bool got_url = false;
	char* qry;

	// Do we have a query string?
	size_t buf_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_len > 1) {
		qry = malloc(buf_len);
		if (httpd_req_get_url_query_str(req, qry, buf_len) == ESP_OK) {		
			if (httpd_query_key_value(qry, "url", encoded_url, sizeof(encoded_url)) == ESP_OK) {
				urlndecode(url, encoded_url, sizeof(url));
				got_url = true;
				ESP_LOGI(TAG, "starting ota with url => %s", url);
			}
		}
	}
	
	if (got_url) {
		do_ota(url);
		httpd_resp_send(req, HTML_TOP "Starting... go <a href='/ota/status'>here</a> for status." HTML_BOTTOM, HTTPD_RESP_USE_STRLEN);
	} else {
		httpd_resp_send(req, HTML_TOP "please give me a url to nom..." OK_BTN HTML_BOTTOM, HTTPD_RESP_USE_STRLEN);
	}
	
	return ESP_OK;
}

httpd_uri_t http_ota_upgrade = {
	.uri = "/ota/upgrade",
	.method = HTTP_GET,
	.handler = http_ota_upgrade_handler,
	.user_ctx = NULL
};

esp_err_t http_ota_start_handler(httpd_req_t *req) {
    httpd_resp_send(req,
    
    HTML_TOP
    
    "<form method='get' action='/ota/upgrade'>"
    "<input type='text' name='url'><input type='submit'>"
    "</form>"
    
    HTML_BOTTOM
    
    , HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t http_ota_start = {
	.uri = "/ota/start",
	.method = HTTP_GET,
	.handler = http_ota_start_handler,
	.user_ctx = NULL
};

/* Start the thing */

httpd_handle_t start_httpd(void) {
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	httpd_handle_t server = NULL;

	httpd_start(&server, &config);
	
	if(server != NULL) {
		httpd_register_uri_handler(server, &http_root);
		httpd_register_uri_handler(server, &http_ota_status);
		httpd_register_uri_handler(server, &http_ota_upgrade);
		httpd_register_uri_handler(server, &http_ota_start);
	}
	return server;
}
