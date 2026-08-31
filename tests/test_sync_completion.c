#include <assert.h>
#include "../src/c/sync_completion.h"
typedef struct { uint8_t workout, exercise, set, selected, blocked, reps; } Active;
int main(void) {
  SyncQueue q={0}; SyncRecord p={.id=9};
  assert(sync_completion_capacity_blocked(3,true,2,4,2,4)); assert(!sync_completion_capacity_blocked(2,true,2,4,2,4));
  assert(sync_completion_status(3,true)==WORKOUT_SYNC_REQUIRED); assert(sync_completion_status(1,false)==WORKOUT_SYNC_NOT_SYNCED); assert(sync_completion_status(0,false)==WORKOUT_SYNC_CLEAR);
  assert(sync_completion_promote(&q,&p,true)==SYNC_PUSH_ADDED);
  Active b={1,2,4,3,0,0}; assert(sync_completion_capacity_blocked(3,true,b.exercise,b.set,2,4)); b.blocked=1; assert(b.blocked&&b.selected==3&&b.reps==0&&b.exercise==2&&b.set==4);
  Active reload=b; assert(sync_completion_status(3,reload.blocked)==WORKOUT_SYNC_REQUIRED&&reload.selected==3);
  q.count=3; q.records[0].id=1; q.records[1].id=2; q.records[2].id=3; p.id=4;
  assert(sync_completion_promote(&q,&p,true)==SYNC_PUSH_FULL); assert(b.blocked);
  q.count=1; assert(sync_completion_promote(&q,&p,true)==SYNC_PUSH_ADDED); b.blocked=0; assert(sync_completion_status(q.count,b.blocked)==WORKOUT_SYNC_NOT_SYNCED);
  b.reps=b.selected; b.selected=5; assert(b.reps==3&&b.selected==5);
  Active a={0,2,4,3,0,0}; assert(sync_completion_capacity_blocked(3,true,a.exercise,a.set,2,4)); a.blocked=1; assert(a.blocked&&a.selected==3&&a.reps==0); a.blocked=0; a.reps=a.selected; a.selected=5; assert(a.reps==3&&a.selected==5);
  return 0;
}
