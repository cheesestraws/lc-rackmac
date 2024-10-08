/* OTA example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include <esp_http_server.h>
#include <esp_netif.h>
#include "string.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <sys/socket.h>

#include "ethernet.h"
#include "wifi.h"
#include "http.h"
#include "tunables.h"
#include "http_noncore.h"
#include "ota.h"
#include "led.h"
#include "reset.h"
#include "gpio_interrupts.h"
#include "temperature.h"
#include "buffer_pool.h"
#include "uart.h"
#include "packet_types.h"
#include "beep.h"
#include "mdns_service.h"

void app_main(void)
{

	led_init();
	reset_init();
	temperature_start();
	sound_init();
	
	buffer_pool_t* packet_pool = new_buffer_pool(180, sizeof(llap_packet));
	uart_init();
	uart_start(packet_pool);

    // Initialize NVS.
	esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // 1.OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // 2.NVS partition contains data in new format and cannot be recognized by this version of code.
        // If this happens, we erase NVS partition and initialize NVS again.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
	#ifndef WIFI
	ethernet_init();
	#else
	wifi_init();
	#endif
	httpd_handle_t httpd = start_httpd();
	install_noncore_handlers(httpd);
	mdns_start();
	start_ota();
	
	gpio_intr_init();
	led_boot_complete();
	bootbeep();
	reset_open();
}
