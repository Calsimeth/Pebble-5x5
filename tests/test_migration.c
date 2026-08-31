#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../src/c/plates.h"
#include "../src/c/warmups.h"

/* Byte-for-byte schema 7 fixture reconstructed from commit 40a3547. */
typedef struct {
  uint8_t schema_version, next_workout, active, active_workout, exercise_index, set_index, rest_active;
  int32_t rest_start, rest_end; uint8_t halfway_alerted;
  Weight weights[5], active_weights[3]; uint8_t work_reps[3][5], failure_streaks[5];
  PlateCounts inventory_counts; uint8_t warmup_active, warmup_index; WarmupPlan warmup_plan;
  int32_t last_completed; uint8_t deload_pending[5], gap_reviewed[5], accepted_deloads[5];
} Schema7;

int main(void) {
  assert(sizeof(Schema7) == 132);
  assert(offsetof(Schema7, weights) < offsetof(Schema7, active_weights));
  assert(offsetof(Schema7, work_reps) < offsetof(Schema7, failure_streaks));
  assert(offsetof(Schema7, last_completed) < offsetof(Schema7, deload_pending));
  assert(offsetof(Schema7, deload_pending) < offsetof(Schema7, gap_reviewed));
  assert(offsetof(Schema7, gap_reviewed) < offsetof(Schema7, accepted_deloads));
  Schema7 old = {0}; old.schema_version = 7; old.active = 1; old.active_workout = 1;
  old.exercise_index = 2; old.set_index = 1; old.rest_active = 1; old.rest_start = 100; old.rest_end = 280;
  old.weights[4] = WEIGHT_LB(205); old.active_weights[2] = WEIGHT_LB(205);
  old.work_reps[1][0] = 5; old.failure_streaks[4] = 3; old.last_completed = 50;
  old.deload_pending[4] = 1; old.gap_reviewed[2] = 1; old.accepted_deloads[4] = 2;
  Schema7 migrated = {0};
  migrated.schema_version = 8; migrated.next_workout = old.next_workout; migrated.active = old.active;
  migrated.active_workout = old.active_workout; migrated.exercise_index = old.exercise_index; migrated.set_index = old.set_index;
  migrated.rest_active = old.rest_active; migrated.rest_start = old.rest_start; migrated.rest_end = old.rest_end;
  memcpy(migrated.weights, old.weights, sizeof old.weights); memcpy(migrated.active_weights, old.active_weights, sizeof old.active_weights);
  memcpy(migrated.work_reps, old.work_reps, sizeof old.work_reps); memcpy(migrated.failure_streaks, old.failure_streaks, sizeof old.failure_streaks);
  memcpy(migrated.deload_pending, old.deload_pending, sizeof old.deload_pending); memcpy(migrated.gap_reviewed, old.gap_reviewed, sizeof old.gap_reviewed);
  memcpy(migrated.accepted_deloads, old.accepted_deloads, sizeof old.accepted_deloads);
  assert(migrated.active && migrated.rest_end == 280 && migrated.weights[4] == WEIGHT_LB(205));
  assert(migrated.failure_streaks[4] == 3 && migrated.deload_pending[4] == 1 && migrated.gap_reviewed[2] == 1);
  return 0;
}
