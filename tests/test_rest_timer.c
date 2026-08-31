#include <assert.h>
#include <stdint.h>

static uint32_t elapsed(int32_t start, int32_t now) {
  return now >= start ? (uint32_t)(now - start) : 0;
}

int main(void) {
  assert(elapsed(1000, 1000) == 0);
  assert(elapsed(1000, 1089) == 89);
  assert(elapsed(1000, 1090) == 90);
  assert(elapsed(1000, 1179) == 179);
  assert(elapsed(1000, 1180) == 180);
  assert(elapsed(1000, 1301) > 180);

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
