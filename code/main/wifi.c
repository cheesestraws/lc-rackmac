#include "esp_wifi.h"

#include "tunables.h"
#include "mdns_service.h"

#include "wifi.h"

void wifi_init(void) {
	esp_netif_t* wifi_if = esp_netif_create_default_wifi_sta();
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
	char* hostname = generate_hostname();
	ESP_ERROR_CHECK(esp_netif_set_hostname(wifi_if, hostname));
	free(hostname);
	ESP_ERROR_CHECK(esp_wifi_start() );
}