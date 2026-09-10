#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct { uint8_t version, head, count; } Meta;
typedef struct { uint8_t code; uint32_t a, b; } Event;
enum { DBG_META = 30, DBG_DATA = 31, DBG_CAP = 48, MAX_BYTES = 768 };

static int valid_meta(const Meta *m) {
  return m->version == 1 && m->head < DBG_CAP && m->count <= DBG_CAP;
}

int main(void) {
  Meta m = { 1, 0, 0 }, corrupt = { 2, 0, 0 };
  Event ring[DBG_CAP] = { 0 }, e = { 7, 123, 456 };
  assert(sizeof(Event) <= 12);
  assert(3 + DBG_CAP * sizeof(Event) <= MAX_BYTES);
  assert(DBG_META != 1 && DBG_META != 2 && DBG_META != 10 && DBG_META != 11 && DBG_META != 12 && DBG_META != 13 && DBG_META != 14);
  assert(DBG_DATA != 1 && DBG_DATA != 2 && DBG_DATA != 10 && DBG_DATA != 11 && DBG_DATA != 12 && DBG_DATA != 13 && DBG_DATA != 14);
  assert(valid_meta(&m) && !valid_meta(&corrupt));
  for (unsigned n = 0; n < DBG_CAP + 5; n++) {
    ring[m.head] = e;
    ring[m.head].a = n;
    m.head = (uint8_t)((m.head + 1) % DBG_CAP);
    if (m.count < DBG_CAP) m.count++;
  }
  assert(m.count == DBG_CAP && m.head == 5);
  assert(ring[4].a == DBG_CAP + 4);
  assert(ring[5].a == 5);
  Meta before = m;
  /* A failed metadata write leaves the prior committed cursor usable. */
  m.head = 99; m = before;
  assert(valid_meta(&m) && m.head == before.head && m.count == before.count);
  char decoded[32];
  int n = snprintf(decoded, sizeof decoded, "%u:%lu/%lu", ring[4].code,
                   (unsigned long)ring[4].a, (unsigned long)ring[4].b);
  assert(n > 0 && strstr(decoded, "7:") == decoded);
  puts("debug diagnostics buffer bounds, wrap, recovery, decoding, and key separation passed");
  return 0;
}
