#include "deload.h"

bool deload_after_failure(uint8_t failure_streak) {
  return failure_streak >= DELOAD_FAILURE_THRESHOLD;
}

Weight deload_weight(Weight current, const PlateInventory *inventory) {
  if (!inventory || !weight_valid(current)) return current;
  Weight reduced = (Weight)(((int64_t)current * DELOAD_PERCENT_NUMERATOR) /
                            DELOAD_PERCENT_DENOMINATOR);
  if (reduced < inventory->bar_weight) reduced = inventory->bar_weight;
  return normalize_weight_down(reduced, inventory) < inventory->bar_weight
             ? inventory->bar_weight
             : normalize_weight_down(reduced, inventory);
}

bool deload_gap_due(int64_t now, int64_t last_completed) {
  if (now < 0 || last_completed < 0 || now < last_completed) return false;
  return now - last_completed > (int64_t)DELOAD_GAP_DAYS * 24 * 60 * 60;
}

bool deload_should_prompt(const DeloadState *state, bool gap_due) {
  if (!state) return false;
  return (deload_after_failure(state->failure_streak) && !state->failure_reviewed) ||
         (gap_due && !state->gap_reviewed);
}

void deload_accept(DeloadState *state) {
  if (!state) return;
  state->pending = false;
  state->failure_streak = 0;
  if (state->accepted_deloads != UINT8_MAX) state->accepted_deloads++;
}

void deload_decline(DeloadState *state) {
  if (state) { state->pending = false; state->failure_reviewed = true; }
}

void deload_review_gap(DeloadState *state) {
  if (state) state->gap_reviewed = true;
}

void deload_review_failure(DeloadState *state) {
  if (state) state->failure_reviewed = true;
}

bool plateau_advisory_due(const DeloadState *state) {
  return state && state->accepted_deloads >= DELOAD_PLATEAU_THRESHOLD &&
         state->failure_streak >= DELOAD_FAILURE_THRESHOLD;
}

uint8_t deload_workout_order(bool workout_b, uint8_t out[3]) {
  if (!out) return 0;
  out[0] = 0; out[1] = workout_b ? 3 : 1; out[2] = workout_b ? 4 : 2;
  return 3;
}

void plateau_dismiss(DeloadState *state) {
  if (state) state->failure_reviewed = true;
}
