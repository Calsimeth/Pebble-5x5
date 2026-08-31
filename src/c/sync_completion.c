#include "sync_completion.h"
bool sync_completion_capacity_blocked(uint8_t queue_count, bool pending_valid, uint8_t exercise_index, uint8_t set_index, uint8_t final_exercise, uint8_t final_set) { return exercise_index==final_exercise && set_index==final_set && queue_count>=SYNC_QUEUE_CAPACITY && pending_valid; }
bool sync_completion_log_set(uint8_t *destination, uint8_t *selected_reps, uint8_t *blocked, uint8_t queue_count, bool pending_valid, uint8_t exercise_index, uint8_t set_index, uint8_t final_exercise, uint8_t final_set) { if(!destination||!selected_reps||!blocked)return false; if(sync_completion_capacity_blocked(queue_count,pending_valid,exercise_index,set_index,final_exercise,final_set)){*blocked=1;return false;} destination[0]=*selected_reps;*selected_reps=5;*blocked=0;return true; }
WorkoutSyncStatus sync_completion_status(uint8_t queue_count, bool blocked) { if (blocked || queue_count>=SYNC_QUEUE_CAPACITY) return WORKOUT_SYNC_REQUIRED; return queue_count ? WORKOUT_SYNC_NOT_SYNCED : WORKOUT_SYNC_CLEAR; }
SyncPushResult sync_completion_promote(SyncQueue *q, SyncRecord *pending, bool valid) { return sync_pending_promote(q,pending,valid); }
SyncPushResult sync_completion_ack_promote(SyncQueue *q, uint32_t id, SyncRecord *pending, uint8_t *pending_valid, uint8_t *blocked) { if(!q||!pending_valid||!blocked||!sync_queue_ack(q,id)) return SYNC_PUSH_CONFLICT; SyncPushResult result=sync_pending_promote(q,pending,*pending_valid); if(result==SYNC_PUSH_ADDED||result==SYNC_PUSH_IDENTICAL){*pending_valid=0;*blocked=0;} return result; }
bool sync_completion_build_record(SyncRecord *r, uint32_t id, uint8_t workout, int32_t timestamp, const uint16_t weights[3], const uint8_t reps[3][5], const uint8_t sets[3], uint8_t deload_mask) {
  if (!r || !id || workout > 1 || !weights || !reps || !sets) return false;
  const uint8_t ids[2][3]={{0,1,2},{0,3,4}}; *r=(SyncRecord){.id=id,.schema_version=SYNC_RECORD_VERSION,.workout=workout,.completed_at=timestamp,.complete=1,.rep_count=(uint8_t)(workout?11:15),.deload_mask=deload_mask};
  uint8_t offset=0; for(uint8_t e=0;e<3;e++){if(!sets[e]||sets[e]>5)return false;r->exercise_ids[e]=ids[workout][e];r->weights[e]=weights[e];for(uint8_t s=0;s<sets[e];s++)r->reps[offset++]=reps[e][s];}
  return offset==r->rep_count && sync_record_valid(r);
}
