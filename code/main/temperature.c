#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <ds18x20.h>

#include "pins.h"

#include "temperature.h"

static const char* TAG = "TEMPERATURE";

TaskHandle_t temperature_poll_task_handle;

void temperature_poll_task(void* dummyParameter) {
	esp_err_t err = 0;
	float temperature = 0;

	ESP_ERROR_CHECK(gpio_reset_pin(PIN_1WIRE));
	ESP_ERROR_CHECK(gpio_set_direction(PIN_1WIRE, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_set_pull_mode(PIN_1WIRE, GPIO_FLOATING));

	for(;;) {
		err = ds18b20_measure_and_read(PIN_1WIRE, DS18X20_ANY, &temperature);
		if (err != ESP_OK) {
            ESP_LOGE(TAG, "Could not read from sensor: %d (%s)", err, esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "Sensor: %.2f°C", temperature);
    	}
	
		vTaskDelay(15000 / portTICK_PERIOD_MS);
	}
}

void temperature_start(void) {
	xTaskCreate(&temperature_poll_task, "temperature_poll_task", 2048, NULL, 5, &temperature_poll_task_handle);
}