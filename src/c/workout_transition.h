#ifndef WORKOUT_TRANSITION_H
#define WORKOUT_TRANSITION_H

#include <stdbool.h>
#include <stdint.h>

typedef enum { FINAL_SET_IDLE, FINAL_SET_VISIBLE, FINAL_SET_ADVANCED } FinalSetPhase;
typedef enum { FINAL_SET_RECORDED, FINAL_SET_VISIBLE_LOG, FINAL_SET_ADVANCE_LOG, FINAL_SET_DUPLICATE_IGNORED } FinalSetEvent;
typedef void (*FinalSetLogFn)(FinalSetEvent event, uint8_t exercise, uint8_t set, uint8_t reps, uint32_t elapsed_ms, void *context);

typedef struct {
  FinalSetPhase phase;
  uint8_t exercise, set, reps;
  uint32_t visible_at_ms;
  uint8_t advance_count;
  FinalSetLogFn log;
  void *log_context;
} FinalSetTransition;

void final_set_transition_init(FinalSetTransition *transition, FinalSetLogFn log, void *context);
bool final_set_transition_begin(FinalSetTransition *transition, uint8_t exercise, uint8_t set, uint8_t reps, bool persisted, uint32_t now_ms);
bool final_set_transition_select(FinalSetTransition *transition);
bool final_set_transition_advance(FinalSetTransition *transition, uint32_t now_ms);
void final_set_transition_cancel(FinalSetTransition *transition);
bool final_set_transition_visible(const FinalSetTransition *transition);

#endif
