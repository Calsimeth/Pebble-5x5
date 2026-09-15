#ifndef REST_STATE_H
#define REST_STATE_H
#include <stdint.h>
typedef struct { uint8_t active, halfway_alerted, completion_alerted, five_minute_alerted; int32_t start; uint32_t elapsed; } RestState;
/* Schema 10 already persisted rest_end but no longer used it. Reserve an
 * impossible timestamp there so the core layout stays byte-for-byte stable. */
#define REST_FIVE_MINUTE_ALERTED_END INT32_MIN
int rest_five_minute_alerted_from_end(int32_t rest_end);
int32_t rest_end_with_five_minute_alert(int alerted);
uint32_t rest_elapsed(int32_t start, int32_t now);
int rest_valid(const RestState *r, int32_t now);
int rest_alerts_due(RestState *r, int32_t now);
void rest_begin(RestState *r, int32_t start);
void rest_reset(RestState *r);
#endif
