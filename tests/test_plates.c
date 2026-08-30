#include <assert.h>
#include <string.h>
#include "../src/c/plates.h"
int main(void) {
  PlateInventory i = plate_inventory_default(); PlateLoad l; char b[64]; PlateCounts counts;
  assert(plate_inventory_valid(&i)); assert(i.plates[1].count_per_side == 0 && i.plates[3].count_per_side == 0);
  plate_inventory_counts(&i, counts); counts[1] = 3; counts[3] = 1; i = plate_inventory_from_counts(counts);
  assert(i.plates[1].count_per_side == 0 && i.plates[3].count_per_side == 1);
  l = calculate_plate_load(WEIGHT_LB(45), &i); assert(l.exact && l.total_plates_per_side == 0);
  l = calculate_plate_load(WEIGHT_LB(65), &i); assert(l.exact); format_plate_side(&i,&l,b,sizeof b); assert(strcmp(b,"10") == 0);
  l = calculate_plate_load(WEIGHT_LB(145), &i); assert(l.exact); format_plate_side(&i,&l,b,sizeof b); assert(strcmp(b,"45+5") == 0);
  l = calculate_plate_load(WEIGHT_LB(95), &i); assert(l.exact);
  l = calculate_plate_load(WEIGHT_LB(210), &i); assert(l.exact && l.actual_total == WEIGHT_LB(210));
  l = calculate_plate_load(WEIGHT_LB(250), &i); assert(l.exact);
  i.plates[1].enabled = true; i.plates[1].count_per_side = 2; l = calculate_plate_load(WEIGHT_LB(210), &i); assert(l.exact);
  i.plates[3].enabled = false; i.plates[3].count_per_side = 0; l = calculate_plate_load(WEIGHT_LB(44), &i); assert(!l.exact && l.actual_total == WEIGHT_LB(45));
  assert(weight_format(WEIGHT_LB(102)+2,b,sizeof b) && strcmp(b,"102.5 lb") == 0); assert(strlen(b) <= 20);
  assert(next_achievable_total(WEIGHT_LB(45), &i) > WEIGHT_LB(45));
  assert(previous_achievable_total(WEIGHT_LB(46), &i) == WEIGHT_LB(45));
  assert(normalize_weight_down(WEIGHT_LB(44), &i) == WEIGHT_LB(45));
  i = plate_inventory_default(); l = calculate_plate_load(WEIGHT_LB(145), &i); format_plate_side(&i, &l, b, sizeof b);
  assert(strcmp(b, "45+5") == 0 && strlen(b) <= 20);
  return 0;
}
