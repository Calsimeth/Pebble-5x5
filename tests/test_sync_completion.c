#include <assert.h>
#include "../src/c/sync_completion.h"
int main(void) {
  SyncQueue q={0}; SyncRecord p={.id=9};
  assert(sync_completion_capacity_blocked(3,true,2,4,2,4)); assert(!sync_completion_capacity_blocked(2,true,2,4,2,4));
  assert(sync_completion_status(3,true)==WORKOUT_SYNC_REQUIRED); assert(sync_completion_status(1,false)==WORKOUT_SYNC_NOT_SYNCED); assert(sync_completion_status(0,false)==WORKOUT_SYNC_CLEAR);
  assert(sync_completion_promote(&q,&p,true)==SYNC_PUSH_ADDED);
  uint8_t selected=3, blocked=0, logged=0;
  assert(!sync_completion_log_set(&logged,&selected,&blocked,3,true,2,4,2,4));
  assert(blocked==1&&selected==3&&logged==0);
  assert(sync_completion_status(3,blocked)==WORKOUT_SYNC_REQUIRED);
  q.count=3; q.records[0].id=1; q.records[1].id=2; q.records[2].id=3; p.id=4;
  assert(sync_completion_promote(&q,&p,true)==SYNC_PUSH_FULL); assert(blocked);
  q.count=1; assert(sync_completion_promote(&q,&p,true)==SYNC_PUSH_ADDED); blocked=0; assert(sync_completion_status(q.count,blocked)==WORKOUT_SYNC_NOT_SYNCED);
  blocked=0; assert(sync_completion_log_set(&logged,&selected,&blocked,2,true,2,4,2,4)&&logged==3&&selected==5&&!blocked);
  int32_t weights_before[5]={1,2,3,4,5}, weights_after[5]={0}; uint8_t flags[5]={1,2,3,4,5}, zeros[5]={0}; SyncCompletionRollback rollback;
  sync_completion_snapshot(&rollback,weights_before,flags,flags,flags,flags,flags,1,2,4,1,77,1,&p);
  uint8_t restored_active=0, restored_exercise=0, restored_set=0, restored_workout=0, restored_pending=0; int32_t restored_time=0;
  sync_completion_restore(&rollback,weights_after,zeros,zeros,zeros,zeros,zeros,&restored_active,&restored_exercise,&restored_set,&restored_workout,&restored_time,&restored_pending,&p);
  assert(weights_after[2]==3&&zeros[0]==1&&restored_active==1&&restored_exercise==2&&restored_set==4&&restored_workout==1&&restored_time==77&&restored_pending==1&&p.id==4);
  uint16_t weights[3]={180,180,260}; uint8_t work[3][5]={{5,5,5,5,5},{5,5,5,5,5},{0,0,0,0,0}}; uint8_t sets[3]={5,5,5}; SyncRecord built={0}; assert(sync_completion_build_record(&built,21,0,123,weights,work,sets,0)); assert(built.exercise_ids[0]==0&&built.exercise_ids[1]==1&&built.exercise_ids[2]==2&&built.rep_count==15&&built.reps[10]==0);
  sets[0]=5; sets[1]=5; sets[2]=2; assert(!sync_completion_build_record(&built,22,1,123,weights,work,sets,0)); sets[2]=1; assert(sync_completion_build_record(&built,22,1,123,weights,work,sets,0)); assert(built.exercise_ids[0]==0&&built.exercise_ids[1]==3&&built.exercise_ids[2]==4&&built.rep_count==11);
  return 0;
}
