#ifdef STRONGLIFTS_DEBUG
#include <pebble.h>
#include <stdio.h>
#include <string.h>
#include "debug_diagnostics.h"
#define DBG_META 30
#define DBG_DATA 31
#define DBG_CAP 48
typedef struct { uint8_t version,head,count; } DMeta; typedef struct { uint8_t code; uint32_t a,b; } DEvent;
static bool meta(DMeta*m){return persist_read_data(DBG_META,m,sizeof*m)==(int)sizeof*m&&m->version==1&&m->head<DBG_CAP&&m->count<=DBG_CAP;}
void debug_diag_event(uint8_t c,uint32_t a,uint32_t b){DMeta m={1,0,0},o;DEvent e={c,a,b};if(meta(&o))m=o;uint8_t i=m.head;int r=persist_write_data(DBG_DATA+i*sizeof e,&e,sizeof e);if(r!=(int)sizeof e){APP_LOG(APP_LOG_LEVEL_ERROR,"DBG_WRITE_FAIL %u %d",c,r);return;}m.head=(i+1)%DBG_CAP;if(m.count<DBG_CAP)m.count++;r=persist_write_data(DBG_META,&m,sizeof m);if(r!=(int)sizeof m){APP_LOG(APP_LOG_LEVEL_ERROR,"DBG_META_FAIL %d",r);return;}APP_LOG(APP_LOG_LEVEL_INFO,"DBG_EVENT %u %lu %lu",c,(unsigned long)a,(unsigned long)b);}
void debug_diag_boot(void){APP_LOG(APP_LOG_LEVEL_INFO,"DBG_KEYS %d %d",persist_get_size(DBG_META),persist_get_size(DBG_DATA));debug_diag_event(1,0,0);} void debug_diag_shutdown(void){debug_diag_event(2,0,0);} void debug_diag_clear(void){persist_delete(DBG_META);persist_delete(DBG_DATA);APP_LOG(APP_LOG_LEVEL_INFO,"DBG_CLEAR");}
void debug_diag_render(char *out,uint8_t size,uint8_t page){DMeta m={0};DEvent e={0};if(!out||!size||!meta(&m)){snprintf(out,size,"No events");return;}uint8_t start=(uint8_t)(page*4);uint8_t shown=0;out[0]=0;for(uint8_t n=0;n<m.count&&shown<4;n++){uint8_t pos=(uint8_t)((m.head+m.count-1-n)%DBG_CAP);if(n<start)continue;if(persist_read_data(DBG_DATA+pos*sizeof e,&e,sizeof e)!=(int)sizeof e)continue;snprintf(out+strlen(out),size-strlen(out),"%u:%lu/%lu\n",e.code,(unsigned long)e.a,(unsigned long)e.b);shown++;}if(!shown)snprintf(out,size,"No events");}
#endif
