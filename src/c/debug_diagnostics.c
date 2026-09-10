#ifdef STRONGLIFTS_DEBUG
#include <pebble.h>
#include <stdio.h>
#include <string.h>
#include "debug_diagnostics.h"
#define DBG_META 30
#define DBG_DATA 31
#define DBG_CAP 48
typedef struct { uint8_t version,head,count; } DMeta; typedef struct { uint8_t code; uint32_t a,b; } DEvent;
uint32_t debug_diag_key(uint8_t index){return index==0?DBG_META:DBG_DATA+(uint32_t)(index-1)*sizeof(DEvent);}
static bool meta(DMeta*m){return persist_read_data(debug_diag_key(0),m,sizeof*m)==(int)sizeof*m&&m->version==1&&m->head<DBG_CAP&&m->count<=DBG_CAP;}
void debug_diag_event(uint8_t c,uint32_t a,uint32_t b){DMeta m={1,0,0},o;DEvent e={c,a,b};if(meta(&o))m=o;uint8_t i=m.head;int r=persist_write_data(debug_diag_key((uint8_t)(i+1)),&e,sizeof e);if(r!=(int)sizeof e){APP_LOG(APP_LOG_LEVEL_ERROR,"DBG_WRITE_FAIL %u %d",c,r);return;}m.head=(i+1)%DBG_CAP;if(m.count<DBG_CAP)m.count++;r=persist_write_data(debug_diag_key(0),&m,sizeof m);if(r!=(int)sizeof m){APP_LOG(APP_LOG_LEVEL_ERROR,"DBG_META_FAIL %d",r);return;}APP_LOG(APP_LOG_LEVEL_INFO,"DBG_EVENT %u %lu %lu",c,(unsigned long)a,(unsigned long)b);}
void debug_diag_persist(const char *op,uint32_t key,uint32_t size,int result){APP_LOG(APP_LOG_LEVEL_INFO,"DBG_PERSIST_%s k=%lu n=%lu r=%d",op,(unsigned long)key,(unsigned long)size,result);}
void debug_diag_state(const char *phase,const int32_t *weights,const uint8_t *plates,uint8_t count){if(!phase||!weights||!plates||count<7)return;APP_LOG(APP_LOG_LEVEL_INFO,"DBG_STATE_%s W=%ld,%ld,%ld,%ld,%ld P=%u,%u,%u,%u,%u,%u,%u",phase,(long)(weights[0]/4),(long)(weights[1]/4),(long)(weights[2]/4),(long)(weights[3]/4),(long)(weights[4]/4),plates[0],plates[1],plates[2],plates[3],plates[4],plates[5],plates[6]);}
void debug_diag_boot(void){APP_LOG(APP_LOG_LEVEL_INFO,"DBG_KEYS %d %d",persist_get_size(debug_diag_key(0)),persist_get_size(debug_diag_key(1)));debug_diag_event(1,0,0);} void debug_diag_shutdown(void){debug_diag_event(2,0,0);} void debug_diag_clear(void){for(uint8_t i=0;i<=DBG_CAP;i++)persist_delete(debug_diag_key(i));APP_LOG(APP_LOG_LEVEL_INFO,"DBG_CLEAR");}
void debug_diag_render(char *out,uint8_t size,uint8_t page){DMeta m={0};DEvent e={0};if(!out||!size||!meta(&m)){snprintf(out,size,"No events");return;}uint8_t start=(uint8_t)(page*4);uint8_t shown=0;out[0]=0;for(uint8_t n=0;n<m.count&&shown<4;n++){uint8_t pos=(uint8_t)((m.head+DBG_CAP-1-n)%DBG_CAP);if(n<start)continue;if(persist_read_data(debug_diag_key((uint8_t)(pos+1)),&e,sizeof e)!=(int)sizeof e)continue;snprintf(out+strlen(out),size-strlen(out),"%u:%lu/%lu\n",e.code,(unsigned long)e.a,(unsigned long)e.b);shown++;}if(!shown)snprintf(out,size,"No events");}
#endif
