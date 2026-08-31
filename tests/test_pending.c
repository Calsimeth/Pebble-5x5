#include <assert.h>
#include "../src/c/sync_completion.h"

static SyncRecord rec(uint32_t id) { SyncRecord r={0}; r.id=id; r.schema_version=1; r.workout=0; r.exercise_ids[0]=0; r.exercise_ids[1]=1; r.exercise_ids[2]=2; r.weights[0]=100; r.weights[1]=100; r.weights[2]=100; r.rep_count=15; r.complete=1; return r; }
int main(void) {
  SyncQueue q={0}; SyncRecord p=rec(4);
  assert(sync_pending_promote(&q,&p,true)==SYNC_PUSH_ADDED && q.count==1);
  assert(sync_pending_promote(&q,&p,true)==SYNC_PUSH_IDENTICAL && q.count==1);
  q.count=3; q.records[0]=rec(1); q.records[1]=rec(2); q.records[2]=rec(3);
  assert(sync_pending_promote(&q,&p,true)==SYNC_PUSH_FULL && q.count==3);
  p.id=2; p.weights[0]=101; assert(sync_pending_promote(&q,&p,true)==SYNC_PUSH_CONFLICT && q.count==3);
  assert(sync_pending_promote(&q,&p,false)==SYNC_PUSH_IDENTICAL);
  /* The same production helper is used by startup and ACK handling. */
  q.count=0; p=rec(10); assert(sync_pending_promote(&q,&p,true)==SYNC_PUSH_ADDED); q.count=0; assert(sync_pending_promote(&q,&p,true)==SYNC_PUSH_ADDED);
  q.count=3; q.records[0]=rec(1); q.records[1]=rec(2); q.records[2]=rec(3); p=rec(10); assert(sync_pending_promote(&q,&p,true)==SYNC_PUSH_FULL&&q.count==3);
  p=rec(2); p.weights[1]=101; assert(sync_pending_promote(&q,&p,true)==SYNC_PUSH_CONFLICT&&q.records[1].weights[1]==100);
  q.count=2; q.records[0]=rec(10); q.records[1]=rec(11); p=rec(12); uint8_t pv=1, blocked=1; assert(sync_completion_ack_promote(&q,10,&p,&pv,&blocked)==SYNC_PUSH_ADDED&&!pv&&!blocked&&q.count==2);
  return 0;
}
