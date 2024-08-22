#pragma once

#include <driver/gpio.h>

#include "pins.h"

#include "led.h"

void led_init(void);
void led_boot_complete(void);