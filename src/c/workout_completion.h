#ifndef WORKOUT_COMPLETION_H
#define WORKOUT_COMPLETION_H
#include <stdint.h>
#include <stddef.h>
#include "plates.h"
#include "warmups.h"
#include "sync.h"

#define WORKOUT_STORAGE_SCHEMA 9
typedef enum { WORKOUT_A = 0, WORKOUT_B = 1 } WorkoutType;

typedef struct {
  uint8_t schema_version, next_workout, active, active_workout, exercise_index, set_index;
  uint8_t rest_active; int32_t rest_start, rest_end; uint8_t halfway_alerted;
  Weight weights[5]; Weight active_weights[3]; uint8_t work_reps[3][5], failure_streaks[5];
  PlateCounts inventory_counts; uint8_t warmup_active, warmup_index; WarmupPlan warmup_plan;
  int32_t last_completed; uint8_t deload_pending[5], gap_reviewed[5], failure_reviewed[5], plateau_reviewed[5], accepted_deloads[5];
  SyncQueue outbox; uint32_t next_record_id; SyncRecord pending_record; uint8_t pending_valid, completion_blocked, selected_reps;
} PersistedState;

typedef enum { COMPLETION_OK, COMPLETION_BLOCKED, COMPLETION_ERROR } CompletionResult;
CompletionResult workout_completion_attempt(PersistedState *, uint8_t selected_reps, int32_t completion_time);
SyncPushResult workout_completion_handle_ack(PersistedState *, uint32_t acknowledged_id);
size_t workout_weight_index(uint8_t workout, uint8_t exercise);
bool workout_state_valid(const PersistedState *state);
#endif
