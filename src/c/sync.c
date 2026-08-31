#include "sync.h"
#include <string.h>
#include <stdio.h>


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
SyncPushResult sync_pending_promote(SyncQueue *q, const SyncRecord *r, bool pending_valid) {
  if (!pending_valid) return SYNC_PUSH_IDENTICAL;
  return sync_queue_push_result(q, r);
}
uint32_t sync_highest_retained_id(const SyncQueue *q, const SyncRecord *p, bool pv) { uint32_t h=0; if(q) for(uint8_t i=0;i<q->count && i<SYNC_QUEUE_CAPACITY;i++) if(q->records[i].id>h) h=q->records[i].id; if(pv && p && p->id>h) h=p->id; return h; }
bool sync_allocate_id_limited(uint32_t *counter, const SyncQueue *q, const SyncRecord *p, bool pv, uint32_t *out, uint32_t limit) { if(!counter||!out||!limit)return false; uint32_t id=*counter; for(uint32_t tries=0;tries<limit;tries++){id++;if(!id)id++;bool used=pv&&p&&p->id==id;if(q)for(uint8_t i=0;i<q->count&&i<SYNC_QUEUE_CAPACITY;i++)if(q->records[i].id==id)used=true;if(!used){*counter=id;*out=id;return true;}}return false; }
bool sync_allocate_id(uint32_t *counter, const SyncQueue *q, const SyncRecord *p, bool pv, uint32_t *out) { return sync_allocate_id_limited(counter,q,p,pv,out,16); }
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
