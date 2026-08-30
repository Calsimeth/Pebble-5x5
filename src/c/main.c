#include <pebble.h>

static Window *s_window;
static TextLayer *s_title_layer;
static TextLayer *s_exercise_layer;
static TextLayer *s_hint_layer;
static int s_selected;

static const char *EXERCISES[] = {"Squat 5x5", "Bench 5x5", "Row 5x5"};

static void update_selection(void) {
  static char selection[64];
  snprintf(selection, sizeof(selection), "%c %s\n%c %s\n%c %s",
           s_selected == 0 ? '>' : ' ', EXERCISES[0],
           s_selected == 1 ? '>' : ' ', EXERCISES[1],
           s_selected == 2 ? '>' : ' ', EXERCISES[2]);
  text_layer_set_text(s_exercise_layer, selection);
}

static void select_click(ClickRecognizerRef recognizer, void *context) {
  s_selected = (s_selected + 1) % 3;
  update_selection();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  const int round_adjust = PBL_IF_ROUND_ELSE(8, 0);

  s_title_layer = text_layer_create(GRect(0, 14 + round_adjust, bounds.size.w, 34));
  text_layer_set_text(s_title_layer, "Workout A");
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_color(s_title_layer, GColorWhite);
  text_layer_set_background_color(s_title_layer, GColorClear);
  layer_add_child(root, text_layer_get_layer(s_title_layer));

  s_exercise_layer = text_layer_create(GRect(0, 58 + round_adjust, bounds.size.w, 84));
  text_layer_set_text_alignment(s_exercise_layer, GTextAlignmentCenter);
  text_layer_set_font(s_exercise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(s_exercise_layer, GColorWhite);
  text_layer_set_background_color(s_exercise_layer, GColorClear);
  layer_add_child(root, text_layer_get_layer(s_exercise_layer));

  s_hint_layer = text_layer_create(GRect(0, bounds.size.h - 30, bounds.size.w, 24));
  text_layer_set_text(s_hint_layer, "Select: next");
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  text_layer_set_font(s_hint_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_color(s_hint_layer, GColorWhite);
  text_layer_set_background_color(s_hint_layer, GColorClear);
  layer_add_child(root, text_layer_get_layer(s_hint_layer));
  update_selection();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_exercise_layer);
  text_layer_destroy(s_hint_layer);
}

static void init(void) {
  s_window = window_create();
  window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));
  window_set_click_config_provider(s_window, click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load, .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

static void deinit(void) { window_destroy(s_window); }

int main(void) {
  init();
  app_event_loop();
  deinit();
}
