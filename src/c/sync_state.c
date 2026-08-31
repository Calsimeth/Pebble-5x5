#include "sync_state.h"
static const uint16_t DELAYS[] = {5,15,30,60,60};
void sync_machine_init(SyncMachine *m,uint32_t id){*m=(SyncMachine){.state=id?SYNC_IDLE:SYNC_IDLE,.head_id=id};}
uint32_t sync_machine_retry_delay(const SyncMachine *m){return DELAYS[m->retry_index<5?m->retry_index:4];}
static void retry(SyncMachine *m){m->state=SYNC_WAITING_RETRY;m->in_flight=false;m->timer_active=true;}
bool sync_machine_begin(SyncMachine *m,bool ok){if(!m||m->state!=SYNC_IDLE||!ok){if(m&&m->state==SYNC_IDLE)retry(m);return false;}m->state=SYNC_WAITING_TRANSPORT;m->in_flight=false;return true;}
void sync_machine_transport(SyncMachine *m,bool ok){if(!m)return;if(ok){m->state=SYNC_WAITING_ACK;m->in_flight=true;m->timer_active=true;}else retry(m);}
bool sync_machine_ack(SyncMachine *m,uint32_t id){if(!m||m->state!=SYNC_WAITING_ACK||!id||id!=m->head_id)return false;m->state=SYNC_IDLE;m->in_flight=false;m->timer_active=false;m->retry_index=0;return true;}
void sync_machine_timeout(SyncMachine *m){if(m&&m->state==SYNC_WAITING_ACK)retry(m);}
void sync_machine_retry_elapsed(SyncMachine *m){if(m&&m->state==SYNC_WAITING_RETRY){if(m->retry_index<4)m->retry_index++;m->state=SYNC_IDLE;m->timer_active=false;}}
void sync_machine_deinit(SyncMachine *m){if(m){m->timer_active=false;m->in_flight=false;m->state=SYNC_IDLE;}}
