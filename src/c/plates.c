#include "plates.h"
#include <stdio.h>
#include <string.h>

Weight weight_from_quarters(int32_t quarters) { return quarters; }
bool weight_valid(Weight weight) { return weight >= WEIGHT_LB(45) && weight <= WEIGHT_LB(1000); }
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

bool plate_inventory_valid(const PlateInventory *i) {
  if (!i || i->count != PLATE_MAX_SIZES || i->bar_weight != WEIGHT_LB(45)) return false;
  for (size_t n = 0; n < PLATE_MAX_SIZES; n++)
    if (i->plates[n].count_per_side > 2 || i->plates[n].size <= 0) return false;
  return true;
}

PlateInventory plate_inventory_from_counts(const PlateCounts counts) {
  PlateInventory i = plate_inventory_default();
  if (counts) for (size_t n = 0; n < PLATE_MAX_SIZES; n++) {
    i.plates[n].count_per_side = counts[n] <= 2 ? counts[n] : i.plates[n].count_per_side;
    i.plates[n].enabled = i.plates[n].count_per_side != 0;
  }
  return i;
}

void plate_inventory_counts(const PlateInventory *i, PlateCounts counts) {
  if (!counts) return;
  for (size_t n = 0; n < PLATE_MAX_SIZES; n++) counts[n] = i && i->plates[n].count_per_side <= 2 ? i->plates[n].count_per_side : 0;
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
  if (best.requested_total > WEIGHT_LB(1000)) best.requested_total = WEIGHT_LB(1000);
  best.actual_total = i->bar_weight;
  cur.requested_total = best.requested_total; cur.actual_total = 0;
  Weight target = (best.requested_total - i->bar_weight) / 2;
  (void)target;
  search(0, target, best.requested_total, i, &best, &cur);
  best.exact = best.actual_total == requested;
  return best;
}

Weight normalize_weight_down(Weight requested, const PlateInventory *i) {
  if (!i) return WEIGHT_LB(45);
  return calculate_plate_load(requested, i).actual_total;
}

Weight next_achievable_total(Weight current, const PlateInventory *i) {
  if (!i) return WEIGHT_LB(1000);
  for (Weight candidate = current + 1; candidate <= WEIGHT_LB(1000); candidate++)
    if (calculate_plate_load(candidate, i).actual_total == candidate) return candidate;
  return current > WEIGHT_LB(1000) ? WEIGHT_LB(1000) : current;
}

Weight previous_achievable_total(Weight current, const PlateInventory *i) {
  if (!i) return WEIGHT_LB(45);
  if (current > WEIGHT_LB(1000)) current = WEIGHT_LB(1000);
  for (Weight candidate = current - 1; candidate >= WEIGHT_LB(45); candidate--)
    if (calculate_plate_load(candidate, i).actual_total == candidate) return candidate;
  return WEIGHT_LB(45);
}

size_t format_plate_side(const PlateInventory *i, const PlateLoad *l, char *b, size_t z) {
  return format_plate_loading(i, l, b, z);
}

size_t format_plate_loading(const PlateInventory *i, const PlateLoad *l, char *b, size_t z) {
  size_t used = 0; bool first = true;
  if (!b || !z) return 0;
  b[0] = 0;
  for (size_t n = 0; i && n < i->count; n++) if (l->counts[n]) {
    char w[16]; int whole = i->plates[n].size / 4; int fraction = i->plates[n].size % 4;
    if (!fraction) snprintf(w, sizeof w, "%d", whole);
    else if (fraction == 2) snprintf(w, sizeof w, "%d.5", whole);
    else snprintf(w, sizeof w, "%d.%d5", whole, fraction == 1 ? 2 : 7);
    char item[24]; snprintf(item, sizeof item, "%s%s%s", first ? "" : "+", l->counts[n] > 1 ? "2x" : "", w);
    size_t len = strlen(item); if (used + len >= z) break;
    memcpy(b + used, item, len); used += len; b[used] = 0; first = false;
  }
  if (first) snprintf(b, z, "empty");
  return strlen(b);
}
