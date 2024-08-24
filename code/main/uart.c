#include <time.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include "pins.h"
#include "tashtalk.h"
#include "packet_types.h"

#include "uart.h"

static const char* TAG = "uart";
static const int uart_num = UART_NUM_1;

TaskHandle_t uart_rx_task = NULL;
QueueHandle_t uart_rx_queue = NULL;

#define RX_BUFFER_SIZE 1024

uint8_t uart_buffer[RX_BUFFER_SIZE];

void uart_init_tashtalk(void) {
	char init[1024] = { 0 };
	uart_write_bytes(uart_num, init, 1024);
}

void uart_init(void) {	
	const uart_config_t uart_config = {
		.baud_rate = 1000000,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
		.rx_flow_ctrl_thresh = 0,		
	};
	
	ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(uart_num, PIN_TT_TX, PIN_TT_RX, -1, PIN_TT_CTS));
	ESP_ERROR_CHECK(uart_driver_install(uart_num, 2048, 0, 0, NULL, 0));

	uart_init_tashtalk();

	ESP_LOGI(TAG, "uart_init complete");
}

void uart_rx_runloop(void* buffer_pool) {
	static const char *TAG = "UART_RX";	
	ESP_LOGI(TAG, "started");
	
	tashtalk_rx_state_t* rxstate = new_tashtalk_rx_state((buffer_pool_t*)buffer_pool, uart_rx_queue);
	
	while(1){
		/* TODO: this is stupid, read a byte at a time instead and wait for MAX_DELAY */
		const int len = uart_read_bytes(uart_num, uart_buffer, 1, 1000 / portTICK_PERIOD_MS);
		if (len > 0) {
			feed_all(rxstate, uart_buffer, len);
		}
	}
}

void uart_start(buffer_pool_t* packet_pool, QueueHandle_t rxQueue) {
	uart_rx_queue = rxQueue;
	xTaskCreate(&uart_rx_runloop, "UART_RX", 4096, (void*)packet_pool, 5, &uart_rx_task);
}
