#ifndef DELOAD_H
#define DELOAD_H

#include <stdbool.h>
#include <stdint.h>
#include "plates.h"

#define DELOAD_FAILURE_THRESHOLD 3
#define DELOAD_GAP_DAYS 7
#define DELOAD_PERCENT_NUMERATOR 90
#define DELOAD_PERCENT_DENOMINATOR 100
#define DELOAD_PLATEAU_THRESHOLD 2

typedef struct {
  Weight current_weight;
  Weight proposed_weight;
  uint8_t failure_streak;
  uint8_t accepted_deloads;
  bool pending;
  bool gap_reviewed;
} DeloadState;

bool deload_after_failure(uint8_t failure_streak);
Weight deload_weight(Weight current, const PlateInventory *inventory);
bool deload_gap_due(int64_t now, int64_t last_completed);
bool deload_should_prompt(const DeloadState *state, bool gap_due);
void deload_accept(DeloadState *state);
void deload_decline(DeloadState *state);
void deload_review_gap(DeloadState *state);
bool plateau_advisory_due(const DeloadState *state);

#endif
