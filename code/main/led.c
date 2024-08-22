#include "led.h"

void led_init(void) {
	ESP_ERROR_CHECK(gpio_reset_pin(PIN_LT_ACT_LED));
	ESP_ERROR_CHECK(gpio_set_direction(PIN_LT_ACT_LED, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_level(PIN_LT_ACT_LED, 0));

	ESP_ERROR_CHECK(gpio_reset_pin(PIN_LT_ERR_LED));
	ESP_ERROR_CHECK(gpio_set_direction(PIN_LT_ERR_LED, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_level(PIN_LT_ERR_LED, 0));
		
	ESP_ERROR_CHECK(gpio_reset_pin(PIN_HDLED_OUT));
	ESP_ERROR_CHECK(gpio_set_direction(PIN_HDLED_OUT, GPIO_MODE_OUTPUT));	
	ESP_ERROR_CHECK(gpio_set_level(PIN_HDLED_OUT, 0));
	
	gpio_dump_io_configuration(stdout, (1ULL << PIN_LT_ACT_LED) | (1ULL << PIN_HDLED_OUT));	
}

void led_boot_complete(void) {
	ESP_ERROR_CHECK(gpio_set_level(PIN_LT_ACT_LED, 1));
	ESP_ERROR_CHECK(gpio_set_level(PIN_LT_ERR_LED, 1));
	ESP_ERROR_CHECK(gpio_set_level(PIN_HDLED_OUT, 1));
}