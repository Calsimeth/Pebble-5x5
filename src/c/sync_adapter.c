#include "sync_adapter.h"

static void enter_retry(SyncAdapter *a) {
  sync_machine_transport(&a->machine, false);
  sync_adapter_schedule_retry(a);
}
void sync_adapter_init(SyncAdapter *a, uint32_t id, SyncAdapterOp begin, SyncAdapterOp write, SyncAdapterOp send, SyncAdapterTimerOp timer, SyncAdapterCancelOp cancel, void *context) {
  *a=(SyncAdapter){0}; sync_machine_init(&a->machine,id); a->begin=begin; a->write=write; a->send=send; a->timer=timer; a->cancel=cancel; a->context=context;
}
bool sync_adapter_schedule_retry(SyncAdapter *a) {
  if (!a || a->timer_registered || a->machine.state != SYNC_WAITING_RETRY) return a && a->timer_registered;
  if (!a->timer || !a->timer(sync_machine_retry_delay(&a->machine),a->context)) return false;
  a->timer_registered=true; a->machine.timer_active=true; return true;
}
bool sync_adapter_start(SyncAdapter *a) {
  if (!a || a->machine.state != SYNC_IDLE || !a->begin || !a->write || !a->send) return false;
  if (!sync_machine_begin(&a->machine,a->begin(a->context))) { sync_adapter_schedule_retry(a); return false; }
  if (!a->write(a->context)) { enter_retry(a); return false; }
  if (!a->send(a->context)) { enter_retry(a); return false; }
  sync_machine_transport(&a->machine,true); return true;
}
bool sync_adapter_transport(SyncAdapter *a, bool delivered) { if (!a) return false; if (!delivered) { enter_retry(a); return false; } sync_machine_transport(&a->machine,true); return true; }
bool sync_adapter_ack(SyncAdapter *a, uint32_t id) { if (!a || !sync_machine_ack(&a->machine,id)) return false; if (a->timer_registered && a->cancel) a->cancel(a->context); a->timer_registered=false; return true; }
void sync_adapter_timeout(SyncAdapter *a) { if (!a) return; a->timer_registered=false; sync_machine_timeout(&a->machine); sync_adapter_schedule_retry(a); }
void sync_adapter_retry_elapsed(SyncAdapter *a) { if (!a) return; a->timer_registered=false; sync_machine_retry_elapsed(&a->machine); }
void sync_adapter_deinit(SyncAdapter *a) { if (!a) return; if (a->timer_registered && a->cancel) a->cancel(a->context); a->timer_registered=false; sync_machine_deinit(&a->machine); }
