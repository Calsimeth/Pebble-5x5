#include <assert.h>
#include <string.h>
#include "../src/c/workout_transition.h"

static FinalSetEvent events[8]; static uint32_t times[8]; static unsigned count;
static void log_event(FinalSetEvent e, uint8_t exercise, uint8_t set, uint8_t reps, uint32_t elapsed, void *ctx) { (void)exercise; (void)set; (void)reps; (void)ctx; events[count]=e; times[count++]=elapsed; }
static void run_case(uint8_t exercise, uint8_t set, uint8_t reps) {
  FinalSetTransition t; memset(events,0,sizeof events); memset(times,0,sizeof times); count=0;
  final_set_transition_init(&t, log_event, NULL);
  assert(!final_set_transition_begin(&t,exercise,set,reps,false,1000));
  assert(final_set_transition_begin(&t,exercise,set,reps,true,1000));
  assert(final_set_transition_visible(&t)); assert(events[0]==FINAL_SET_RECORDED && events[1]==FINAL_SET_VISIBLE_LOG);
  assert(!final_set_transition_advance(&t,2999)); assert(t.phase==FINAL_SET_VISIBLE);
  assert(final_set_transition_select(&t)); assert(events[2]==FINAL_SET_DUPLICATE_IGNORED);
  assert(final_set_transition_advance(&t,3000)); assert(t.phase==FINAL_SET_ADVANCED && t.advance_count==1 && times[3]==2000);
  assert(!final_set_transition_advance(&t,4000)); assert(t.advance_count==1);
  final_set_transition_cancel(&t); assert(t.phase==FINAL_SET_IDLE); assert(!final_set_transition_advance(&t,5000));
}
int main(void) { run_case(2,4,5); run_case(2,4,2); run_case(2,0,5); return 0; }
