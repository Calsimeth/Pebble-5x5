#ifndef PLATES_H
#define PLATES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int32_t Weight;
#define WEIGHT_QUARTER_LB 1
#define WEIGHT_LB(lb) ((Weight)((lb) * 4))

typedef struct {
  Weight size;
  uint8_t count_per_side;
  bool enabled;
} Plate;

#define PLATE_MAX_SIZES 7
typedef struct { Plate plates[PLATE_MAX_SIZES]; size_t count; Weight bar_weight; } PlateInventory;

typedef struct {
  bool exact;
  Weight requested_total;
  Weight actual_total;
  uint8_t counts[PLATE_MAX_SIZES];
  uint8_t total_plates_per_side;
} PlateLoad;

Weight weight_from_quarters(int32_t quarters);
bool weight_valid(Weight weight);
int weight_compare(Weight a, Weight b);
size_t weight_format(Weight weight, char *buffer, size_t buffer_size);
PlateInventory plate_inventory_default(void);
PlateLoad calculate_plate_load(Weight requested_total, const PlateInventory *inventory);
size_t format_plate_side(const PlateInventory *inventory, const PlateLoad *load,
                         char *buffer, size_t buffer_size);

#endif
