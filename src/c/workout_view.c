#include "workout_view.h"

bool workout_view_rep_selected_valid(uint8_t reps) { return reps <= 5; }
uint8_t workout_view_rep_up(uint8_t reps) { return reps >= 5 ? 5 : reps + 1; }
uint8_t workout_view_rep_down(uint8_t reps) { return reps == 0 ? 0 : reps - 1; }

bool workout_view_model_valid(const WorkoutViewModel *model) {
  if (!model || model->set_count == 0 || model->set_count > 5 || !workout_view_rep_selected_valid(model->selected_reps)) return false;
  for (uint8_t n = 0; n < model->set_count; n++) if (model->completed_reps[n] > 5) return false;
  return true;
}

WorkoutCircleLayout workout_circle_layout(uint16_t width, uint16_t height, uint8_t set_count) {
  WorkoutCircleLayout layout = {0};
  if (!set_count) return layout;
  int16_t available = (int16_t)width - 16;
  int16_t diameter = available / (set_count * 2 - 1);
  if (diameter > 34) diameter = 34;
  if (diameter < 12) diameter = 12;
  layout.diameter = diameter;
  layout.gap = set_count == 1 ? 0 : (available - diameter * set_count) / (set_count - 1);
  layout.x = (width - (diameter * set_count + layout.gap * (set_count - 1))) / 2;
  layout.y = height > 110 ? 78 : height / 2;
  return layout;
}
