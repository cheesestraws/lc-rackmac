#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "pins.h"
#include "stats.h"

#include "reset.h"

// reset.c also manages on/off if ONOFF is set in tunables.h

void reset_init(void) {
	ESP_ERROR_CHECK(gpio_reset_pin(PIN_RESET));
	ESP_ERROR_CHECK(gpio_set_direction(PIN_RESET, GPIO_MODE_OUTPUT));
	#ifndef ONOFF
	ESP_ERROR_CHECK(gpio_set_level(PIN_RESET, 1));
	#else
	ESP_ERROR_CHECK(gpio_set_level(PIN_RESET, 0));
	#endif
}

void reset_close(void) {
	ESP_ERROR_CHECK(gpio_set_level(PIN_RESET, 1));
	stats.relay_status = 1;
}

void reset_open(void) {
	ESP_ERROR_CHECK(gpio_set_level(PIN_RESET, 0));
	stats.relay_status = 0;
}

void reset_do(void) {
	#ifdef ONOFF
	reset_open();
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	#endif
	reset_close();
	#ifndef ONOFF
	vTaskDelay(100 / portTICK_PERIOD_MS);
	reset_open();
	#endif
}
