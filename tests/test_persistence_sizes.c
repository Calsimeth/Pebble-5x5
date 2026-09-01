#include <assert.h>
#include <stdio.h>
#include "../src/c/workout_completion.h"

/* Pebble persist values are limited to 256 bytes. Keep this executable as a
 * schema guard: changing either persisted region beyond the device limit is a
 * build/test failure, not a runtime surprise. */
int main(void) {
  const size_t limit = 256;
  printf("monolithic=%zu core=%zu sync=%zu metadata=%zu limit=%zu\n",
         sizeof(PersistedState), sizeof(PersistedCoreState),
         sizeof(PersistedSyncState), sizeof(uint32_t), limit);
  assert(sizeof(PersistedCoreState) <= limit);
  assert(sizeof(PersistedSyncState) <= limit);
  assert(sizeof(uint32_t) <= limit);
  return 0;
}
