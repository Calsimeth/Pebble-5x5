#include "workout_completion.h"
#include "progression.h"
#include "deload.h"
#include "sync_completion.h"
#include <string.h>

static size_t weight_index(uint8_t workout, uint8_t exercise) { return exercise == 0 ? 0 : (workout == WORKOUT_A ? exercise : exercise + 1); }
static uint8_t set_count(uint8_t workout, uint8_t exercise) { (void)exercise; return workout == WORKOUT_B && exercise == 2 ? 1 : 5; }

CompletionResult workout_completion_attempt(PersistedState *s, uint8_t selected_reps, int32_t completion_time) {
  if (!s || !s->active || s->exercise_index != 2 || s->set_index != set_count(s->active_workout, 2) - 1 || !repetition_valid(selected_reps)) return COMPLETION_ERROR;
  s->selected_reps = selected_reps;
  if (s->outbox.count >= SYNC_QUEUE_CAPACITY && s->pending_valid) { s->completion_blocked = 1; return COMPLETION_BLOCKED; }
  PersistedState before = *s;
  s->work_reps[2][s->set_index] = selected_reps;
  SyncRecord record = {0}; uint32_t id = 0;
  if (!sync_allocate_id(&s->next_record_id, &s->outbox, &s->pending_record, s->pending_valid, &id)) { *s = before; s->completion_blocked = 1; s->selected_reps = selected_reps; return COMPLETION_ERROR; }
  uint16_t weights[3] = {(uint16_t)s->active_weights[0], (uint16_t)s->active_weights[1], (uint16_t)s->active_weights[2]};
  uint8_t sets[3] = {5,5,set_count(s->active_workout,2)};
  if (!sync_completion_build_record(&record,id,s->active_workout,completion_time,weights,(const uint8_t (*)[5])s->work_reps,sets,0)) { *s = before; s->completion_blocked = 1; s->selected_reps = selected_reps; return COMPLETION_ERROR; }
  SyncPushResult result = sync_queue_push_result(&s->outbox,&record);
  if (result == SYNC_PUSH_FULL) { s->pending_record=record; s->pending_valid=1; }
  else if (result != SYNC_PUSH_ADDED && result != SYNC_PUSH_IDENTICAL) { *s=before; s->completion_blocked=1; s->selected_reps=selected_reps; return COMPLETION_ERROR; }
  size_t wi=weight_index(s->active_workout,2); PlateInventory inv=plate_inventory_from_counts(s->inventory_counts);
  bool success=exercise_succeeded(s->work_reps[2],sets[2]); s->weights[wi]=success?successful_target(s->active_weights[2],&inv):failed_target(s->active_weights[2]); s->failure_streaks[wi]=failure_streak_after(success,s->failure_streaks[wi]);
  if(!success&&deload_after_failure(s->failure_streaks[wi])){s->deload_pending[wi]=1;s->failure_reviewed[wi]=0;s->plateau_reviewed[wi]=0;}
  s->active=0; s->last_completed=completion_time; s->next_workout=s->active_workout==WORKOUT_A?WORKOUT_B:WORKOUT_A; s->set_index=0; s->exercise_index=0; s->completion_blocked=0; s->selected_reps=5; return COMPLETION_OK;
}

SyncPushResult workout_completion_handle_ack(PersistedState *s, uint32_t acknowledged_id) {
  if (!s) return SYNC_PUSH_CONFLICT;
  return sync_completion_ack_promote(&s->outbox,acknowledged_id,&s->pending_record,&s->pending_valid,&s->completion_blocked);
}
