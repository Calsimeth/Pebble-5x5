#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "../src/c/persistence.h"
#include "../src/c/rest_state.h"
#include "../src/c/workout_completion.h"

/* Exact schema-10 core layout shipped before the five-minute alert. */
typedef struct {
  uint8_t schema_version, next_workout, active, active_workout, exercise_index, set_index;
  uint8_t rest_active; int32_t rest_start, rest_end; uint8_t halfway_alerted, completion_alerted; uint32_t rest_elapsed;
  Weight weights[5]; Weight active_weights[3]; uint8_t work_reps[3][5], failure_streaks[5];
  PlateCounts inventory_counts; uint8_t warmup_active, warmup_index; WarmupPlan warmup_plan;
  int32_t last_completed; uint8_t deload_pending[5], gap_reviewed[5], failure_reviewed[5], plateau_reviewed[5], accepted_deloads[5];
} ShippedSchema10Core;

typedef struct { uint8_t data[32][256]; size_t size[32]; } Store;
static bool exists(uint32_t key, void *ctx) { Store *s=ctx; return key<32 && s->size[key]; }
static int read_value(uint32_t key, void *out, size_t size, void *ctx) { Store *s=ctx; if(key>=32 || s->size[key]!=size)return -1; memcpy(out,s->data[key],size); return (int)size; }
static int write_value(uint32_t key, const void *value, size_t size, void *ctx) { Store *s=ctx; if(key>=32 || size>256)return -1; memcpy(s->data[key],value,size); s->size[key]=size; return (int)size; }
static const PersistenceAdapter adapter={exists,0,read_value,write_value};

int main(void) {
  assert(sizeof(ShippedSchema10Core)==sizeof(PersistedCoreState));
  assert(offsetof(ShippedSchema10Core,weights)==offsetof(PersistedCoreState,weights));
  assert(offsetof(ShippedSchema10Core,warmup_plan)==offsetof(PersistedCoreState,warmup_plan));
  assert(offsetof(ShippedSchema10Core,accepted_deloads)==offsetof(PersistedCoreState,accepted_deloads));

  ShippedSchema10Core shipped={0}; PersistedSyncState shipped_sync={0}; Store store={0};
  shipped.schema_version=10; shipped.next_workout=WORKOUT_B; shipped.active=1; shipped.active_workout=WORKOUT_A;
  shipped.exercise_index=1; shipped.set_index=3; shipped.rest_active=1; shipped.rest_start=100;
  shipped.halfway_alerted=1; shipped.completion_alerted=1; shipped.rest_elapsed=299;
  shipped.weights[4]=WEIGHT_LB(225); shipped.work_reps[2][4]=4; shipped.inventory_counts[6]=2;
  shipped.warmup_active=1; shipped.last_completed=77; shipped.accepted_deloads[4]=9;
  shipped_sync.schema_version=10; shipped_sync.next_record_id=42; shipped_sync.selected_reps=4;
  assert(persistence_save(&adapter,sizeof shipped,&shipped,sizeof shipped_sync,&shipped_sync,&store)==PERSIST_OK);

  PersistedCoreState core={0}; PersistedSyncState sync={0};
  assert(persistence_load(&adapter,sizeof core,&core,sizeof sync,&sync,&store,0)==PERSIST_OK);
  assert(core.schema_version==10 && core.next_workout==WORKOUT_B && core.set_index==3);
  assert(core.rest_start==100 && core.rest_elapsed==299 && core.weights[4]==WEIGHT_LB(225));
  assert(core.work_reps[2][4]==4 && core.inventory_counts[6]==2 && core.last_completed==77);
  assert(core.accepted_deloads[4]==9 && sync.next_record_id==42 && sync.selected_reps==4);

  RestState rest={.active=core.rest_active,.halfway_alerted=core.halfway_alerted,
    .completion_alerted=core.completion_alerted,.five_minute_alerted=rest_five_minute_alerted_from_end(core.rest_end),
    .start=core.rest_start,.elapsed=core.rest_elapsed};
  assert(rest_alerts_due(&rest,400)==4);
  core.rest_elapsed=rest.elapsed; core.rest_end=rest_end_with_five_minute_alert(rest.five_minute_alerted);
  assert(persistence_save(&adapter,sizeof core,&core,sizeof sync,&sync,&store)==PERSIST_OK);
  memset(&core,0,sizeof core); memset(&sync,0,sizeof sync);
  assert(persistence_load(&adapter,sizeof core,&core,sizeof sync,&sync,&store,0)==PERSIST_OK);
  rest=(RestState){.active=core.rest_active,.halfway_alerted=core.halfway_alerted,
    .completion_alerted=core.completion_alerted,.five_minute_alerted=rest_five_minute_alerted_from_end(core.rest_end),
    .start=core.rest_start,.elapsed=core.rest_elapsed};
  assert(rest_alerts_due(&rest,401)==0);
  return 0;
}
