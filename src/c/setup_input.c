#include "setup_input.h"
uint8_t setup_repeat_steps(uint32_t held_ms) {
  if (held_ms < SETUP_REPEAT_INTERVAL_MS) return 1;
  uint32_t repeats = held_ms / SETUP_REPEAT_INTERVAL_MS;
  return repeats > UINT8_MAX - 1 ? UINT8_MAX : (uint8_t)(repeats + 1);
}
