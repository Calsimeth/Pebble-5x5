#include <assert.h>
#include "../src/c/setup_input.h"
int main(void) {
  assert(SETUP_INITIAL_HOLD_DELAY_MS == 800);
  assert(SETUP_REPEAT_INTERVAL_MS == 400);
  assert(setup_repeat_steps(0) == 1 && setup_repeat_steps(799) == 1);
  assert(setup_repeat_steps(800) == 2 && setup_repeat_steps(1199) == 2);
  assert(setup_repeat_steps(1200) == 3 && setup_repeat_steps(1599) == 3);
  assert(setup_repeat_steps(1600) == 4);
  assert(setup_repeat_steps(UINT32_MAX) == UINT8_MAX);
  assert(setup_manual_up(540) == 560 && setup_manual_up(560) == 580);
  assert(setup_manual_up(4000) == 4000);
  assert(setup_manual_down(560) == 540 && setup_manual_down(540) == 520);
  return 0;
}
