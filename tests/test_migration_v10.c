#include <assert.h>
#include "../src/c/migration.h"
int main(void) {
  PersistedStateV9 v = {0}; PersistedState out = {0};
  v.schema_version=9; v.next_workout=WORKOUT_A; v.active=1; v.active_workout=WORKOUT_A;
  v.rest_active=1; v.rest_start=100; v.selected_reps=3;
  assert(migrate_v9_to_v10(&v,&out,100));
  assert(out.schema_version==10 && out.rest_elapsed==0 && out.selected_reps==3);
  v.schema_version=10; assert(!migrate_v9_to_v10(&v,&out,100));
  return 0;
}
