#include "sync.h"
#include <string.h>

static void put16(uint8_t *p, uint16_t v) { p[0]=v; p[1]=v>>8; }
static uint16_t get16(const uint8_t *p) { return p[0] | ((uint16_t)p[1]<<8); }
static void put32(uint8_t *p, uint32_t v) { for (int i=0;i<4;i++) p[i]=(uint8_t)(v>>(8*i)); }
static uint32_t get32(const uint8_t *p) { uint32_t v=0; for (int i=3;i>=0;i--) v=(v<<8)|p[i]; return v; }

bool sync_queue_push(SyncQueue *q, const SyncRecord *r) {
  if (!q || !r || q->count >= SYNC_QUEUE_CAPACITY || r->id == 0) return false;
  for (uint8_t i=0;i<q->count;i++) if (q->records[i].id == r->id) return true;
  q->records[q->count++] = *r; return true;
}
bool sync_queue_ack(SyncQueue *q, uint32_t id) {
  if (!q || id == 0) return false;
  for (uint8_t i=0;i<q->count;i++) if (q->records[i].id == id) {
    memmove(&q->records[i], &q->records[i+1], (q->count-i-1)*sizeof(SyncRecord)); q->count--; return true;
  }
  return false;
}
const SyncRecord *sync_queue_peek(const SyncQueue *q) { return q && q->count ? &q->records[0] : NULL; }

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
