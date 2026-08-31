#include <assert.h>
#include "../src/c/sync_state.h"
int main(void){
  SyncMachine m; sync_machine_init(&m,9);
  assert(sync_machine_begin(&m,false)==false && m.state==SYNC_WAITING_RETRY && m.timer_active);
  assert(sync_machine_retry_delay(&m)==5); sync_machine_retry_elapsed(&m);
  assert(sync_machine_begin(&m,true)); assert(m.state==SYNC_WAITING_TRANSPORT);
  sync_machine_transport(&m,false); assert(m.state==SYNC_WAITING_RETRY&&sync_machine_retry_delay(&m)==15);
  sync_machine_retry_elapsed(&m); assert(sync_machine_begin(&m,true)); sync_machine_transport(&m,false);
  assert(sync_machine_retry_delay(&m)==30); sync_machine_retry_elapsed(&m);
  assert(sync_machine_begin(&m,true)); sync_machine_transport(&m,false); assert(sync_machine_retry_delay(&m)==60);
  sync_machine_retry_elapsed(&m); assert(sync_machine_begin(&m,true)); sync_machine_transport(&m,false);
  assert(sync_machine_retry_delay(&m)==60); sync_machine_retry_elapsed(&m);
  assert(sync_machine_begin(&m,true)); sync_machine_transport(&m,true);
  assert(m.state==SYNC_WAITING_ACK&&m.timer_active);
  assert(!sync_machine_ack(&m,0)&&m.timer_active); assert(!sync_machine_ack(&m,8)&&m.timer_active);
  assert(sync_machine_ack(&m,9)&&!m.timer_active&&sync_machine_retry_delay(&m)==5);
  assert(!sync_machine_ack(&m,9)); sync_machine_deinit(&m); assert(!m.timer_active&&!m.in_flight);
  return 0;
}
