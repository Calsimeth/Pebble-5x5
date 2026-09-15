#include "migration.h"
#include <string.h>
#include "rest_state.h"
int migrate_v9_to_v10(const PersistedStateV9 *old, PersistedState *out, int32_t now) {
  if (!old || !out || old->schema_version != 9 || old->next_workout > WORKOUT_B || old->active_workout > WORKOUT_B || old->active > 1 || old->rest_active > 1 || old->halfway_alerted > 1 || old->completion_blocked > 1 || old->selected_reps > 5) return 0;
  if (old->active && (old->exercise_index >= 3 || old->set_index >= (old->active_workout==WORKOUT_B && old->exercise_index==2 ? 1 : 5))) return 0;
  if (old->rest_active && (!old->active || old->rest_start <= 0 || now < old->rest_start)) return 0;
  memset(out,0,sizeof *out);
  out->schema_version=WORKOUT_STORAGE_SCHEMA; out->next_workout=old->next_workout; out->active=old->active; out->active_workout=old->active_workout; out->exercise_index=old->exercise_index; out->set_index=old->set_index;
  out->rest_active=old->rest_active; out->rest_start=old->rest_start; out->rest_end=old->rest_end; out->halfway_alerted=old->halfway_alerted;
  memcpy(out->weights,old->weights,sizeof out->weights); memcpy(out->active_weights,old->active_weights,sizeof out->active_weights); memcpy(out->work_reps,old->work_reps,sizeof out->work_reps); memcpy(out->failure_streaks,old->failure_streaks,sizeof out->failure_streaks); memcpy(out->inventory_counts,old->inventory_counts,sizeof out->inventory_counts); out->warmup_active=old->warmup_active; out->warmup_index=old->warmup_index; out->warmup_plan=old->warmup_plan; out->last_completed=old->last_completed;
  memcpy(out->deload_pending,old->deload_pending,sizeof out->deload_pending); memcpy(out->gap_reviewed,old->gap_reviewed,sizeof out->gap_reviewed); memcpy(out->failure_reviewed,old->failure_reviewed,sizeof out->failure_reviewed); memcpy(out->plateau_reviewed,old->plateau_reviewed,sizeof out->plateau_reviewed); memcpy(out->accepted_deloads,old->accepted_deloads,sizeof out->accepted_deloads); out->outbox=old->outbox; out->next_record_id=old->next_record_id; out->pending_record=old->pending_record; out->pending_valid=old->pending_valid; out->completion_blocked=old->completion_blocked; out->selected_reps=old->selected_reps;
  out->rest_end=0; out->rest_elapsed=rest_elapsed(old->rest_start,now);
  out->selected_reps = old->selected_reps;
  out->completion_alerted = out->rest_elapsed >= 180;
  out->rest_end = rest_end_with_five_minute_alert(out->rest_elapsed >= 300);
  workout_state_repair_sync(out);
  return 1;
}
