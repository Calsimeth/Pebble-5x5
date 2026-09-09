#include "setup_input.h"
uint8_t setup_repeat_steps(uint32_t held_ms) {
  if (held_ms < SETUP_INITIAL_HOLD_DELAY_MS) return 1;
  uint32_t repeats = 1 + (held_ms - SETUP_INITIAL_HOLD_DELAY_MS) / SETUP_REPEAT_INTERVAL_MS;
  return repeats > UINT8_MAX - 1 ? UINT8_MAX : (uint8_t)(repeats + 1);
}

int32_t setup_manual_up(int32_t current) { return current < 4000 ? current + 20 : 4000; }
int32_t setup_manual_down(int32_t current) { return current > 180 ? current - 20 : 180; }
