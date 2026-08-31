#include <assert.h>
#include "../src/c/sync_completion.h"
int main(void) { SyncQueue q={0}; SyncRecord p={.id=9}; assert(sync_completion_capacity_blocked(3,true,2,4,2,4)); assert(!sync_completion_capacity_blocked(2,true,2,4,2,4)); assert(sync_completion_status(3,true)==WORKOUT_SYNC_REQUIRED); assert(sync_completion_status(1,false)==WORKOUT_SYNC_NOT_SYNCED); assert(sync_completion_status(0,false)==WORKOUT_SYNC_CLEAR); assert(sync_completion_promote(&q,&p,true)==SYNC_PUSH_ADDED); return 0; }
