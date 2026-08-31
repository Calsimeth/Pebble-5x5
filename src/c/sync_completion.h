#ifndef SYNC_COMPLETION_H
#define SYNC_COMPLETION_H
#include <stdbool.h>
#include <stdint.h>
#include "sync.h"
typedef enum { WORKOUT_SYNC_CLEAR, WORKOUT_SYNC_NOT_SYNCED, WORKOUT_SYNC_REQUIRED } WorkoutSyncStatus;
bool sync_completion_capacity_blocked(uint8_t queue_count, bool pending_valid, uint8_t exercise_index, uint8_t set_index, uint8_t final_exercise, uint8_t final_set);
WorkoutSyncStatus sync_completion_status(uint8_t queue_count, bool completion_blocked);
SyncPushResult sync_completion_promote(SyncQueue *, SyncRecord *, bool pending_valid);
#endif
