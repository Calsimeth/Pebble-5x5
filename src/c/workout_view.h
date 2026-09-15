#ifndef WORKOUT_VIEW_H
#define WORKOUT_VIEW_H

#include <stdbool.h>
#include <stdint.h>

typedef struct { int16_t x, y, diameter, gap; } WorkoutCircleLayout;
typedef struct { uint8_t set_count, completed_count, selected_reps; bool confirmation, handoff; uint8_t completed_reps[5]; } WorkoutViewModel;
typedef struct { int16_t x, y, width, height; } WorkoutViewFrame;
typedef struct { WorkoutViewFrame next_exercise, select_instruction; } WorkoutHandoffLayout;

bool workout_view_model_valid(const WorkoutViewModel *model);
WorkoutCircleLayout workout_circle_layout(uint16_t width, uint16_t height, uint8_t set_count);
bool workout_view_rep_selected_valid(uint8_t reps);
uint8_t workout_view_rep_up(uint8_t reps);
uint8_t workout_view_rep_down(uint8_t reps);
uint8_t workout_view_display_reps(const WorkoutViewModel *model, uint8_t circle);
bool workout_view_confirmation_visible(const WorkoutViewModel *model);
bool workout_view_handoff_visible(const WorkoutViewModel *model);
bool workout_view_final_transition_ready(bool pending, uint32_t elapsed_ms);
WorkoutViewFrame workout_view_clock_frame(uint16_t width, uint16_t height, bool round);
WorkoutViewFrame workout_view_warmup_frame(uint16_t width, uint16_t height, bool mono);
WorkoutHandoffLayout workout_view_handoff_layout(uint16_t width, uint16_t height, bool round);

#endif
