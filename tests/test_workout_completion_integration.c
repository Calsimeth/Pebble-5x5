#include <assert.h>
#include <string.h>
#include "../src/c/workout_completion.h"

static SyncRecord record(uint32_t id) {
  SyncRecord r={.id=id,.schema_version=1,.workout=0,.exercise_ids={0,1,2},.weights={180,180,260},.rep_count=15,.complete=1};
  memset(r.reps,5,15); return r;
}
static void restart_round_trip(const PersistedState *state, PersistedState *reloaded) {
  uint8_t wire[WORKOUT_STATE_WIRE_SIZE];
  int length = workout_state_encode(state, wire, sizeof wire);
  assert(length == (int)sizeof wire);
  assert(workout_state_decode(reloaded, wire, (uint16_t)length));
}
static PersistedState initial_state(void) {
  PersistedState s={0}; s.schema_version=WORKOUT_STORAGE_SCHEMA; s.active=1; s.active_workout=WORKOUT_A; s.exercise_index=2; s.set_index=4; s.selected_reps=3;
  s.active_weights[0]=180; s.active_weights[1]=180; s.active_weights[2]=260; s.inventory_counts[0]=2; s.inventory_counts[2]=2; s.inventory_counts[4]=2;
  for(unsigned e=0;e<3;e++) for(unsigned set=0;set<5;set++) s.work_reps[e][set]=5;
  s.outbox.records[0]=record(1); s.outbox.records[1]=record(2); s.outbox.records[2]=record(3); s.outbox.count=3; s.pending_record=record(4); s.pending_valid=1; s.next_record_id=4;
  return s;
}
int main(void) {
  PersistedState s=initial_state(), before=s, reloaded={0};
  assert(workout_completion_attempt(&s,3,100)==COMPLETION_BLOCKED);
  assert(s.completion_blocked&&s.selected_reps==3&&s.work_reps[2][4]==before.work_reps[2][4]);
  assert(s.active==before.active&&s.exercise_index==before.exercise_index&&s.set_index==before.set_index&&s.next_workout==before.next_workout&&s.last_completed==before.last_completed);
  restart_round_trip(&s,&reloaded); assert(reloaded.completion_blocked&&reloaded.selected_reps==3);
  assert(workout_completion_handle_ack(&reloaded,1)==SYNC_PUSH_ADDED); assert(!reloaded.completion_blocked&&!reloaded.pending_valid&&reloaded.outbox.count==3);
  assert(workout_completion_attempt(&reloaded,3,101)==COMPLETION_OK); assert(!reloaded.active&&reloaded.next_workout==WORKOUT_B&&reloaded.last_completed==101&&reloaded.outbox.count==3&&reloaded.pending_valid);
  PersistedState completed={0}; restart_round_trip(&reloaded,&completed); assert(completed.last_completed==101&&completed.outbox.count==3&&completed.pending_valid);
  PersistedState failed=initial_state(); failed.outbox.count=0; failed.pending_valid=0; failed.work_reps[2][4]=0; Weight old=failed.active_weights[2]; unsigned streak=failed.failure_streaks[2];
  assert(workout_completion_attempt(&failed,3,200)==COMPLETION_OK); assert(failed.weights[2]==old&&failed.failure_streaks[2]==streak+1);
  return 0;
}
