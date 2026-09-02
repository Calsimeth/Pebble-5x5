#include <assert.h>
#include "../src/c/setup_input.h"
int main(void) {
  assert(SETUP_INITIAL_HOLD_DELAY_MS == 700);
  assert(SETUP_REPEAT_INTERVAL_MS == 500);
  assert(SETUP_PEBBLE_REPEAT_INTERVAL_MS == 700);
  assert(setup_repeat_steps(0) == 1 && setup_repeat_steps(699) == 1);
  assert(setup_repeat_steps(700) == 2 && setup_repeat_steps(1200) == 3);
  assert(setup_repeat_steps(1201) == 3);
  assert(setup_repeat_steps(UINT32_MAX) == UINT8_MAX);
  return 0;
}
