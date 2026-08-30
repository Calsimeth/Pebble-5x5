#ifndef WARMUPS_H
#define WARMUPS_H

#include <stdint.h>
#include "plates.h"

#define WARMUP_MAX_SETS 3

typedef struct {
  Weight weight;
  uint8_t repetitions;
} WarmupSet;

typedef struct {
  uint8_t count;
  WarmupSet sets[WARMUP_MAX_SETS];
} WarmupPlan;

WarmupPlan calculate_warmup_plan(Weight working_weight, const PlateInventory *inventory);
int warmup_plan_valid(const WarmupPlan *plan, Weight working_weight,
                     const PlateInventory *inventory);

#endif
