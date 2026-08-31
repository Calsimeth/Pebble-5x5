#include "history_progress.h"
#include <string.h>
static uint8_t month_days(uint16_t y,uint8_t m){static const uint8_t d[]={31,28,31,30,31,30,31,31,30,31,30,31};return d[m-1]+(m==2&&((y%4==0&&y%100!=0)||y%400==0));}
bool calendar_response_valid(const CalendarResponse *r,uint16_t q,uint16_t y,uint8_t m){if(!r||!q||r->request_id!=q||r->year!=y||r->month!=m||m<1||m>12||r->days!=month_days(y,m))return false;uint32_t allowed=r->days==31?0x7fffffffU:((1u<<r->days)-1u);return !(r->mask&~allowed)&&!(r->mask_a&~allowed)&&!(r->mask_b&~allowed)&&((r->mask_a|r->mask_b)&~r->mask)==0;}
void progress_assembly_reset(ProgressAssembly *a){if(a)memset(a,0,sizeof(*a));}
bool progress_chunk_add(ProgressAssembly *a,uint16_t q,uint8_t exercise,uint8_t page,uint8_t total,uint8_t index,uint8_t count,uint8_t point_count,const ProgressPoint *p){
  if(!a||!q||count>4||index>=count||point_count>5||(!point_count&&!(!total&&count==1&&index==0))||(point_count&&!p)||(total&&page>=total))return false;
  for(uint8_t i=0;i<point_count;i++)if(!p[i].t||!p[i].w)return false;
  if(a->received&&(a->request!=q||a->exercise!=exercise||a->page!=page||a->total!=total||a->count!=count))return false;
  if(!a->received){a->request=q;a->exercise=exercise;a->page=page;a->total=total;a->count=count;}
  if(a->seen[index])return a->point_counts[index]==point_count&&memcmp(a->points+index*5,p,point_count*sizeof(*p))==0;
  if(a->total_points+point_count>20)return false;
  memcpy(a->points+index*5,p,point_count*sizeof(*p));
  a->point_counts[index]=point_count;a->total_points+=point_count;a->seen[index]=1;a->received++;return true;
}
bool progress_assembly_complete(const ProgressAssembly *a){return a&&a->received==a->count;}
int graph_coordinate(int32_t v,int32_t min,int32_t max,int h){if(h<=0)return 0;if(max<=min)return h/2;if(v<=min)return h-1;if(v>=max)return 0;return (int)(((int64_t)(max-v)*(h-1))/(max-min));}
