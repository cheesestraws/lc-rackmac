#include <strings.h>

#include <esp_log.h>

#include "stats.h"

#include "buffer_pool.h"

static const char* TAG = "bufferpool";

/* the buffer_pool_t type is defined in the header file */

void bp_panic() {
	ESP_LOGE(TAG, "PANIC AT THE DISCO!  PANIC IN THE BUFFER POOL!");
	stats.bp_freakout_count++;
}


buffer_pool_t* new_buffer_pool(int buffer_count, int buffer_size) {
	QueueHandle_t queue;
	
	// Note that the items we store in the queue are *pointers* to the buffers
	// not the buffers themselves.  So 
	queue = xQueueCreate(buffer_count, sizeof(char*));
	
	if (queue==NULL) {
		goto handle_err;
	}
	
	// Now create the buffers
	for (int i = 0; i < buffer_count; i++) {
		char* buffer;
		buffer = malloc(buffer_size);
		if (buffer == NULL) {
			goto handle_err;
		}
		
		bzero(buffer, buffer_size);
		
		BaseType_t result = xQueueSendToBack(queue, &buffer, 0);
		if (result == errQUEUE_FULL) {
			ESP_LOGE(TAG, "BUG: in new_buffer_pool, why is the queue full?");
			stats.bp_freakout_count++;
			free(buffer);
		}
	}
	
	// Now construct the struct to return
	buffer_pool_t* ret = malloc(sizeof(buffer_pool_t));
	if (ret == NULL) {
		goto handle_err;
	}
	
	ret->buffer_count = buffer_count;
	ret->buffer_size = buffer_size;
	ret->queue = queue;
	return ret;
	
handle_err:
	/* TODO: error handling */
	bp_panic();
	return NULL;
}

void* bp_fetch(buffer_pool_t* bp) {
	if (bp == NULL || bp->queue == NULL) {
		ESP_LOGE(TAG, "BUG: bp_fetch called on null pool or pool with null queue");
		stats.bp_freakout_count++;
		return NULL;
	}
	
	void* buffer;
	BaseType_t ret = xQueueReceive(bp->queue, &buffer, 0);
	
	if (ret == pdTRUE) {
		return buffer;
	} else {
		ESP_LOGW(TAG, "BUG: bp_fetch underflowed buffer pool");
		stats.bp_freakout_count++;
		return NULL;
	}
}

void bp_relinquish(buffer_pool_t* bp, void** buff) {
	if (buff == NULL) {
		ESP_LOGE(TAG, "BUG: bp_relinquish called on null pointer");
		stats.bp_freakout_count++;
	}
	if (*buff == NULL) {
		ESP_LOGE(TAG, "BUG: bp_relinquish called on pointer to null buffer");
		stats.bp_freakout_count++;
	}

	// Try to push the buffer to the queue	
	BaseType_t result = xQueueSendToBack(bp->queue, buff, 0);	
	if (result == errQUEUE_FULL) {
		ESP_LOGE(TAG, "BUG: bp_relinquish queue overflow; have you relinquished a bad pointer?");
		stats.bp_freakout_count++;
		return;
	}
		
	bzero(*buff, bp->buffer_size);
	
	*buff = NULL;	
}

int bp_buffersize(buffer_pool_t* bp) {
	return bp->buffer_size;
}
