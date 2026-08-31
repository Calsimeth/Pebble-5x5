#include "progression.h"

bool repetition_valid(uint8_t repetitions) { return repetitions <= 5; }

bool exercise_succeeded(const uint8_t *repetitions, uint8_t set_count) {
  if (!repetitions || set_count == 0 || set_count > PROGRESSION_MAX_SETS) return false;
  for (uint8_t n = 0; n < set_count; n++) if (repetitions[n] != 5) return false;
  return true;
}

Weight successful_target(Weight current, const PlateInventory *inventory) {
  if (!inventory || !weight_valid(current)) return current;
  Weight target = current >= WEIGHT_LB(1000) - WEIGHT_LB(5) ? WEIGHT_LB(1000) : current + WEIGHT_LB(5);
  Weight next = next_achievable_total(current, inventory);
  if (next < target) next = normalize_weight_down(target, inventory);
  if (next < target) return current;
  return next > WEIGHT_LB(1000) ? current : next;
}

Weight failed_target(Weight current) { return current; }

uint8_t failure_streak_after(bool success, uint8_t previous) {
  if (success) return 0;
  return previous == UINT8_MAX ? UINT8_MAX : (uint8_t)(previous + 1);
}

uint8_t failure_streak_after_manual_weight_change(uint8_t previous, Weight old_weight, Weight new_weight) {
  return old_weight == new_weight ? previous : 0;
}
