#include "sync_completion.h"
bool sync_completion_capacity_blocked(uint8_t queue_count, bool pending_valid, uint8_t exercise_index, uint8_t set_index, uint8_t final_exercise, uint8_t final_set) { return exercise_index==final_exercise && set_index==final_set && queue_count>=SYNC_QUEUE_CAPACITY && pending_valid; }
WorkoutSyncStatus sync_completion_status(uint8_t queue_count, bool blocked) { if (blocked || queue_count>=SYNC_QUEUE_CAPACITY) return WORKOUT_SYNC_REQUIRED; return queue_count ? WORKOUT_SYNC_NOT_SYNCED : WORKOUT_SYNC_CLEAR; }
SyncPushResult sync_completion_promote(SyncQueue *q, SyncRecord *pending, bool valid) { return sync_pending_promote(q,pending,valid); }
