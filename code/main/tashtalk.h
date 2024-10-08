#pragma once

#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "buffer_pool.h"
#include "packet_types.h"
#include "crc.h"

/* This implements the TashTalk *protocol*.  UART wrangling is handled in
   uart.c instead. */
   
/* a tashtalk_rx_state_t value represents a state machine for getting LLAP 
   packets out of the byte stream from TashTalk. */
typedef struct {
	buffer_pool_t* buffer_pool; // the pool to get packet buffers from
	llap_packet* packet_in_progress; // the buffer that holds the in-flight packet
	bool in_escape;
	crc_state_t crc;
} tashtalk_rx_state_t;

tashtalk_rx_state_t* new_tashtalk_rx_state(buffer_pool_t* buffer_pool);
void feed(tashtalk_rx_state_t* state, unsigned char byte);
void feed_all(tashtalk_rx_state_t* state, unsigned char* buf, int count);

