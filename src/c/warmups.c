#include "warmups.h"

static void add_if_valid(WarmupPlan *plan, Weight requested, Weight working,
                         const PlateInventory *inventory) {
  if (plan->count >= WARMUP_MAX_SETS || requested <= inventory->bar_weight) return;
  PlateLoad load = calculate_plate_load(requested, inventory);
  Weight actual = load.actual_total;
  if (actual <= inventory->bar_weight || actual >= working) return;
  if (plan->count && actual <= plan->sets[plan->count - 1].weight) return;
  for (uint8_t n = 0; n < plan->count; n++) if (plan->sets[n].weight == actual) return;
  plan->sets[plan->count++] = (WarmupSet){actual, 5};
}

WarmupPlan calculate_warmup_plan(Weight working, const PlateInventory *inventory) {
  WarmupPlan plan = {0};
  if (!inventory || !plate_inventory_valid(inventory) || !weight_valid(working) ||
      working <= inventory->bar_weight) return plan;

  plan.sets[0] = (WarmupSet){inventory->bar_weight, 5};
  plan.count = 1;
  PlateLoad work_load = calculate_plate_load(working, inventory);
  uint8_t forty_fives = work_load.counts[0];
  uint8_t intermediates = 0;
  if (forty_fives >= 2 && working >= WEIGHT_LB(225)) intermediates = 2;
  else if (forty_fives >= 1 && working >= WEIGHT_LB(135)) intermediates = 1;
  Weight distance = working - inventory->bar_weight;
  for (uint8_t n = 1; n <= intermediates; n++) {
    Weight target = inventory->bar_weight + (distance * n) / (intermediates + 1);
    add_if_valid(&plan, target, working, inventory);
  }
  return plan;
}

int warmup_plan_valid(const WarmupPlan *plan, Weight working, const PlateInventory *inventory) {
  if (!plan || !inventory || !plate_inventory_valid(inventory) || !weight_valid(working) ||
      plan->count > WARMUP_MAX_SETS) return 0;
  Weight previous = inventory->bar_weight;
  for (uint8_t n = 0; n < plan->count; n++) {
    WarmupSet set = plan->sets[n];
    if (set.repetitions != 5 || (n == 0 ? set.weight != previous : set.weight <= previous) ||
        set.weight >= working ||
        calculate_plate_load(set.weight, inventory).actual_total != set.weight) return 0;
    previous = set.weight;
  }
  return 1;
}
