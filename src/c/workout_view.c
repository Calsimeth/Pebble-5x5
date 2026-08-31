#include "workout_view.h"

bool workout_view_rep_selected_valid(uint8_t reps) { return reps <= 5; }
uint8_t workout_view_rep_up(uint8_t reps) { return reps >= 5 ? 5 : reps + 1; }
uint8_t workout_view_rep_down(uint8_t reps) { return reps == 0 ? 0 : reps - 1; }
uint8_t workout_view_display_reps(const WorkoutViewModel *m, uint8_t circle) {
  if (!m || circle >= m->set_count) return 0;
  if (circle < m->completed_count) return m->completed_reps[circle];
  return circle == m->completed_count ? m->selected_reps : 5;
}

bool workout_view_model_valid(const WorkoutViewModel *model) {
  if (!model || model->set_count == 0 || model->set_count > 5 || model->completed_count > model->set_count || !workout_view_rep_selected_valid(model->selected_reps)) return false;
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
  layout.y = ((int16_t)height - diameter) / 2;
  return layout;
}
