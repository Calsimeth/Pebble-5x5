#include "plates.h"
#include <stdio.h>
#include <string.h>

Weight weight_from_quarters(int32_t quarters) { return quarters; }
bool weight_valid(Weight weight) { return weight >= 0; }
int weight_compare(Weight a, Weight b) { return (a > b) - (a < b); }

size_t weight_format(Weight weight, char *buffer, size_t size) {
  if (!buffer || !size) return 0;
  if (weight < 0) weight = 0;
  int whole = weight / 4, fraction = weight % 4;
  if (!fraction) snprintf(buffer, size, "%d lb", whole);
  else if (fraction == 1) snprintf(buffer, size, "%d.25 lb", whole);
  else if (fraction == 2) snprintf(buffer, size, "%d.5 lb", whole);
  else snprintf(buffer, size, "%d.75 lb", whole);
  return strlen(buffer);
}

PlateInventory plate_inventory_default(void) {
  PlateInventory i = { .count = 7, .bar_weight = WEIGHT_LB(45), .plates = {
    {WEIGHT_LB(45), 2, true}, {WEIGHT_LB(35), 0, false}, {WEIGHT_LB(25), 2, true},
    {WEIGHT_LB(15), 0, false}, {WEIGHT_LB(10), 2, true}, {WEIGHT_LB(5), 2, true},
    {WEIGHT_LB(2) + 2, 2, true}
  }};
  return i;
}

static bool better(const PlateLoad *a, const PlateLoad *b, const PlateInventory *i) {
  if (a->actual_total != b->actual_total) return a->actual_total > b->actual_total;
  if (a->total_plates_per_side != b->total_plates_per_side)
    return a->total_plates_per_side < b->total_plates_per_side;
  for (size_t n = 0; n < i->count; n++) if (a->counts[n] != b->counts[n]) return a->counts[n] > b->counts[n];
  size_t ad = 0, bd = 0;
  for (size_t n = 0; n < i->count; n++) { ad += a->counts[n] != 0; bd += b->counts[n] != 0; }
  return ad < bd;
}

static void search(size_t n, Weight remaining, Weight requested, const PlateInventory *i, PlateLoad *best, PlateLoad *cur) {
  if (n == i->count) {
    Weight total = i->bar_weight + 2 * cur->actual_total;
    if (total <= requested) {
      PlateLoad candidate = *cur;
      candidate.actual_total = total;
      if (better(&candidate, best, i)) *best = candidate;
    }
    return;
  }
  Plate p = i->plates[n];
  uint8_t max = (p.enabled && p.size > 0) ? p.count_per_side : 0;
  for (uint8_t c = 0; c <= max; c++) {
    cur->counts[n] = c; cur->total_plates_per_side += c; cur->actual_total += c * p.size;
    search(n + 1, remaining - c * p.size, requested, i, best, cur);
    cur->actual_total -= c * p.size; cur->total_plates_per_side -= c;
  }
  cur->counts[n] = 0;
}

PlateLoad calculate_plate_load(Weight requested, const PlateInventory *i) {
  PlateLoad best = {0}, cur = {0};
  if (!i) return best;
  best.requested_total = requested < i->bar_weight ? i->bar_weight : requested;
  best.actual_total = i->bar_weight;
  cur.requested_total = best.requested_total; cur.actual_total = 0;
  Weight target = (best.requested_total - i->bar_weight) / 2;
  (void)target;
  search(0, target, best.requested_total, i, &best, &cur);
  best.exact = best.actual_total == requested;
  return best;
}

size_t format_plate_side(const PlateInventory *i, const PlateLoad *l, char *b, size_t z) {
  size_t used = 0; bool first = true;
  for (size_t n = 0; n < i->count; n++) for (uint8_t c = 0; c < l->counts[n]; c++) {
    char w[16]; weight_format(i->plates[n].size, w, sizeof w);
    char item[20]; snprintf(item, sizeof item, "%s%s", first ? "" : "+", w);
    size_t len = strlen(item); if (used + len + 1 >= z) return used;
    memcpy(b + used, item, len); used += len; b[used] = 0; first = false;
  }
  if (first && z) snprintf(b, z, "empty");
  return strlen(b);
}
