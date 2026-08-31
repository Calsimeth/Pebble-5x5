#include <stdio.h>
#include <string.h>
#include "../src/c/sync.h"
static void emit(uint8_t workout, uint32_t id, int32_t t) { SyncRecord r={.id=id,.schema_version=1,.workout=workout,.completed_at=t,.exercise_ids={0,1,2},.weights={65535,65535,65535},.rep_count=15,.complete=1}; if(workout){r.exercise_ids[1]=3;r.exercise_ids[2]=4;r.rep_count=11;} memset(r.reps,5,sizeof r.reps); r.reps[0]=0; char out[256]; if(sync_record_to_json(&r,out,sizeof out)>0) puts(out); }
int main(void) { emit(0,7,123); emit(1,UINT32_MAX,2147483647); return 0; }
