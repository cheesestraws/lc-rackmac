#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_https_ota.h>
#include <esp_tls.h>
#include <esp_crt_bundle.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "ota.h"

static const char* TAG = "OTA";

_Atomic(char*) current_status = "no update in progress";

#define URL_BUFFER_SIZE 512
static char url_buf[URL_BUFFER_SIZE];

TaskHandle_t ota_task_handle;

char* get_ota_status() {
	return current_status;
}

void set_ota_status(char* status) {
	current_status = status;
	ESP_LOGI(TAG, "ota status: %s", status);
}

bool do_ota(char* url) {
	int l = strnlen(url, URL_BUFFER_SIZE);
	if (l == URL_BUFFER_SIZE) {
		return false;
	} else {
		strlcpy(url_buf, url, URL_BUFFER_SIZE);
		xTaskNotify(ota_task_handle, 1, eSetValueWithOverwrite);
		return true;
	}
}

static void ota_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	if (event_base == ESP_HTTPS_OTA_EVENT) {
		switch (event_id) {
			case ESP_HTTPS_OTA_START:
				set_ota_status("OTA started");
				break;
			case ESP_HTTPS_OTA_CONNECTED:
				set_ota_status("Connected to server");
				break;
			case ESP_HTTPS_OTA_GET_IMG_DESC:
				set_ota_status("Reading Image Description");
				break;
			case ESP_HTTPS_OTA_VERIFY_CHIP_ID:
				set_ota_status("Verifying chip id of new image");
				break;
			case ESP_HTTPS_OTA_DECRYPT_CB:
				set_ota_status("Callback to decrypt function");
				break;
			case ESP_HTTPS_OTA_WRITE_FLASH:
				set_ota_status("Writing to flash");
				break;
			case ESP_HTTPS_OTA_UPDATE_BOOT_PARTITION:
				set_ota_status("Boot partition updated");
				break;
			case ESP_HTTPS_OTA_FINISH:
				set_ota_status("OTA finished");
				break;
			case ESP_HTTPS_OTA_ABORT:
				set_ota_status("OTA aborted");
				break;
		}
	}
}

void ota_task(void *pvParameter)
{
	uint32_t dummy;
	
	while(1) {
		// Wait to be tickled
		xTaskNotifyWait(0, 0, &dummy, portMAX_DELAY);
		
		char local_response_buffer[2048 + 1] = {0};

		 esp_http_client_config_t xconfig = {
			.host = "192.168.0.1",
			.path = "/",
			.query = "",
			.user_data = local_response_buffer,        // Pass address of local buffer to get response
		};
		esp_http_client_handle_t xclient = esp_http_client_init(&xconfig);

		esp_err_t err = esp_http_client_perform(xclient);
		if (err == ESP_OK) {
			ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %"PRId64,
					esp_http_client_get_status_code(xclient),
					esp_http_client_get_content_length(xclient));
		} else {
			ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
		}
		ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));
		
		esp_http_client_config_t config = {
			.url = url_buf,
			.keep_alive_enable = true,
			.crt_bundle_attach = esp_crt_bundle_attach,
		};

		esp_https_ota_config_t ota_config = {
			.http_config = &config,
		};
		ESP_LOGI(TAG, "Attempting to download update from %s", config.url);
		esp_err_t ret = esp_https_ota(&ota_config);
		if (ret == ESP_OK) {
			ESP_LOGI(TAG, "OTA Succeed, Rebooting...");
			esp_restart();
		} else {
			ESP_LOGE(TAG, "Firmware upgrade failed");
		}
	}
}

void start_ota(void) {
	ESP_ERROR_CHECK(esp_event_handler_register(ESP_HTTPS_OTA_EVENT, ESP_EVENT_ANY_ID, &ota_event_handler, NULL));
	
	xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, &ota_task_handle);
}
