#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/sdm.h"
#include "driver/gptimer.h"

#include "pins.h"
#include "beep.h"

#define MHZ                             (1000000)
#define CONST_PI                        (3.1416f)           // Constant of PI, used for calculating the sine wave
#define EXAMPLE_SIGMA_DELTA_GPIO_NUM    PIN_SNDOUT                 // Select GPIO_NUM_0 as the sigma-delta output pin
#define EXAMPLE_OVER_SAMPLE_RATE        (10 * MHZ)          // 10 MHz over sample rate
#define EXAMPLE_TIMER_RESOLUTION        (1  * MHZ)          // 1 MHz timer counting resolution
#define EXAMPLE_CALLBACK_INTERVAL_US    (100)               // 100 us interval of each timer callback
#define EXAMPLE_ALARM_COUNT             (EXAMPLE_CALLBACK_INTERVAL_US * (EXAMPLE_TIMER_RESOLUTION / MHZ))
#define EXAMPLE_SINE_WAVE_FREQ_HZ       (880)               // 100 Hz sine wave, adjust this value to decide the sine wave frequency
#define EXAMPLE_SINE_WAVE_AMPLITUDE     (64.0f)            // 1 ~ 127, adjust this value to decide the sine wave amplitude
#define EXAMPLE_SINE_WAVE_POINT_NUM     (MHZ / (EXAMPLE_CALLBACK_INTERVAL_US * EXAMPLE_SINE_WAVE_FREQ_HZ))

static int8_t sine_wave[EXAMPLE_SINE_WAVE_POINT_NUM];   // Sine wave data buffer
sdm_channel_handle_t sdm_chan;
gptimer_handle_t timer_handle;

static bool IRAM_ATTR example_timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    static uint32_t cnt = 0;
    sdm_channel_handle_t sdm_chan = (sdm_channel_handle_t)user_ctx;
    /* Set the pulse density */
    sdm_channel_set_pulse_density(sdm_chan, sine_wave[cnt++]);
    /* Loop the sine wave data buffer */
    if (cnt >= EXAMPLE_SINE_WAVE_POINT_NUM) {
        cnt = 0;
    }
    return false;
}

void sound_init(void) {
	/* Assign sine wave data */
	for (int i = 0; i < EXAMPLE_SINE_WAVE_POINT_NUM; i++) {
		sine_wave[i] = (int8_t)((sin(2 * (float)i * CONST_PI / EXAMPLE_SINE_WAVE_POINT_NUM)) * EXAMPLE_SINE_WAVE_AMPLITUDE);
	}

	// initialise SDM
	sdm_config_t config = {
		.clk_src = SDM_CLK_SRC_DEFAULT,
		.gpio_num = EXAMPLE_SIGMA_DELTA_GPIO_NUM,
		.sample_rate_hz = EXAMPLE_OVER_SAMPLE_RATE,
	};
	ESP_ERROR_CHECK(sdm_new_channel(&config, &sdm_chan));
	ESP_ERROR_CHECK(sdm_channel_enable(sdm_chan));
    sdm_channel_set_pulse_density(sdm_chan, -128);

	/* Allocate GPTimer handle */
	gptimer_config_t timer_cfg = {
		.clk_src = GPTIMER_CLK_SRC_DEFAULT,
		.direction = GPTIMER_COUNT_UP,
		.resolution_hz = EXAMPLE_TIMER_RESOLUTION,
	};
	ESP_ERROR_CHECK(gptimer_new_timer(&timer_cfg, &timer_handle));

	/* Set the timer alarm configuration */
	gptimer_alarm_config_t alarm_cfg = {
		.alarm_count = EXAMPLE_ALARM_COUNT,
		.reload_count = 0,
		.flags.auto_reload_on_alarm = true,
	};
	ESP_ERROR_CHECK(gptimer_set_alarm_action(timer_handle, &alarm_cfg));

	/* Register the alarm callback */
	gptimer_event_callbacks_t cbs = {
		.on_alarm = example_timer_callback,
	};
	ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer_handle, &cbs, sdm_chan));

	/* Clear the timer raw count value, make sure it'll count from 0 */
	ESP_ERROR_CHECK(gptimer_set_raw_count(timer_handle, 0));
}

void start_beep(void) {
	ESP_ERROR_CHECK(gptimer_enable(timer_handle));
	ESP_ERROR_CHECK(gptimer_start(timer_handle));
}

void stop_beep(void) {
	ESP_ERROR_CHECK(gptimer_stop(timer_handle));
	ESP_ERROR_CHECK(gptimer_disable(timer_handle));
    sdm_channel_set_pulse_density(sdm_chan, -128);
}

void bootbeep(void) {
	start_beep();
	vTaskDelay(250 / portTICK_PERIOD_MS);
	stop_beep();
}