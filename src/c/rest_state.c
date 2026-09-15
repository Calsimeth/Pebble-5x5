#include "rest_state.h"
uint32_t rest_elapsed(int32_t start, int32_t now) { return start > 0 && now >= start ? (uint32_t)(now-start) : 0; }
int rest_valid(const RestState *r, int32_t now) { return r && r->active && r->start > 0 && now >= r->start; }
int rest_alerts_due(RestState *r, int32_t now) {
  if (!rest_valid(r, now)) return 0;
  r->elapsed = rest_elapsed(r->start, now); int due = 0;
  if (!r->halfway_alerted && r->elapsed >= 90) { r->halfway_alerted=1; due |= 1; }
  if (!r->completion_alerted && r->elapsed >= 180) { r->completion_alerted=1; due |= 2; }
  if (!r->five_minute_alerted && r->elapsed >= 300) { r->five_minute_alerted=1; due |= 4; }
  return due;
}
void rest_begin(RestState *r, int32_t start) { if (!r) return; *r=(RestState){.active=1,.start=start}; }
void rest_reset(RestState *r) { if (r) *r=(RestState){0}; }
