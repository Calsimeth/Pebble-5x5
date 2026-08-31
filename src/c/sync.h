#ifndef SYNC_H
#define SYNC_H
#include <stdint.h>
#include <stdbool.h>

#define SYNC_QUEUE_CAPACITY 3
#define SYNC_RECORD_VERSION 1
#define SYNC_MAX_SETS 15

typedef struct {
  uint32_t id;
  uint8_t schema_version, workout, exercise_ids[3], complete;
  int32_t completed_at;
  uint16_t weights[3];
  uint8_t reps[SYNC_MAX_SETS];
  uint8_t rep_count;
  uint8_t deload_mask, deload_decisions;
} SyncRecord;

typedef struct { uint8_t count; SyncRecord records[SYNC_QUEUE_CAPACITY]; } SyncQueue;
bool sync_queue_push(SyncQueue *, const SyncRecord *);
bool sync_queue_ack(SyncQueue *, uint32_t id);
const SyncRecord *sync_queue_peek(const SyncQueue *);
uint16_t sync_record_serialize(const SyncRecord *, uint8_t *out, uint16_t cap);
bool sync_record_deserialize(SyncRecord *, const uint8_t *, uint16_t);
bool sync_record_valid(const SyncRecord *);
bool sync_queue_valid(const SyncQueue *);
#endif
