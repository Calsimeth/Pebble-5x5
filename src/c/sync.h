#ifndef SYNC_H
#define SYNC_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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
typedef enum { SYNC_PUSH_ADDED, SYNC_PUSH_IDENTICAL, SYNC_PUSH_FULL, SYNC_PUSH_CONFLICT } SyncPushResult;
SyncPushResult sync_queue_push_result(SyncQueue *, const SyncRecord *);
SyncPushResult sync_pending_promote(SyncQueue *, const SyncRecord *, bool pending_valid);
bool sync_queue_ack(SyncQueue *, uint32_t id);
const SyncRecord *sync_queue_peek(const SyncQueue *);
bool sync_record_valid(const SyncRecord *);
bool sync_queue_valid(const SyncQueue *);
bool sync_record_equal(const SyncRecord *, const SyncRecord *);
uint32_t sync_highest_retained_id(const SyncQueue *, const SyncRecord *, bool);
bool sync_allocate_id(uint32_t *counter, const SyncQueue *, const SyncRecord *, bool, uint32_t *out);
bool sync_allocate_id_limited(uint32_t *counter, const SyncQueue *, const SyncRecord *, bool, uint32_t *out, uint32_t search_limit);
int sync_record_to_json(const SyncRecord *, char *, size_t);
#endif
