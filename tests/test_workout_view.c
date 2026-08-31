#include <assert.h>
#include "../src/c/workout_view.h"

int main(void) {
  WorkoutViewModel m = {.set_count=5, .selected_reps=5, .completed_reps={0,1,2,3,4}};
  assert(workout_view_model_valid(&m)); assert(!workout_view_rep_selected_valid(6));
  assert(workout_view_rep_up(4)==5 && workout_view_rep_up(5)==5);
  assert(workout_view_rep_down(1)==0 && workout_view_rep_down(0)==0);
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
