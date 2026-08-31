#ifndef SYNC_COMPLETION_H
#define SYNC_COMPLETION_H
#include <stdbool.h>
#include <stdint.h>
#include "sync.h"
typedef enum { WORKOUT_SYNC_CLEAR, WORKOUT_SYNC_NOT_SYNCED, WORKOUT_SYNC_REQUIRED } WorkoutSyncStatus;
bool sync_completion_capacity_blocked(uint8_t queue_count, bool pending_valid, uint8_t exercise_index, uint8_t set_index, uint8_t final_exercise, uint8_t final_set);
bool sync_completion_log_set(uint8_t *destination, uint8_t *selected_reps, uint8_t *blocked, uint8_t queue_count, bool pending_valid, uint8_t exercise_index, uint8_t set_index, uint8_t final_exercise, uint8_t final_set);
WorkoutSyncStatus sync_completion_status(uint8_t queue_count, bool completion_blocked);
SyncPushResult sync_completion_promote(SyncQueue *, SyncRecord *, bool pending_valid);
bool sync_completion_build_record(SyncRecord *, uint32_t id, uint8_t workout, int32_t timestamp, const uint16_t weights[3], const uint8_t reps[3][5], const uint8_t sets[3], uint8_t deload_mask);
#endif
