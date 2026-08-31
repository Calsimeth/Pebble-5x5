#include <assert.h>
#include <stdint.h>
#include "../src/c/deload.h"

int main(void) {
  PlateInventory inventory = plate_inventory_default();
  DeloadState state = { .current_weight = WEIGHT_LB(225) };

  assert(deload_after_failure(3));
  assert(!deload_after_failure(2));
  assert(deload_weight(WEIGHT_LB(225), &inventory) == WEIGHT_LB(200));
  assert(deload_weight(WEIGHT_LB(48), &inventory) == WEIGHT_LB(45));
  assert(deload_weight(WEIGHT_LB(45), &inventory) == WEIGHT_LB(45));

  assert(!deload_gap_due(7 * 86400, 0));
  assert(deload_gap_due(7 * 86400 + 1, 0));
  assert(!deload_gap_due(0, 1));

  state.failure_streak = 1; assert(!deload_should_prompt(&state, false));
  state.failure_streak = 2; assert(!deload_should_prompt(&state, false));
  state.failure_streak = 3;
  state.pending = true;
  assert(deload_should_prompt(&state, false));
  state.pending = false;
  assert(deload_should_prompt(&state, false));
  deload_decline(&state);
  assert(!deload_should_prompt(&state, false));
  state.failure_streak = 1;
  assert(deload_should_prompt(&state, true));
  deload_review_gap(&state);
  assert(!deload_should_prompt(&state, true));
  state.gap_reviewed = false; state.failure_reviewed = true;
  assert(deload_should_prompt(&state, true));

  state.accepted_deloads = 1;
  state.failure_streak = 3;
  deload_accept(&state);
  assert(state.accepted_deloads == 2 && state.failure_streak == 0 && !state.pending);
  state.failure_streak = 3;
  assert(plateau_advisory_due(&state));
  uint8_t order[3];
  assert(deload_workout_order(false, order) == 3 && order[0] == 0 && order[1] == 1 && order[2] == 2);
  assert(deload_workout_order(true, order) == 3 && order[0] == 0 && order[1] == 3 && order[2] == 4);
  assert(order[1] != 1 && order[2] == 4);
  plateau_dismiss(&state);
  assert(!deload_should_prompt(&state, false));
  deload_accept(&state);
  assert(state.current_weight == WEIGHT_LB(225));
  return 0;
}
