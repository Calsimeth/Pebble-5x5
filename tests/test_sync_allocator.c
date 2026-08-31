#include <assert.h>
#include <stdint.h>
#include "../src/c/sync.h"
int main(void){
  SyncQueue q={0}; SyncRecord p={.id=2}; uint32_t id;
  uint32_t c=0; assert(sync_allocate_id(&c,&q,&p,true,&id)&&id==1);
  q.count=3; q.records[0].id=1; q.records[1].id=2; q.records[2].id=3;
  c=1; assert(sync_allocate_id(&c,&q,&p,true,&id)&&id==4);
  c=UINT32_MAX; assert(sync_allocate_id(&c,&q,&p,true,&id)&&id==4);
  q.records[0].id=1; q.records[1].id=2; q.records[2].id=3; p.id=3;
  c=UINT32_MAX; assert(sync_allocate_id_limited(&c,&q,&p,true,&id,4)&&id==4);
  q.records[0].id=1; q.records[1].id=2; q.records[2].id=3; p.id=4; c=UINT32_MAX;
  assert(!sync_allocate_id_limited(&c,&q,&p,true,&id,4));
  q.count=0; p.id=99; c=0; assert(sync_allocate_id(&c,&q,&p,false,&id)&&id==1);
  return 0;
}
