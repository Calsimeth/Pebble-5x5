#include <assert.h>
#include <stdint.h>
#include "../src/c/sync.h"
int main(void){SyncQueue q={0};SyncRecord p={.id=2};uint32_t id;assert(sync_allocate_id(&(uint32_t){0},&q,&p,true,&id)&&id==1);q.count=3;q.records[0].id=1;q.records[1].id=2;q.records[2].id=3;uint32_t c=UINT32_MAX;assert(sync_allocate_id(&c,&q,&p,true,&id)&&id==4);c=0;q.count=0;assert(sync_allocate_id(&c,&q,&p,false,&id)&&id==1);return 0;}
