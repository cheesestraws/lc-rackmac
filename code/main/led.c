#include <stdint.h>

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "pins.h"
#include "led.h"

#define FLASH_LENGTH 60

TaskHandle_t ltact_task;
TaskHandle_t lterr_task;

void led_task_runloop (void* param) {
	gpio_num_t pin = (gpio_num_t)param; // thanks, I hate it
	
	uint32_t dummy_value = 0;
	
	/* We just run in a loop, waiting to be notified, flashing our little
	   light, then going back to sleep again. */
	while(1) {
		xTaskNotifyWait(0, 0, &dummy_value, portMAX_DELAY);
		gpio_set_level(pin, 0);
		vTaskDelay(FLASH_LENGTH / portTICK_PERIOD_MS);
		gpio_set_level(pin, 1);
		vTaskDelay(FLASH_LENGTH / portTICK_PERIOD_MS);
	}
}

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
	
	xTaskCreate(&led_task_runloop, "lt_act_task", 1024, (void*)PIN_LT_ACT_LED, tskIDLE_PRIORITY, &ltact_task);
	xTaskCreate(&led_task_runloop, "lt_err_task", 1024, (void*)PIN_LT_ERR_LED, tskIDLE_PRIORITY, &lterr_task);
}

void led_act_flash(void) {
	xTaskNotify(ltact_task, 1, eSetValueWithOverwrite);
}

void led_err_flash(void) {
	xTaskNotify(lterr_task, 1, eSetValueWithOverwrite);
}

void led_boot_complete(void) {
	ESP_ERROR_CHECK(gpio_set_level(PIN_LT_ACT_LED, 1));
	ESP_ERROR_CHECK(gpio_set_level(PIN_LT_ERR_LED, 1));
	ESP_ERROR_CHECK(gpio_set_level(PIN_HDLED_OUT, 1));
}