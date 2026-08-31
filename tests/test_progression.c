#include <assert.h>
#include "../src/c/progression.h"

int main(void) {
  PlateInventory i = plate_inventory_default();
  uint8_t good[5] = {5,5,5,5,5}, bad[5] = {5,5,0,5,5};
  uint8_t deadlift[1] = {5}, incomplete[5] = {5,5,0,0,0};
  assert(repetition_valid(0) && repetition_valid(5) && !repetition_valid(6));
  assert(exercise_succeeded(good, 5) && !exercise_succeeded(bad, 5));
  assert(exercise_succeeded(deadlift, 1) && !exercise_succeeded(incomplete, 5));
  assert(successful_target(WEIGHT_LB(45), &i) >= WEIGHT_LB(50));
  assert(successful_target(WEIGHT_LB(1000), &i) == WEIGHT_LB(1000));
  i.plates[0].count_per_side = 0; i.plates[2].count_per_side = 0; i.plates[4].count_per_side = 0;
  i.plates[5].count_per_side = 0; i.plates[6].count_per_side = 0;
  assert(successful_target(WEIGHT_LB(1000), &i) == WEIGHT_LB(1000));
  assert(failed_target(WEIGHT_LB(95)) == WEIGHT_LB(95));
  assert(failure_streak_after(false, 0) == 1 && failure_streak_after(true, 9) == 0);
  assert(failure_streak_after(false, 255) == 255);
  return 0;
}
