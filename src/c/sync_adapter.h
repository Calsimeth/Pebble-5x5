#ifndef SYNC_ADAPTER_H
#define SYNC_ADAPTER_H
#include "sync_state.h"
#include <stdbool.h>
#include <stdint.h>

typedef bool (*SyncAdapterOp)(void *context);
typedef bool (*SyncAdapterTimerOp)(uint32_t seconds, void *context);
typedef void (*SyncAdapterCancelOp)(void *context);
typedef struct {
  SyncMachine machine;
  SyncAdapterOp begin, write, send;
  SyncAdapterTimerOp timer;
  SyncAdapterCancelOp cancel;
  void *context;
  bool timer_registered;
} SyncAdapter;

void sync_adapter_init(SyncAdapter *, uint32_t head_id, SyncAdapterOp, SyncAdapterOp, SyncAdapterOp, SyncAdapterTimerOp, SyncAdapterCancelOp, void *);
void sync_adapter_submission_failed(SyncMachine *);
bool sync_adapter_start(SyncAdapter *);
bool sync_adapter_transport(SyncAdapter *, bool delivered);
bool sync_adapter_ack(SyncAdapter *, uint32_t id);
void sync_adapter_timeout(SyncAdapter *);
bool sync_adapter_schedule_retry(SyncAdapter *);
void sync_adapter_retry_elapsed(SyncAdapter *);
void sync_adapter_deinit(SyncAdapter *);
#endif
