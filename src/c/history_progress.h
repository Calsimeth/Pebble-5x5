#ifndef HISTORY_PROGRESS_H
#define HISTORY_PROGRESS_H
#include <stdint.h>
#include <stdbool.h>
typedef struct { uint16_t request_id, year; uint8_t month, days; uint32_t mask, mask_a, mask_b; } CalendarResponse;
bool calendar_response_valid(const CalendarResponse *r, uint16_t request, uint16_t year, uint8_t month);
typedef struct { int32_t t; uint16_t w; } ProgressPoint;
typedef struct { uint16_t request; uint8_t exercise,page,total,index,count,received,total_points; ProgressPoint points[20]; uint8_t point_counts[4]; uint8_t seen[4]; } ProgressAssembly;
void progress_assembly_reset(ProgressAssembly *a);
bool progress_chunk_add(ProgressAssembly *a,uint16_t request,uint8_t exercise,uint8_t page,uint8_t total,uint8_t index,uint8_t count,uint8_t point_count,const ProgressPoint *p);
bool progress_assembly_complete(const ProgressAssembly *a);
int graph_coordinate(int32_t value,int32_t min,int32_t max,int height);
int graph_x_coordinate(uint8_t index,uint8_t count,int width);
#endif
