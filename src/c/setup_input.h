#ifndef SETUP_INPUT_H
#define SETUP_INPUT_H
#include <stdint.h>
#define SETUP_INITIAL_HOLD_DELAY_MS 700
#define SETUP_REPEAT_INTERVAL_MS 500
#define SETUP_PEBBLE_REPEAT_INTERVAL_MS SETUP_INITIAL_HOLD_DELAY_MS
uint8_t setup_repeat_steps(uint32_t held_ms);
#endif
