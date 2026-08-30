#include <assert.h>
#include <string.h>
#include "../src/c/plates.h"
int main(void) {
  PlateInventory i = plate_inventory_default(); PlateLoad l; char b[64];
  l = calculate_plate_load(WEIGHT_LB(45), &i); assert(l.exact && l.total_plates_per_side == 0);
  l = calculate_plate_load(WEIGHT_LB(65), &i); assert(l.exact); format_plate_side(&i,&l,b,sizeof b); assert(strcmp(b,"10 lb") == 0);
  l = calculate_plate_load(WEIGHT_LB(145), &i); assert(l.exact); format_plate_side(&i,&l,b,sizeof b); assert(strcmp(b,"45 lb+5 lb") == 0);
  l = calculate_plate_load(WEIGHT_LB(95), &i); assert(l.exact);
  l = calculate_plate_load(WEIGHT_LB(210), &i); assert(l.exact && l.actual_total == WEIGHT_LB(210));
  l = calculate_plate_load(WEIGHT_LB(250), &i); assert(l.exact);
  i.plates[1].enabled = true; i.plates[1].count_per_side = 2; l = calculate_plate_load(WEIGHT_LB(210), &i); assert(l.exact);
  i.plates[3].enabled = false; i.plates[3].count_per_side = 0; l = calculate_plate_load(WEIGHT_LB(44), &i); assert(!l.exact && l.actual_total == WEIGHT_LB(45));
  assert(weight_format(WEIGHT_LB(102)+2,b,sizeof b) && strcmp(b,"102.5 lb") == 0); assert(strlen(b) <= 20);
  return 0;
}
