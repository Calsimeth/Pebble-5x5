#ifndef SETUP_INPUT_H
#define SETUP_INPUT_H
#include <stdint.h>
#define SETUP_INITIAL_HOLD_DELAY_MS 700
#define SETUP_REPEAT_INTERVAL_MS 400
uint8_t setup_repeat_steps(uint32_t held_ms);
int32_t setup_manual_up(int32_t current);
int32_t setup_manual_down(int32_t current);
#endif
