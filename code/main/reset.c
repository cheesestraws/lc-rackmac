#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "pins.h"

#include "reset.h"

void reset_init(void) {
	ESP_ERROR_CHECK(gpio_reset_pin(PIN_RESET));
	ESP_ERROR_CHECK(gpio_set_direction(PIN_RESET, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_level(PIN_RESET, 1));
}

void reset_close(void) {
	ESP_ERROR_CHECK(gpio_set_level(PIN_RESET, 1));
}

void reset_open(void) {
	ESP_ERROR_CHECK(gpio_set_level(PIN_RESET, 0));
}

void reset_do(void) {
	reset_close();
	vTaskDelay(100 / portTICK_PERIOD_MS);
	reset_open();
}
