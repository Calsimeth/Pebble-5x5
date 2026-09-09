#include <assert.h>
#include "../src/c/setup_input.h"
int main(void) {
  assert(SETUP_INITIAL_HOLD_DELAY_MS == 1200);
  assert(SETUP_REPEAT_INTERVAL_MS == 500);
  assert(setup_repeat_steps(0) == 1 && setup_repeat_steps(1199) == 1);
  assert(setup_repeat_steps(1200) == 2 && setup_repeat_steps(1699) == 2);
  assert(setup_repeat_steps(1700) == 3 && setup_repeat_steps(2199) == 3);
  assert(setup_repeat_steps(2200) == 4);
  assert(setup_repeat_steps(UINT32_MAX) == UINT8_MAX);
  assert(setup_manual_up(540) == 560 && setup_manual_up(560) == 580);
  assert(setup_manual_up(4000) == 4000);
  assert(setup_manual_down(560) == 540 && setup_manual_down(540) == 520);
  return 0;
}
