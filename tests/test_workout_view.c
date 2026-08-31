#include <assert.h>
#include "../src/c/workout_view.h"

int main(void) {
  WorkoutViewModel m = {.set_count=5, .selected_reps=5, .completed_reps={0,1,2,3,4}};
  assert(workout_view_model_valid(&m)); assert(!workout_view_rep_selected_valid(6));
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
  return 0;
}
