#include <assert.h>
#include "../src/c/workout_view.h"

int main(void) {
  WorkoutViewModel m = {.set_count=5, .selected_reps=5, .completed_reps={0,1,2,3,4}};
  assert(workout_view_model_valid(&m)); assert(!workout_view_rep_selected_valid(6));
  m.confirmation=true; assert(workout_view_confirmation_visible(&m)); m.confirmation=false;
  assert(workout_view_rep_up(4)==5 && workout_view_rep_up(5)==5);
  assert(workout_view_rep_down(1)==0 && workout_view_rep_down(0)==0);
  for (uint8_t n=0;n<=5;n++) { WorkoutViewModel current={.set_count=5,.completed_count=0,.selected_reps=n}; assert(workout_view_display_reps(&current,0)==n); }
  WorkoutViewModel m2={.set_count=5,.completed_count=2,.completed_reps={3,0},.selected_reps=2};
  assert(workout_view_display_reps(&m2,0)==3 && workout_view_display_reps(&m2,1)==0 && workout_view_display_reps(&m2,2)==2 && workout_view_display_reps(&m2,3)==5);
  WorkoutViewModel dead={.set_count=1,.completed_count=0,.selected_reps=2}; assert(workout_view_display_reps(&dead,0)==2); dead.completed_count=1; dead.completed_reps[0]=3; assert(workout_view_display_reps(&dead,0)==3);
  assert(workout_view_model_valid(&(WorkoutViewModel){.set_count=1,.selected_reps=0}));
  WorkoutCircleLayout a=workout_circle_layout(144,168,5), b=workout_circle_layout(144,168,1);
  assert(a.diameter>0 && a.x>=0 && a.x+a.diameter*5+a.gap*4<=144);
  assert(b.diameter>0 && b.x>=0 && b.x+b.diameter<=144);
  a=workout_circle_layout(144,168,5); b=workout_circle_layout(180,180,5);
  assert(a.x>=0 && b.x>=0 && a.x+a.diameter*5+a.gap*4<=144 && b.x+b.diameter*5+b.gap*4<=180);
  a=workout_circle_layout(144,168,5); b=workout_circle_layout(180,180,1);
  assert(a.y>=0 && b.y>=0);
  WorkoutViewModel final_a={.set_count=5,.completed_count=5,.completed_reps={5,5,5,5,5}};
  WorkoutViewModel final_fail={.set_count=5,.completed_count=5,.completed_reps={5,5,5,5,2}};
  WorkoutViewModel deadlift={.set_count=1,.completed_count=1,.completed_reps={3}};
  assert(workout_view_display_reps(&final_a,4)==5 && workout_view_display_reps(&final_fail,4)==2 && workout_view_display_reps(&deadlift,0)==3);
  assert(!workout_view_final_transition_ready(true,649) && workout_view_final_transition_ready(true,650));
  WorkoutViewFrame clock=workout_view_clock_frame(180,180,true);
  assert(clock.x>=14 && clock.y>=14 && clock.x+clock.width<=166 && clock.y+clock.height<=166);
  clock=workout_view_clock_frame(144,168,false); assert(clock.x>=0 && clock.y>=0 && clock.x+clock.width<=144 && clock.y+clock.height<=168);
  WorkoutViewFrame warm=workout_view_warmup_frame(144,168,true);
  assert(warm.y+warm.height<=168-20);
  return 0;
}
