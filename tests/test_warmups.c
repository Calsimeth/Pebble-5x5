#include <assert.h>
#include <string.h>
#include "../src/c/warmups.h"

static void assert_plan(Weight work, uint8_t count, const PlateInventory *i) {
  WarmupPlan p = calculate_warmup_plan(work, i);
  assert(p.count == count && warmup_plan_valid(&p, work, i));
  for (uint8_t n = 0; n < p.count; n++) assert(p.sets[n].repetitions == 5);
}

int main(void) {
  PlateInventory i = plate_inventory_default();
  assert_plan(WEIGHT_LB(45), 0, &i);
  assert_plan(WEIGHT_LB(65), 1, &i);
  assert_plan(WEIGHT_LB(95), 1, &i);
  assert_plan(WEIGHT_LB(135), 2, &i);
  assert_plan(WEIGHT_LB(210), 2, &i);
  assert_plan(WEIGHT_LB(250), 3, &i);
  PlateCounts counts; plate_inventory_counts(&i, counts); counts[1] = 2; i = plate_inventory_from_counts(counts);
  WarmupPlan a = calculate_warmup_plan(WEIGHT_LB(210), &i), b = calculate_warmup_plan(WEIGHT_LB(210), &i);
  assert(a.count == b.count && memcmp(&a, &b, sizeof a) == 0 && warmup_plan_valid(&a, WEIGHT_LB(210), &i));
  counts[0] = 0; i = plate_inventory_from_counts(counts); assert_plan(WEIGHT_LB(250), 1, &i);
  assert(calculate_warmup_plan(WEIGHT_LB(44), &i).count == 0);
  assert(calculate_warmup_plan(WEIGHT_LB(250), NULL).count == 0);
  return 0;
}
