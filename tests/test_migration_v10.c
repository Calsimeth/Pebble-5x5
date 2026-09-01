#include <assert.h>
#include "../src/c/migration.h"
int main(void) {
  assert(sizeof(PersistedCoreState) <= 256);
  assert(sizeof(PersistedSyncState) <= 256);
  PersistedStateV9 v = {0}; PersistedState out = {0};
  v.schema_version=9; v.next_workout=WORKOUT_A; v.active=1; v.active_workout=WORKOUT_A;
  v.rest_active=1; v.rest_start=100; v.selected_reps=3;
  assert(migrate_v9_to_v10(&v,&out,100));
  assert(out.schema_version==10 && out.rest_elapsed==0 && out.selected_reps==3);
  v.schema_version=9; v.weights[0]=777; v.inventory_counts[0]=4; v.work_reps[1][2]=3;
  v.outbox.count=SYNC_QUEUE_CAPACITY+1; v.pending_valid=1; v.pending_record.schema_version=99;
  assert(migrate_v9_to_v10(&v,&out,100));
  assert(out.weights[0]==777 && out.inventory_counts[0]==4 && out.work_reps[1][2]==3);
  assert(out.outbox.count==0 && out.pending_valid==0 && out.completion_blocked==0);
  v.schema_version=10; assert(!migrate_v9_to_v10(&v,&out,100));
  return 0;
}
