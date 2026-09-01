#include <assert.h>
#include "../src/c/setup_input.h"
int main(void) {
  assert(SETUP_REPEAT_INTERVAL_MS == 400);
  assert(setup_repeat_steps(0) == 1 && setup_repeat_steps(399) == 1);
  assert(setup_repeat_steps(400) == 2 && setup_repeat_steps(800) == 3);
  assert(setup_repeat_steps(801) == 3);
  assert(setup_repeat_steps(UINT32_MAX) == UINT8_MAX);
  return 0;
}
