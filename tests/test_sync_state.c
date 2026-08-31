#include <assert.h>
#include "../src/c/sync_state.h"
int main(void){SyncMachine m;sync_machine_init(&m,9);assert(sync_machine_begin(&m,true));sync_machine_transport(&m,true);assert(!sync_machine_ack(&m,8));sync_machine_timeout(&m);assert(sync_machine_retry_delay(&m)==15);assert(sync_machine_begin(&m,true)==false);m.state=SYNC_IDLE;assert(sync_machine_begin(&m,true));sync_machine_transport(&m,true);assert(sync_machine_ack(&m,9));assert(sync_machine_retry_delay(&m)==5);sync_machine_deinit(&m);assert(!m.timer_active);return 0;}
