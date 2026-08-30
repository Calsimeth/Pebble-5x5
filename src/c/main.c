#include <pebble.h>

enum {
  STORAGE_KEY_STATE = 1,
  STORAGE_SCHEMA_VERSION = 2,
  REST_SECONDS = 180,
};

typedef enum { WORKOUT_A = 0, WORKOUT_B = 1 } WorkoutType;

typedef struct {
  const char *name;
  uint8_t sets;
} ExerciseDefinition;

typedef struct {
  uint8_t schema_version;
  uint8_t next_workout;
  uint8_t active;
  uint8_t active_workout;
  uint8_t exercise_index;
  uint8_t set_index;
  uint8_t rest_active;
  int32_t rest_start;
  int32_t rest_end;
  uint8_t halfway_alerted;
} PersistedState;

typedef struct {
  uint8_t schema_version;
  uint8_t next_workout;
  uint8_t active;
  uint8_t active_workout;
  uint8_t exercise_index;
  uint8_t set_index;
} LegacyState;

static const ExerciseDefinition WORKOUTS[2][3] = {
  {{"Squat", 5}, {"Bench", 5}, {"Row", 5}},
  {{"Squat", 5}, {"OHP", 5}, {"Deadlift", 1}},
};

static Window *s_window;
static TextLayer *s_title_layer;
static TextLayer *s_exercise_layer;
static TextLayer *s_hint_layer;
static PersistedState s_state;
static bool s_saved;
static bool s_confirm_abandon;
static char s_exercise_text[64];
static char s_hint_text[32];
static AppTimer *s_rest_timer;

// Black canvas, white content, and red accents (white on Flint).
static GColor palette_background(void) { return GColorBlack; }
static GColor palette_primary_text(void) { return GColorWhite; }
static GColor palette_accent(void) { return PBL_IF_COLOR_ELSE(GColorRed, GColorWhite); }

static void save_state(void);
static void update_display(void);

static void stop_rest_services(void) {
  if (s_rest_timer) {
    app_timer_cancel(s_rest_timer);
    s_rest_timer = NULL;
  }
  tick_timer_service_unsubscribe();
}

static void clear_rest(void) {
  stop_rest_services();
  s_state.rest_active = 0;
  s_state.rest_start = 0;
  s_state.rest_end = 0;
  s_state.halfway_alerted = 0;
}

static bool rest_values_valid(time_t now) {
  return s_state.rest_active && s_state.rest_start > 0 &&
      s_state.rest_end > s_state.rest_start &&
      s_state.rest_end - s_state.rest_start <= REST_SECONDS &&
      s_state.rest_end >= now;
}

static void finish_rest(void *context);

static void rest_tick(struct tm *tick_time, TimeUnits units_changed) {
  time_t now = time(NULL);
  if (!rest_values_valid(now)) {
    clear_rest();
    save_state();
    update_display();
    return;
  }
  if (!s_state.halfway_alerted && now >= s_state.rest_start + REST_SECONDS / 2) {
    vibes_short_pulse();
    s_state.halfway_alerted = 1;
    save_state();
  }
  update_display();
}

static void start_rest_services(void) {
  stop_rest_services();
  tick_timer_service_subscribe(SECOND_UNIT, rest_tick);
  time_t remaining = s_state.rest_end - time(NULL);
  s_rest_timer = app_timer_register((uint32_t)remaining * 1000, finish_rest, NULL);
}

static void finish_rest(void *context) {
  if (!s_state.rest_active) return;
  vibes_double_pulse();
  clear_rest();
  save_state();
  update_display();
}

static const char *workout_name(WorkoutType workout) {
  return workout == WORKOUT_B ? "Workout B" : "Workout A";
}

static void save_state(void) {
  s_state.schema_version = STORAGE_SCHEMA_VERSION;
  persist_write_data(STORAGE_KEY_STATE, &s_state, sizeof(s_state));
}

