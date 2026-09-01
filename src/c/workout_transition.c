#include "workout_transition.h"

static void emit(FinalSetTransition *t, FinalSetEvent event, uint32_t elapsed) {
  if (t->log) t->log(event, t->exercise, t->set, t->reps, elapsed, t->log_context);
}

void final_set_transition_init(FinalSetTransition *t, FinalSetLogFn log, void *context) {
  if (!t) return;
  *t = (FinalSetTransition){.log=log, .log_context=context};
}

bool final_set_transition_begin(FinalSetTransition *t, uint8_t exercise, uint8_t set, uint8_t reps, bool persisted, uint32_t now_ms) {
  if (!t || !persisted || t->phase != FINAL_SET_IDLE) return false;
  t->exercise=exercise; t->set=set; t->reps=reps; t->visible_at_ms=now_ms; t->advance_count=0; t->phase=FINAL_SET_VISIBLE;
  emit(t, FINAL_SET_RECORDED, 0); emit(t, FINAL_SET_VISIBLE_LOG, 0); return true;
}

bool final_set_transition_select(FinalSetTransition *t) {
  if (!t || t->phase != FINAL_SET_VISIBLE) return false;
  emit(t, FINAL_SET_DUPLICATE_IGNORED, 0); return true;
}

bool final_set_transition_advance(FinalSetTransition *t, uint32_t now_ms) {
  if (!t || t->phase != FINAL_SET_VISIBLE || now_ms - t->visible_at_ms < 2000) return false;
  t->phase=FINAL_SET_ADVANCED; t->advance_count=1; emit(t, FINAL_SET_ADVANCE_LOG, now_ms - t->visible_at_ms); return true;
}

void final_set_transition_cancel(FinalSetTransition *t) { if (t) *t=(FinalSetTransition){.log=t->log,.log_context=t->log_context}; }
bool final_set_transition_visible(const FinalSetTransition *t) { return t && t->phase == FINAL_SET_VISIBLE; }
