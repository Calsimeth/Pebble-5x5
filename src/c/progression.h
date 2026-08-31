#ifndef PROGRESSION_H
#define PROGRESSION_H

#include <stdbool.h>
#include <stdint.h>
#include "plates.h"

#define PROGRESSION_MAX_SETS 5

bool repetition_valid(uint8_t repetitions);
bool exercise_succeeded(const uint8_t *repetitions, uint8_t set_count);
Weight successful_target(Weight current, const PlateInventory *inventory);
Weight failed_target(Weight current);
uint8_t failure_streak_after(bool success, uint8_t previous);

#endif
