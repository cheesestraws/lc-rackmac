#include <string.h>

#include <esp_mac.h>
#include <esp_err.h>
#include <mdns.h>

#include "tunables.h"
#include "mdns_service.h"

static char *generate_hostname(void) {
	uint8_t mac[6];
	char   *hostname;
	esp_read_mac(mac, ESP_MAC_WIFI_STA);
	if (-1 == asprintf(&hostname, "%s-%02X%02X%02X", BASE_HOSTNAME, mac[3], mac[4], mac[5])) {
		abort();
	}

	return hostname;
}

void mdns_start(void) {
	char *hostname = generate_hostname();
	
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set(hostname));
    ESP_ERROR_CHECK(mdns_instance_name_set(hostname));
    
    ESP_ERROR_CHECK(mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0));
    ESP_ERROR_CHECK(mdns_service_add(NULL, "_lcbmc", "_tcp", 80, NULL, 0));
    
    mdns_txt_item_t serviceTxtData[1] = {
        {"path", "/metrics"},
    };
    
	ESP_ERROR_CHECK(mdns_service_add(NULL, "_prometheus", "_tcp", 80, serviceTxtData, 1));
	
	free(hostname);
}