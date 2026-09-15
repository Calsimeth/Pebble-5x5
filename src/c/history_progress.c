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
int graph_x_coordinate(uint8_t index,uint8_t count,int width){if(width<=0||!count||index>=count)return 0;return count==1?width/2:(int)((int64_t)index*(width-1)/(count-1));}
void graph_axis_bounds(int32_t observed_min,int32_t observed_max,bool includes_zero,int32_t *axis_min,int32_t *axis_max){
  if(!axis_min||!axis_max)return;
  if(observed_max<observed_min){*axis_min=*axis_max=0;return;}
  int32_t lo=observed_min,hi=observed_max;
  int32_t span=hi-lo;
  if(!span){lo-=25;hi+=25;}
  else {int32_t pad=span/10;if(pad<5)pad=5;lo-=pad;hi+=pad;}
  lo=(lo/25)*25; if(lo>observed_min)lo-=25;
  hi=((hi+24)/25)*25; if(hi<observed_max)hi+=25;
  if(includes_zero){if(lo>0)lo=0;if(hi<0)hi=0;}
  else {if(observed_min>0&&lo<=0)lo=observed_min;if(observed_max<0&&hi>=0)hi=observed_max;}
  if(hi<=lo)hi=lo+25;
  *axis_min=lo;*axis_max=hi;
}
uint8_t progress_picker_move(uint8_t selected,int8_t delta){int next=(int)selected+delta;if(next<0)next=0;if(next>4)next=4;return (uint8_t)next;}
ProgressDisplayState progress_display_state(bool connected,bool failed,bool complete,uint8_t point_count){if(!connected)return failed?PROGRESS_DISPLAY_PHONE_NEEDED:PROGRESS_DISPLAY_LOADING;if(complete&&point_count)return PROGRESS_DISPLAY_GRAPH;return complete?PROGRESS_DISPLAY_EMPTY:PROGRESS_DISPLAY_LOADING;}
