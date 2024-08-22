#pragma once

#include <esp_http_server.h>

bool do_ota(char* url);
char* get_ota_status();
void start_ota(void);