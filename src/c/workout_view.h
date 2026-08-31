#ifndef WORKOUT_VIEW_H
#define WORKOUT_VIEW_H

#include <stdbool.h>
#include <stdint.h>

typedef struct { int16_t x, y, diameter, gap; } WorkoutCircleLayout;
typedef struct { uint8_t set_count, selected_reps; uint8_t completed_reps[5]; } WorkoutViewModel;

bool workout_view_model_valid(const WorkoutViewModel *model);
WorkoutCircleLayout workout_circle_layout(uint16_t width, uint16_t height, uint8_t set_count);
bool workout_view_rep_selected_valid(uint8_t reps);
uint8_t workout_view_rep_up(uint8_t reps);
uint8_t workout_view_rep_down(uint8_t reps);

#endif