static void initialize_state(void) {
  memset(&s_state, 0, sizeof(s_state));
  s_state.schema_version = STORAGE_SCHEMA_VERSION;
  s_state.next_workout = WORKOUT_A;
  save_state();
}

static void load_state(void) {
  uint8_t version = 0;
  if (persist_exists(STORAGE_KEY_STATE)) persist_read_data(STORAGE_KEY_STATE, &version, sizeof(version));
  if (version == 1) {
    LegacyState old;
    if (persist_read_data(STORAGE_KEY_STATE, &old, sizeof(old)) == sizeof(old) &&
        old.next_workout <= WORKOUT_B && old.active_workout <= WORKOUT_B && old.active <= 1 &&
        (!old.active || (old.exercise_index < 3 && old.set_index < WORKOUTS[old.active_workout][old.exercise_index].sets))) {
      memset(&s_state, 0, sizeof(s_state));
      s_state.next_workout = old.next_workout;
      s_state.active = old.active;
      s_state.active_workout = old.active_workout;
      s_state.exercise_index = old.exercise_index;
      s_state.set_index = old.set_index;
      save_state();
      return;
    }
  }
  if (persist_exists(STORAGE_KEY_STATE) &&
      persist_read_data(STORAGE_KEY_STATE, &s_state, sizeof(s_state)) == sizeof(s_state) &&
      s_state.schema_version == STORAGE_SCHEMA_VERSION &&
      s_state.next_workout <= WORKOUT_B && s_state.active_workout <= WORKOUT_B &&
      (!s_state.active || (s_state.exercise_index < 3 &&
       s_state.set_index < WORKOUTS[s_state.active_workout][s_state.exercise_index].sets)) &&
      s_state.active <= 1 && s_state.rest_active <= 1 && s_state.halfway_alerted <= 1 &&
      (!s_state.rest_active || (s_state.active &&
       s_state.exercise_index < 3 && s_state.set_index < WORKOUTS[s_state.active_workout][s_state.exercise_index].sets))) {
    if (s_state.rest_active) {
      if (rest_values_valid(time(NULL))) start_rest_services();
      else { clear_rest(); save_state(); }
    }
    return;
  }
  initialize_state();
}

static void update_display(void) {
  WorkoutType workout = s_state.active ? s_state.active_workout : s_state.next_workout;

  if (s_saved) {
    text_layer_set_text(s_title_layer, "Workout Saved");
    text_layer_set_text(s_exercise_layer, workout_name(s_state.next_workout));
    text_layer_set_text(s_hint_layer, "Select: start");
    return;
  }

  if (s_state.rest_active) {
    int32_t remaining = s_state.rest_end - (int32_t)time(NULL);
    if (remaining < 0) remaining = 0;
    snprintf(s_exercise_text, sizeof(s_exercise_text), "Rest\n%ld:%02ld\n%s\nSet %d of %d",
             (long)(remaining / 60), (long)(remaining % 60),
             WORKOUTS[s_state.active_workout][s_state.exercise_index].name,
             s_state.set_index + 1, WORKOUTS[s_state.active_workout][s_state.exercise_index].sets);
    text_layer_set_text(s_title_layer, "Rest");
    text_layer_set_text(s_exercise_layer, s_exercise_text);
    text_layer_set_text(s_hint_layer, "Select: skip");
    return;
  }

  text_layer_set_text(s_title_layer, workout_name(workout));
  if (!s_state.active) {
    snprintf(s_exercise_text, sizeof(s_exercise_text), "Next: %s\nNext: %s\nNext: %s",
             WORKOUTS[workout][0].name, WORKOUTS[workout][1].name,
             WORKOUTS[workout][2].name);
    snprintf(s_hint_text, sizeof(s_hint_text), "Select: start");
  } else {
    const ExerciseDefinition *current = &WORKOUTS[workout][s_state.exercise_index];
    snprintf(s_exercise_text, sizeof(s_exercise_text), "%s\nSet %d of %d\n5 reps",
             current->name, s_state.set_index + 1, current->sets);
    snprintf(s_hint_text, sizeof(s_hint_text), s_confirm_abandon ? "Select: abandon" : "Select: complete");
  }
  text_layer_set_text(s_exercise_layer, s_exercise_text);
  text_layer_set_text(s_hint_layer, s_hint_text);
}

