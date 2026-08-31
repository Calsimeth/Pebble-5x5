#include "sync.h"
#include <string.h>
#include <stdio.h>

static void put16(uint8_t *p, uint16_t v) { p[0]=v; p[1]=v>>8; }
static uint16_t get16(const uint8_t *p) { return p[0] | ((uint16_t)p[1]<<8); }
static void put32(uint8_t *p, uint32_t v) { for (int i=0;i<4;i++) p[i]=(uint8_t)(v>>(8*i)); }
static uint32_t get32(const uint8_t *p) { uint32_t v=0; for (int i=3;i>=0;i--) v=(v<<8)|p[i]; return v; }

bool sync_record_equal(const SyncRecord *a, const SyncRecord *b) {
  if (!a || !b || a->id != b->id || a->schema_version != b->schema_version || a->workout != b->workout || a->completed_at != b->completed_at || a->complete != b->complete || a->rep_count != b->rep_count || a->deload_mask != b->deload_mask || a->deload_decisions != b->deload_decisions) return false;
  return !memcmp(a->exercise_ids,b->exercise_ids,3) && !memcmp(a->weights,b->weights,sizeof a->weights) && !memcmp(a->reps,b->reps,a->rep_count);
}
SyncPushResult sync_queue_push_result(SyncQueue *q, const SyncRecord *r) {
  if (!q || !r || r->id == 0) return SYNC_PUSH_CONFLICT;
  for (uint8_t i=0;i<q->count && i<SYNC_QUEUE_CAPACITY;i++) if (q->records[i].id == r->id) return sync_record_equal(&q->records[i],r) ? SYNC_PUSH_IDENTICAL : SYNC_PUSH_CONFLICT;
  if (q->count >= SYNC_QUEUE_CAPACITY) return SYNC_PUSH_FULL;
  q->records[q->count++] = *r; return SYNC_PUSH_ADDED;
}
bool sync_queue_push(SyncQueue *q, const SyncRecord *r) { return sync_queue_push_result(q,r) == SYNC_PUSH_ADDED; }
uint32_t sync_highest_retained_id(const SyncQueue *q, const SyncRecord *p, bool pv) { uint32_t h=0; if(q) for(uint8_t i=0;i<q->count && i<SYNC_QUEUE_CAPACITY;i++) if(q->records[i].id>h) h=q->records[i].id; if(pv && p && p->id>h) h=p->id; return h; }
bool sync_allocate_id(uint32_t *counter, const SyncQueue *q, const SyncRecord *p, bool pv, uint32_t *out) { if(!counter||!out)return false; uint32_t id=*counter; for(uint32_t tries=0;tries<UINT32_MAX-1 && tries<16;tries++){id++;if(!id)id++;bool used=pv&&p&&p->id==id;if(q)for(uint8_t i=0;i<q->count&&i<SYNC_QUEUE_CAPACITY;i++)if(q->records[i].id==id)used=true;if(!used){*counter=id;*out=id;return true;}}return false; }
int sync_record_to_json(const SyncRecord *r, char *o, size_t cap) {
  if (!sync_record_valid(r) || !o || !cap) return 0;
  int n=snprintf(o,cap,"{\"v\":%u,\"id\":%lu,\"t\":%ld,\"w\":%u,\"e\":[%u,%u,%u],\"wt\":[%u,%u,%u],\"r\":[",r->schema_version,(unsigned long)r->id,(long)r->completed_at,r->workout,r->exercise_ids[0],r->exercise_ids[1],r->exercise_ids[2],r->weights[0],r->weights[1],r->weights[2]);
  if (n < 0 || (size_t)n >= cap) return 0;
  for (uint8_t i=0; i<r->rep_count; i++) { int x=snprintf(o+n,cap-n,"%s%u",i?",":"",r->reps[i]); if (x<0 || (size_t)(n+x)>=cap) return 0; n+=x; }
  int x=snprintf(o+n,cap-n,"],\"c\":%u,\"d\":%u}",r->complete,r->deload_mask); return x<0||(size_t)(n+x)>=cap?0:n+x;
}
bool sync_queue_ack(SyncQueue *q, uint32_t id) {
  if (!q || id == 0) return false;
  for (uint8_t i=0;i<q->count;i++) if (q->records[i].id == id) {
    memmove(&q->records[i], &q->records[i+1], (q->count-i-1)*sizeof(SyncRecord)); q->count--; return true;
  }
  return false;
}
const SyncRecord *sync_queue_peek(const SyncQueue *q) { return q && q->count ? &q->records[0] : NULL; }
bool sync_record_valid(const SyncRecord *r) {
  if (!r || !r->id || r->schema_version != SYNC_RECORD_VERSION || r->workout > 1 || r->complete > 1) return false;
  uint8_t expected = r->workout ? 11 : 15;
  if (r->rep_count != expected || r->deload_decisions > 1) return false;
  const uint8_t ids[2][3] = {{0,1,2},{0,3,4}};
  for (uint8_t e=0;e<3;e++) if (r->exercise_ids[e] != ids[r->workout][e] || !r->weights[e]) return false;
  for (uint8_t i=0;i<r->rep_count;i++) if (r->reps[i] > 5) return false;
  return true;
}
bool sync_queue_valid(const SyncQueue *q) {
  if (!q || q->count > SYNC_QUEUE_CAPACITY) return false;
  for (uint8_t i=0;i<q->count;i++) { if (!sync_record_valid(&q->records[i])) return false; for (uint8_t j=0;j<i;j++) if (q->records[i].id == q->records[j].id) return false; }
  return true;
}

uint16_t sync_record_serialize(const SyncRecord *r, uint8_t *o, uint16_t cap) {
  if (!r || !o || r->rep_count > SYNC_MAX_SETS) return 0;
  uint16_t n = (uint16_t)(23 + r->rep_count); if (cap < n) return 0;
  o[0]=r->schema_version; put32(o+1,r->id); put32(o+5,(uint32_t)r->completed_at); o[9]=r->workout; memcpy(o+10,r->exercise_ids,3);
  for (int i=0; i<3; i++) put16(o+13+i*2, r->weights[i]);
  o[19]=r->complete; o[20]=r->rep_count; memcpy(o+21,r->reps,r->rep_count);
  o[21+r->rep_count]=r->deload_mask; o[22+r->rep_count]=r->deload_decisions;
  return (uint16_t)(23+r->rep_count);
}
bool sync_record_deserialize(SyncRecord *r, const uint8_t *o, uint16_t n) {
  if (!r || !o || n < 23 || o[20] > SYNC_MAX_SETS || n != 23+o[20] || o[0] != SYNC_RECORD_VERSION) return false;
  memset(r,0,sizeof(*r)); r->schema_version=o[0]; r->id=get32(o+1); r->completed_at=(int32_t)get32(o+5); r->workout=o[9]; memcpy(r->exercise_ids,o+10,3);
  for (int i=0; i<3; i++) r->weights[i]=get16(o+13+i*2);
  r->complete=o[19]; r->rep_count=o[20]; memcpy(r->reps,o+21,r->rep_count);
  r->deload_mask=o[21+r->rep_count]; r->deload_decisions=o[22+r->rep_count]; return r->id != 0;
}
