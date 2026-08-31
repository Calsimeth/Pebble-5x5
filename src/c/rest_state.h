#ifndef REST_STATE_H
#define REST_STATE_H
#include <stdint.h>
typedef struct { uint8_t active, halfway_alerted, completion_alerted; int32_t start; uint32_t elapsed; } RestState;
uint32_t rest_elapsed(int32_t start, int32_t now);
int rest_valid(const RestState *r, int32_t now);
int rest_alerts_due(RestState *r, int32_t now);
void rest_begin(RestState *r, int32_t start);
void rest_reset(RestState *r);
#endif