static void complete_set(void) {
  const ExerciseDefinition *current = &WORKOUTS[s_state.active_workout][s_state.exercise_index];
  s_state.set_index++;
  bool next_set_same_exercise = s_state.set_index < current->sets;
  if (next_set_same_exercise) {
    time_t now = time(NULL);
    s_state.rest_active = 1;
    s_state.rest_start = (int32_t)now;
    s_state.rest_end = (int32_t)(now + REST_SECONDS);
    s_state.halfway_alerted = 0;
    save_state();
    start_rest_services();
    update_display();
    return;
  }
  if (s_state.set_index >= current->sets) {
    s_state.set_index = 0;
    s_state.exercise_index++;
    if (s_state.exercise_index >= 3) {
      s_state.active = 0;
      s_state.next_workout = s_state.active_workout == WORKOUT_A ? WORKOUT_B : WORKOUT_A;
      s_saved = true;
    }
  }
  save_state();
  update_display();
}

static void select_click(ClickRecognizerRef recognizer, void *context) {
  if (s_saved) {
    s_saved = false;
    update_display();
  } else if (s_confirm_abandon) {
    s_state.active = 0;
    s_state.next_workout = s_state.active_workout;
    s_state.exercise_index = 0;
    s_state.set_index = 0;
    clear_rest();
    s_confirm_abandon = false;
    save_state();
    update_display();
  } else if (s_state.rest_active) {
    clear_rest();
    save_state();
    update_display();
  } else if (!s_state.active) {
    s_state.active = 1;
    s_state.active_workout = s_state.next_workout;
    s_state.exercise_index = 0;
    s_state.set_index = 0;
    save_state();
    update_display();
  } else {
    complete_set();
  }
}

static void back_long_click(ClickRecognizerRef recognizer, void *context) {
  if (s_state.active) {
    if (s_state.rest_active) { clear_rest(); save_state(); }
    s_confirm_abandon = true;
    update_display();
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click);
  window_long_click_subscribe(BUTTON_ID_BACK, 1000, back_long_click, NULL);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  const int round_adjust = PBL_IF_ROUND_ELSE(8, 0);

  s_title_layer = text_layer_create(GRect(0, 14 + round_adjust, bounds.size.w, 34));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_color(s_title_layer, palette_accent());
  text_layer_set_background_color(s_title_layer, GColorClear);
  layer_add_child(root, text_layer_get_layer(s_title_layer));

  s_exercise_layer = text_layer_create(GRect(0, 58 + round_adjust, bounds.size.w, 100));
  text_layer_set_text_alignment(s_exercise_layer, GTextAlignmentCenter);
  text_layer_set_font(s_exercise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(s_exercise_layer, palette_primary_text());
  text_layer_set_background_color(s_exercise_layer, GColorClear);
  layer_add_child(root, text_layer_get_layer(s_exercise_layer));

  s_hint_layer = text_layer_create(GRect(0, bounds.size.h - 30, bounds.size.w, 24));
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  text_layer_set_font(s_hint_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_color(s_hint_layer, palette_accent());
  text_layer_set_background_color(s_hint_layer, GColorClear);
  layer_add_child(root, text_layer_get_layer(s_hint_layer));
  update_display();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_exercise_layer);
  text_layer_destroy(s_hint_layer);
}

static void init(void) {
  load_state();
  s_window = window_create();
  window_set_background_color(s_window, palette_background());
  window_set_click_config_provider(s_window, click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers){.load = window_load, .unload = window_unload});
  window_stack_push(s_window, true);
}

static void deinit(void) { stop_rest_services(); window_destroy(s_window); }

int main(void) {
  init();
  app_event_loop();
  deinit();
}
