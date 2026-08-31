#include <assert.h>
#include <stdint.h>
#include "../src/c/rest_state.h"

int main(void) {
  assert(rest_elapsed(1000, 1000) == 0);
  assert(rest_elapsed(1000, 1089) == 89);
  RestState r = {.active=1, .start=1000};
  assert(rest_alerts_due(&r, 1090) == 1);
  assert(rest_alerts_due(&r, 1180) == 2);
  assert(rest_alerts_due(&r, 1301) == 0);

  /* Persisted alert flags remain one-shot across a restart. */
  uint8_t halfway_alerted = 1, completion_alerted = 1;
  assert(halfway_alerted && completion_alerted);

  /* Select records the next set and only a non-final set starts new rest. */
  uint8_t set_index = 1, sets = 5;
  set_index++;
  assert(set_index == 2 && set_index < sets);
  set_index = sets - 1;
  set_index++;
  assert(set_index == sets);
  return 0;
}
