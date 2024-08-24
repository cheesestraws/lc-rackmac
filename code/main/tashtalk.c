#include <strings.h>
#include <stdbool.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "tashtalk.h"
#include "packet_types.h"
#include "crc.h"
#include "uart.h"
#include "led.h"
#include "packet_utils.h"
#include "stats.h"

static const char* TAG = "tashtalk";

tashtalk_rx_state_t* new_tashtalk_rx_state(buffer_pool_t* buffer_pool) {

	tashtalk_rx_state_t* ns = malloc(sizeof(tashtalk_rx_state_t));
	
	if (ns == NULL) {
		return NULL;
	}
	
	bzero(ns, sizeof(tashtalk_rx_state_t));
	
	ns->buffer_pool = buffer_pool;
	
	return ns;
}

void append(llap_packet* packet, unsigned char byte) {
	if (packet->length < 605) {
		packet->packet[packet->length] = byte;
		packet->length++;
	} else {
		ESP_LOGE(TAG, "buffer overflow in packet");
		stats.tashtalk_llap_too_long_count++;
	}
}

void do_something_sensible_with_packet(tashtalk_rx_state_t* state) {
	led_act_flash();
	stats.tashtalk_llap_rx_count++;

	bp_relinquish(state->buffer_pool, (void**)&state->packet_in_progress);
}

void feed(tashtalk_rx_state_t* state, unsigned char byte) {

	// do we have a buffer?
	if (state->packet_in_progress == NULL) {
		state->packet_in_progress = bp_fetch(state->buffer_pool);
		crc_state_init(&state->crc);
	}
	
	// buggered if I know what else to do here other than go in circles
	while (state->packet_in_progress == NULL) {
		vTaskDelay(10 / portTICK_PERIOD_MS);
		state->packet_in_progress = bp_fetch(state->buffer_pool);
		crc_state_init(&state->crc);
		ESP_LOGE(TAG, "around and around and around we go");
		stats.tashtalk_waiting_for_buffer_count++;
	}
	
	// Are we in an escape state?
	if (state->in_escape) {
		switch(byte) {
			case 0xFF:
				// 0x00 0xFF is a literal 0x00 byte
				append(state->packet_in_progress, 0x00);
				crc_state_append(&state->crc, 0x00);
				break;
				
			case 0xFD:
				// 0x00 0xFD is a complete frame
				if (!crc_state_ok(&state->crc)) {
					ESP_LOGE(TAG, "/!\\ CRC fail: %d", state->crc);
					led_err_flash();
					stats.tashtalk_crc_fail_count++;
				}
				
				do_something_sensible_with_packet(state);
				state->packet_in_progress = NULL;
				break;
				
			case 0xFE:
				// 0x00 0xFD is a complete frame
				ESP_LOGI(TAG, "framing error of %d bytes", 
					state->packet_in_progress->length);
				led_err_flash();
				stats.tashtalk_framing_error_count++;
				
				bp_relinquish(state->buffer_pool, (void**) &state->packet_in_progress);
				state->packet_in_progress = NULL;
				break;

			case 0xFA:
				// 0x00 0xFD is a complete frame
				ESP_LOGI(TAG, "frame abort of %d bytes", 
					state->packet_in_progress->length);
				led_err_flash();
				stats.tashtalk_frame_abort_count++;
				
				bp_relinquish(state->buffer_pool, (void**) &state->packet_in_progress);
				state->packet_in_progress = NULL;
				break;			
		}
		
		state->in_escape = false;
	} else if (byte == 0x00) {
		state->in_escape = true;
	} else {
		append(state->packet_in_progress, byte);
		crc_state_append(&state->crc, byte);
	}
}

void feed_all(tashtalk_rx_state_t* state, unsigned char* buf, int count) {
	for (int i = 0; i < count; i++) {
		feed(state, buf[i]);
	}
}
