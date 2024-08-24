#include <stdint.h>
#include <stddef.h>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "driver/pulse_cnt.h"

#include "pins.h"
#include "stats.h"

#include "gpio_interrupts.h"

static const char* TAG="GPIO_I";

TaskHandle_t gpio_intr_task_handle;
TaskHandle_t hd_led_task_handle;

pcnt_unit_handle_t pcnt_unit;

static void IRAM_ATTR hdled_isr(void* dummyParameter) {
	xTaskNotifyFromISR(hd_led_task_handle, 0, eSetValueWithOverwrite, NULL);
}

void hd_led_task(void *dummyParameter) {
	uint32_t dummy;
	uint32_t old_level = 1;

	for(;;) {
		xTaskNotifyWait(0, 0, &dummy, 1000 / portTICK_PERIOD_MS);
		uint32_t level = gpio_get_level(PIN_HDLED_IN);
		if (old_level == 1 && level == 0) {
			stats.hdled_count++;
		}
		old_level = level;
		gpio_set_level(PIN_HDLED_OUT, level);
	}
}

void gpio_intr_task(void *dummyParameter) {
	ESP_ERROR_CHECK(gpio_set_intr_type(PIN_HDLED_IN, GPIO_INTR_ANYEDGE));

	ESP_ERROR_CHECK(gpio_install_isr_service(0));
	ESP_ERROR_CHECK(gpio_isr_handler_add(PIN_HDLED_IN, hdled_isr, NULL));
	
	pcnt_unit_config_t unit_config = {
		.low_limit = -10000,
		.high_limit = 10000
	};
	ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

	ESP_LOGI(TAG, "install pcnt channels");
	pcnt_chan_config_t chan_config = {
		.edge_gpio_num = PIN_FANTACH,
		.level_gpio_num = -1
	};    
	pcnt_channel_handle_t pcnt_chan = NULL;
	ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_config, &pcnt_chan));
	ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan, PCNT_CHANNEL_EDGE_ACTION_HOLD, PCNT_CHANNEL_EDGE_ACTION_INCREASE));

	ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
	ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
	ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
	
	int fanrevcount = -1;
	
	for (;;) {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &fanrevcount));
		ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
		stats.fanrpm = fanrevcount * 30;
//		ESP_LOGI(TAG, "rev cnt: %d", fanrevcount);
		fanrevcount = 0;
	}
}

void gpio_intr_init(void) {
	// set up GPIO pins we want to be poked about
	ESP_ERROR_CHECK(gpio_reset_pin(PIN_HDLED_IN));
	ESP_ERROR_CHECK(gpio_set_direction(PIN_HDLED_IN, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_set_pull_mode(PIN_HDLED_IN, GPIO_FLOATING));
		
        
	// Should be pinned to a core
	xTaskCreate(&hd_led_task, "hd_led_intr_task", 2048, NULL, 5, &hd_led_task_handle);
	xTaskCreate(&gpio_intr_task, "gpio_intr_task", 8192, NULL, 5, &gpio_intr_task_handle);
}