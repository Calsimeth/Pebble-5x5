#ifndef MIGRATION_H
#define MIGRATION_H
#include "workout_completion.h"
typedef struct {
  uint8_t schema_version, next_workout, active, active_workout, exercise_index, set_index, rest_active;
  int32_t rest_start, rest_end; uint8_t halfway_alerted;
  Weight weights[5], active_weights[3]; uint8_t work_reps[3][5], failure_streaks[5];
  PlateCounts inventory_counts; uint8_t warmup_active, warmup_index; WarmupPlan warmup_plan;
  int32_t last_completed; uint8_t deload_pending[5], gap_reviewed[5], failure_reviewed[5], plateau_reviewed[5], accepted_deloads[5];
  SyncQueue outbox; uint32_t next_record_id; SyncRecord pending_record; uint8_t pending_valid, completion_blocked, selected_reps;
} PersistedStateV9;
int migrate_v9_to_v10(const PersistedStateV9 *old, PersistedState *out, int32_t now);
#endif
