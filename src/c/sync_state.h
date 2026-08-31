#ifndef SYNC_STATE_H
#define SYNC_STATE_H
#include <stdint.h>
#include <stdbool.h>
typedef enum { SYNC_IDLE, SYNC_WAITING_TRANSPORT, SYNC_WAITING_ACK, SYNC_WAITING_RETRY } SyncState;
typedef struct { SyncState state; uint8_t retry_index; bool timer_active; bool in_flight; uint32_t head_id; } SyncMachine;
void sync_machine_init(SyncMachine *m, uint32_t head_id);
bool sync_machine_begin(SyncMachine *m, bool ok);
void sync_machine_transport(SyncMachine *m, bool ok);
bool sync_machine_ack(SyncMachine *m, uint32_t id);
uint32_t sync_machine_retry_delay(const SyncMachine *m);
void sync_machine_timeout(SyncMachine *m);
void sync_machine_retry_elapsed(SyncMachine *m);
void sync_machine_deinit(SyncMachine *m);
#endif
