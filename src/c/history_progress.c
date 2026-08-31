#include "history_progress.h"
#include <string.h>
bool calendar_response_valid(const CalendarResponse *r,uint16_t q,uint16_t y,uint8_t m){return r&&r->request_id==q&&r->year==y&&r->month==m&&r->days>=28&&r->days<=31&&(r->days<32||r->mask>>31==0);}
void progress_assembly_reset(ProgressAssembly *a){if(a)memset(a,0,sizeof(*a));}
bool progress_chunk_add(ProgressAssembly *a,uint16_t q,uint8_t page,uint8_t total,uint8_t index,uint8_t count,const ProgressPoint *p,uint8_t n){if(!a||!p||!q||!count||count>4||index>=count||n>5||!total||page>=total)return false;if(a->received&& (a->request!=q||a->page!=page||a->total!=total||a->count!=count))return false;if(!a->received){a->request=q;a->page=page;a->total=total;a->count=count;}if(a->seen[index])return memcmp(a->points+index*5,p,n*sizeof(*p))==0;if(n==0)return false;memcpy(a->points+index*5,p,n*sizeof(*p));a->seen[index]=1;a->received++;return true;}
bool progress_assembly_complete(const ProgressAssembly *a){return a&&a->received==a->count;}
int graph_coordinate(int32_t v,int32_t min,int32_t max,int h){if(h<=0)return 0;if(max<=min)return h/2;if(v<=min)return h-1;if(v>=max)return 0;return (int)(((int64_t)(max-v)*(h-1))/(max-min));}
