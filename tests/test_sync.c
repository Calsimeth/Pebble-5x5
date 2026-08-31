#include <assert.h>
#include <string.h>
#include "../src/c/sync.h"
int main(void) { SyncQueue q={0}; SyncRecord r={.id=7,.schema_version=1,.rep_count=2,.reps={5,3},.weights={180,180,260}}; uint8_t b[64]; assert(sync_queue_push_result(&q,&r)==SYNC_PUSH_ADDED); assert(sync_queue_push_result(&q,&r)==SYNC_PUSH_IDENTICAL); r.weights[0]++; assert(sync_queue_push_result(&q,&r)==SYNC_PUSH_CONFLICT); r.weights[0]--; assert(q.count==1); assert(sync_record_serialize(&r,b,sizeof b)==25); SyncRecord x; assert(sync_record_deserialize(&x,b,25)&&x.id==7&&x.reps[1]==3); assert(sync_queue_ack(&q,7)&&q.count==0); return 0; }
