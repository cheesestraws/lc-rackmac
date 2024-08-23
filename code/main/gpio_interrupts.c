#include <stdint.h>
#include <stddef.h>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "pins.h"

#include "gpio_interrupts.h"

static const char* TAG="GPIO_I";

TaskHandle_t gpio_intr_task_handle;
TaskHandle_t hd_led_task_handle;


_Atomic uint32_t fanrevcount;

static void IRAM_ATTR hdled_isr(void* dummyParameter) {
	xTaskNotifyFromISR(hd_led_task_handle, 0, eSetValueWithOverwrite, NULL);
}

static void IRAM_ATTR fantach_isr(void* dummyParameter) {
	fanrevcount++;
}

void hd_led_task(void *dummyParameter) {
	uint32_t dummy;

	for(;;) {
		xTaskNotifyWait(0, 0, &dummy, 500 / portTICK_PERIOD_MS);
		uint32_t level = gpio_get_level(PIN_HDLED_IN);
		gpio_set_level(PIN_HDLED_OUT, level);
	}
}

void gpio_intr_task(void *dummyParameter) {
	ESP_ERROR_CHECK(gpio_set_intr_type(PIN_HDLED_IN, GPIO_INTR_POSEDGE));
	ESP_ERROR_CHECK(gpio_set_intr_type(PIN_HDLED_IN, GPIO_INTR_ANYEDGE));

	ESP_ERROR_CHECK(gpio_install_isr_service(0));
	ESP_ERROR_CHECK(gpio_isr_handler_add(PIN_FANTACH, fantach_isr, NULL));
	ESP_ERROR_CHECK(gpio_isr_handler_add(PIN_HDLED_IN, hdled_isr, NULL));
	
	for (;;) {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		ESP_LOGI(TAG, "fan revs: %"PRIu32, fanrevcount);
		fanrevcount = 0;
	}
}

void gpio_intr_init(void) {
	// set up GPIO pins we want to be poked about
	ESP_ERROR_CHECK(gpio_reset_pin(PIN_HDLED_IN));
	ESP_ERROR_CHECK(gpio_set_direction(PIN_HDLED_IN, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_pull_mode(PIN_HDLED_IN, GPIO_FLOATING));
	
	ESP_ERROR_CHECK(gpio_reset_pin(PIN_FANTACH));
	ESP_ERROR_CHECK(gpio_set_direction(PIN_FANTACH, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_pull_mode(PIN_FANTACH, GPIO_FLOATING));

	// Should be pinned to a core
	xTaskCreate(&hd_led_task, "hd_led_intr_task", 2048, NULL, 5, &hd_led_task_handle);
	xTaskCreate(&gpio_intr_task, "gpio_intr_task", 8192, NULL, 5, &gpio_intr_task_handle);
}