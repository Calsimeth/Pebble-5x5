#include <pebble.h>
#include "plates.h"
#include "warmups.h"
#include "progression.h"
#include "deload.h"
#include "sync.h"
#include "sync_state.h"

enum {
  STORAGE_KEY_STATE = 1,
  STORAGE_SCHEMA_VERSION = 9,
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
  Weight weights[5];
  Weight active_weights[3];
  uint8_t work_reps[3][5];
  uint8_t failure_streaks[5];
  PlateCounts inventory_counts;
  uint8_t warmup_active, warmup_index;
  WarmupPlan warmup_plan;
  int32_t last_completed;
  uint8_t deload_pending[5], gap_reviewed[5], failure_reviewed[5], plateau_reviewed[5], accepted_deloads[5];
  SyncQueue outbox;
  uint32_t next_record_id;
  SyncRecord pending_record;
  uint8_t pending_valid;
  uint8_t completion_blocked;
  uint8_t selected_reps;
} PersistedState;

typedef struct {
  uint8_t schema_version, next_workout, active, active_workout, exercise_index, set_index;
  uint8_t rest_active; int32_t rest_start, rest_end; uint8_t halfway_alerted;
  Weight weights[5]; PlateCounts inventory_counts;
  uint8_t warmup_active, warmup_index; WarmupPlan warmup_plan;
} PersistedStateV5;

typedef struct {
  uint8_t schema_version, next_workout, active, active_workout, exercise_index, set_index;
  uint8_t rest_active; int32_t rest_start, rest_end; uint8_t halfway_alerted;
  Weight weights[5], active_weights[3]; uint8_t work_reps[3][5], failure_streaks[5];
  PlateCounts inventory_counts; uint8_t warmup_active, warmup_index; WarmupPlan warmup_plan;
} PersistedStateV6;

/* Exact schema 7 layout: this intentionally stops before schema-8 sync fields. */
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
  Weight weights[5];
  Weight active_weights[3];
  uint8_t work_reps[3][5];
  uint8_t failure_streaks[5];
  PlateCounts inventory_counts;
  uint8_t warmup_active;
  uint8_t warmup_index;
  WarmupPlan warmup_plan;
  int32_t last_completed;
  uint8_t deload_pending[5];
  uint8_t gap_reviewed[5];
  uint8_t accepted_deloads[5];
} PersistedStateV7;

typedef struct {
  uint8_t schema_version, next_workout, active, active_workout, exercise_index, set_index, rest_active;
  int32_t rest_start, rest_end; uint8_t halfway_alerted;
  Weight weights[5], active_weights[3]; uint8_t work_reps[3][5], failure_streaks[5];
  PlateCounts inventory_counts; uint8_t warmup_active, warmup_index; WarmupPlan warmup_plan;
  int32_t last_completed; uint8_t deload_pending[5], gap_reviewed[5], failure_reviewed[5], plateau_reviewed[5], accepted_deloads[5];
  SyncQueue outbox; uint32_t next_record_id; SyncRecord pending_record; uint8_t pending_valid;
} PersistedStateV8;

typedef struct {
  uint8_t schema_version, next_workout, active, active_workout, exercise_index, set_index;
  uint8_t rest_active; int32_t rest_start, rest_end; uint8_t halfway_alerted;
  Weight weights[5]; PlateCounts inventory_counts;
} PersistedStateV4;

typedef struct {
  uint8_t schema_version;
  uint8_t next_workout;
  uint8_t active;
  uint8_t active_workout;
  uint8_t exercise_index;
  uint8_t set_index;
} LegacyState;

typedef struct {
  uint8_t schema_version, next_workout, active, active_workout, exercise_index, set_index;
  uint8_t rest_active;
  int32_t rest_start, rest_end;
  uint8_t halfway_alerted;
} PersistedStateV2;

typedef struct {
  uint8_t schema_version, next_workout, active, active_workout, exercise_index, set_index;
  uint8_t rest_active; int32_t rest_start, rest_end; uint8_t halfway_alerted;
  Weight weights[5];
} PersistedStateV3;

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
static bool s_show_plates;
static bool s_setup;
static bool s_weights_adjusted;
static bool s_deload;
static bool s_deload_adjusting;
static bool s_plateau;
static uint8_t s_plateau_exercise;
static Weight s_deload_weight;
static uint8_t s_deload_workout;
static uint8_t s_selected_reps = 5;
static char s_feedback[24];
static uint8_t s_setup_item;
static char s_exercise_text[64];
static char s_hint_text[32];
static AppTimer *s_rest_timer;
static bool s_sync_ready;
static bool s_sync_in_flight;
static AppTimer *s_sync_ack_timer;
static SyncMachine s_sync_machine;
static void send_oldest(void);
static void retry_sync(void *context) { (void)context; s_sync_ack_timer = NULL; sync_machine_timeout(&s_sync_machine); s_sync_in_flight = false; s_sync_ready = true; send_oldest(); }
static const Weight DEFAULT_WEIGHTS[5] = {WEIGHT_LB(45), WEIGHT_LB(45), WEIGHT_LB(65), WEIGHT_LB(45), WEIGHT_LB(95)};
static const PlateCounts DEFAULT_COUNTS = {2, 0, 2, 0, 2, 2, 2};
static Weight current_weight(uint8_t workout, uint8_t exercise);
static void save_state(void);
static bool allocate_record_id(PersistedState *state, uint32_t *out) {
  return state && out && sync_allocate_id(&state->next_record_id, &state->outbox, &state->pending_record, state->pending_valid, out);
}
static PlateInventory current_inventory(void) { return plate_inventory_from_counts(s_state.inventory_counts); }
static void clear_warmup(void) { s_state.warmup_active = 0; s_state.warmup_index = 0; s_state.warmup_plan.count = 0; }
static void generate_warmup(void) {
  PlateInventory inventory = current_inventory();
  s_state.warmup_plan = calculate_warmup_plan(current_weight(s_state.active_workout, s_state.exercise_index), &inventory);
  s_state.warmup_index = 0; s_state.warmup_active = s_state.warmup_plan.count != 0;
  save_state();
}

static const char *SETUP_WEIGHT_NAMES[5] = {"Squat", "Bench", "Row", "OHP", "Deadlift"};

static size_t exercise_weight_index(uint8_t workout, uint8_t exercise) {
  return exercise == 0 ? 0 : (workout == WORKOUT_A ? exercise : exercise + 1);
}

static Weight current_weight(uint8_t workout, uint8_t exercise) {
  if (s_state.active && exercise < 3) return s_state.active_weights[exercise];
  size_t index = exercise_weight_index(workout, exercise);
  return weight_valid(s_state.weights[index]) ? s_state.weights[index] : DEFAULT_WEIGHTS[index];
}

static Weight future_weight(uint8_t workout, uint8_t exercise) {
  size_t index = exercise_weight_index(workout, exercise);
  return weight_valid(s_state.weights[index]) ? s_state.weights[index] : DEFAULT_WEIGHTS[index];
}

// Black canvas, white content, and red accents (white on Flint).
static GColor palette_background(void) { return GColorBlack; }
static GColor palette_primary_text(void) { return GColorWhite; }
static GColor palette_accent(void) { return PBL_IF_COLOR_ELSE(GColorRed, GColorWhite); }

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

static void sync_failed(DictionaryIterator *i, AppMessageResult result, void *ctx) { (void)i; (void)result; (void)ctx; s_sync_ready = false; s_sync_in_flight = false; sync_machine_transport(&s_sync_machine, false); if (!s_sync_ack_timer) s_sync_ack_timer = app_timer_register(sync_machine_retry_delay(&s_sync_machine) * 1000, retry_sync, NULL); }
static void sync_sent(DictionaryIterator *i, void *ctx) { (void)i; (void)ctx; }
static void sync_received(DictionaryIterator *i, void *ctx) {
  (void)ctx; Tuple *id = dict_find(i, MESSAGE_KEY_ack);
  if (!id) return;
  SyncRecord *r = (SyncRecord *)sync_queue_peek(&s_state.outbox);
  if (r && id->value->uint32 == r->id && sync_machine_ack(&s_sync_machine, r->id) && sync_queue_ack(&s_state.outbox, r->id)) { if (s_sync_ack_timer) { app_timer_cancel(s_sync_ack_timer); s_sync_ack_timer = NULL; } s_sync_in_flight = false; if (s_state.pending_valid && sync_queue_push(&s_state.outbox, &s_state.pending_record)) s_state.pending_valid = 0; if (s_state.completion_blocked && !s_state.pending_valid) s_state.completion_blocked = 0; save_state(); send_oldest(); update_display(); }
}
static void send_oldest(void) {
  const SyncRecord *r = sync_queue_peek(&s_state.outbox); if (!r || !s_sync_ready || s_sync_in_flight) return;
  s_sync_machine.head_id = r->id;
  char wire[128]; int n = sync_record_to_json(r, wire, sizeof wire); if (n <= 0) return;
  DictionaryIterator *out; if (!sync_machine_begin(&s_sync_machine, app_message_outbox_begin(&out) == APP_MSG_OK)) { if (!s_sync_ack_timer) s_sync_ack_timer = app_timer_register(sync_machine_retry_delay(&s_sync_machine) * 1000, retry_sync, NULL); return; }
  if (dict_write_cstring(out, MESSAGE_KEY_message, wire) != DICT_OK) { if (!s_sync_ack_timer) s_sync_ack_timer = app_timer_register(5000, retry_sync, NULL); return; }
  if (app_message_outbox_send() != APP_MSG_OK) { sync_machine_transport(&s_sync_machine, false); if (!s_sync_ack_timer) s_sync_ack_timer = app_timer_register(sync_machine_retry_delay(&s_sync_machine) * 1000, retry_sync, NULL); return; }
  sync_machine_transport(&s_sync_machine, true); s_sync_in_flight = true;
  if (!s_sync_ack_timer) s_sync_ack_timer = app_timer_register(sync_machine_retry_delay(&s_sync_machine) * 1000, retry_sync, NULL);
}

static bool valid_advisory_state(void) {
  if (s_state.last_completed < 0) return false;
  for (size_t n = 0; n < 5; n++)
    if (s_state.deload_pending[n] > 1 || s_state.gap_reviewed[n] > 1 || s_state.failure_reviewed[n] > 1 || s_state.plateau_reviewed[n] > 1) return false;
  return true;
}

static void begin_deload(uint8_t exercise) {
  PlateInventory inventory = current_inventory();
  DeloadState d = { .current_weight = s_state.weights[exercise],
    .failure_streak = s_state.failure_streaks[exercise],
    .accepted_deloads = s_state.accepted_deloads[exercise],
    .pending = s_state.deload_pending[exercise],
    .gap_reviewed = s_state.gap_reviewed[exercise],
    .failure_reviewed = s_state.failure_reviewed[exercise],
    .plateau_reviewed = s_state.plateau_reviewed[exercise] };
  bool gap = s_state.last_completed > 0 && deload_gap_due(time(NULL), s_state.last_completed);
  if (!deload_should_prompt(&d, gap)) return;
  s_deload = true; s_deload_adjusting = false; s_setup_item = exercise;
  s_deload_workout = s_state.next_workout;
  s_deload_weight = deload_weight(s_state.weights[exercise], &inventory);
  s_state.deload_pending[exercise] = 1;
  if (gap) s_state.gap_reviewed[exercise] = 1;
  save_state(); update_display();
}

static void initialize_state(void) {
  memset(&s_state, 0, sizeof(s_state));
  s_state.schema_version = STORAGE_SCHEMA_VERSION;
  s_state.next_workout = WORKOUT_A;
  memcpy(s_state.weights, DEFAULT_WEIGHTS, sizeof DEFAULT_WEIGHTS);
  memcpy(s_state.inventory_counts, DEFAULT_COUNTS, sizeof DEFAULT_COUNTS);
  s_state.last_completed = 0;
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
      memcpy(s_state.weights, DEFAULT_WEIGHTS, sizeof DEFAULT_WEIGHTS);
      save_state();
      return;
    }
  }
  if (version == 2) {
    PersistedStateV2 old;
    if (persist_read_data(STORAGE_KEY_STATE, &old, sizeof old) == sizeof old &&
        old.next_workout <= WORKOUT_B && old.active_workout <= WORKOUT_B && old.active <= 1) {
      memset(&s_state, 0, sizeof(s_state));
      s_state.next_workout = old.next_workout; s_state.active = old.active;
      s_state.active_workout = old.active_workout; s_state.exercise_index = old.exercise_index;
      s_state.set_index = old.set_index; s_state.rest_active = old.rest_active;
      s_state.rest_start = old.rest_start; s_state.rest_end = old.rest_end;
      s_state.halfway_alerted = old.halfway_alerted;
      memcpy(s_state.weights, DEFAULT_WEIGHTS, sizeof DEFAULT_WEIGHTS);
      save_state();
      if (s_state.rest_active && rest_values_valid(time(NULL))) start_rest_services();
      return;
    }
  }
  if (version == 3) {
    PersistedStateV3 old;
    if (persist_read_data(STORAGE_KEY_STATE, &old, sizeof old) == sizeof old &&
        old.next_workout <= WORKOUT_B && old.active_workout <= WORKOUT_B && old.active <= 1) {
      memset(&s_state, 0, sizeof(s_state));
      s_state.next_workout = old.next_workout; s_state.active = old.active;
      s_state.active_workout = old.active_workout; s_state.exercise_index = old.exercise_index;
      s_state.set_index = old.set_index; s_state.rest_active = old.rest_active;
      s_state.rest_start = old.rest_start; s_state.rest_end = old.rest_end;
      s_state.halfway_alerted = old.halfway_alerted;
      memcpy(s_state.weights, old.weights, sizeof old.weights);
      memcpy(s_state.inventory_counts, DEFAULT_COUNTS, sizeof DEFAULT_COUNTS);
      save_state();
      if (s_state.rest_active && rest_values_valid(time(NULL))) start_rest_services();
      return;
    }
  }
  if (version == 4) {
    PersistedStateV4 old;
    if (persist_read_data(STORAGE_KEY_STATE, &old, sizeof old) == sizeof old &&
        old.next_workout <= WORKOUT_B && old.active_workout <= WORKOUT_B && old.active <= 1) {
      memset(&s_state, 0, sizeof s_state);
      s_state.next_workout = old.next_workout; s_state.active = old.active;
      s_state.active_workout = old.active_workout; s_state.exercise_index = old.exercise_index;
      s_state.set_index = old.set_index; s_state.rest_active = old.rest_active;
      s_state.rest_start = old.rest_start; s_state.rest_end = old.rest_end;
      s_state.halfway_alerted = old.halfway_alerted; memcpy(s_state.weights, old.weights, sizeof old.weights);
      memcpy(s_state.inventory_counts, old.inventory_counts, sizeof old.inventory_counts);
      clear_warmup(); save_state();
      if (s_state.rest_active && rest_values_valid(time(NULL))) start_rest_services();
      return;
    }
  }
  if (version == 5) {
    PersistedStateV5 old;
    if (persist_read_data(STORAGE_KEY_STATE, &old, sizeof old) == sizeof old &&
        old.next_workout <= WORKOUT_B && old.active_workout <= WORKOUT_B && old.active <= 1) {
      memset(&s_state, 0, sizeof s_state);
      s_state.next_workout = old.next_workout; s_state.active = old.active;
      s_state.active_workout = old.active_workout; s_state.exercise_index = old.exercise_index;
      s_state.set_index = old.set_index; s_state.rest_active = old.rest_active;
      s_state.rest_start = old.rest_start; s_state.rest_end = old.rest_end;
      s_state.halfway_alerted = old.halfway_alerted; memcpy(s_state.weights, old.weights, sizeof old.weights);
      memcpy(s_state.inventory_counts, old.inventory_counts, sizeof old.inventory_counts);
      s_state.warmup_active = old.warmup_active; s_state.warmup_index = old.warmup_index;
      s_state.warmup_plan = old.warmup_plan;
      for (uint8_t e = 0; e < 3; e++) {
        uint8_t completed = e < s_state.exercise_index ? WORKOUTS[s_state.active_workout][e].sets :
          (e == s_state.exercise_index ? s_state.set_index : 0);
        for (uint8_t set = 0; set < completed && set < 5; set++) s_state.work_reps[e][set] = 5;
      }
      for (uint8_t n = 0; n < 3; n++) {
        size_t index = exercise_weight_index(s_state.active_workout, n);
        s_state.active_weights[n] = weight_valid(s_state.weights[index]) ? s_state.weights[index] : DEFAULT_WEIGHTS[index];
      }
      if (s_state.rest_active && !rest_values_valid(time(NULL))) clear_rest();
      save_state();
      if (s_state.rest_active) start_rest_services();
      return;
    }
  }
  if (version == 6) {
    PersistedStateV6 old;
    if (persist_read_data(STORAGE_KEY_STATE, &old, sizeof old) == sizeof old &&
        old.next_workout <= WORKOUT_B && old.active_workout <= WORKOUT_B && old.active <= 1) {
      memset(&s_state, 0, sizeof s_state);
      memcpy(&s_state, &old, sizeof old);
      s_state.last_completed = 0;
      save_state();
      if (s_state.rest_active && rest_values_valid(time(NULL))) start_rest_services();
      else if (s_state.rest_active) { clear_rest(); save_state(); }
      return;
    }
  }
  if (version == 7) {
    PersistedStateV7 old;
    if (persist_read_data(STORAGE_KEY_STATE, &old, sizeof old) == sizeof old && old.next_workout <= WORKOUT_B && old.active_workout <= WORKOUT_B && old.active <= 1) {
      memset(&s_state, 0, sizeof s_state);
      s_state.schema_version = STORAGE_SCHEMA_VERSION;
      s_state.next_workout = old.next_workout; s_state.active = old.active;
      s_state.active_workout = old.active_workout; s_state.exercise_index = old.exercise_index;
      s_state.set_index = old.set_index; s_state.rest_active = old.rest_active;
      s_state.rest_start = old.rest_start; s_state.rest_end = old.rest_end;
      s_state.halfway_alerted = old.halfway_alerted;
      memcpy(s_state.weights, old.weights, sizeof old.weights);
      memcpy(s_state.active_weights, old.active_weights, sizeof old.active_weights);
      memcpy(s_state.work_reps, old.work_reps, sizeof old.work_reps);
      memcpy(s_state.failure_streaks, old.failure_streaks, sizeof old.failure_streaks);
      memcpy(s_state.inventory_counts, old.inventory_counts, sizeof old.inventory_counts);
      s_state.warmup_active = old.warmup_active; s_state.warmup_index = old.warmup_index;
      s_state.warmup_plan = old.warmup_plan; s_state.last_completed = old.last_completed;
      memcpy(s_state.deload_pending, old.deload_pending, sizeof old.deload_pending);
      memcpy(s_state.gap_reviewed, old.gap_reviewed, sizeof old.gap_reviewed);
      memcpy(s_state.accepted_deloads, old.accepted_deloads, sizeof old.accepted_deloads);
      s_state.next_record_id = 0; s_state.outbox.count = 0; save_state();
      if (s_state.rest_active && rest_values_valid(time(NULL))) start_rest_services();
      else if (s_state.rest_active) { clear_rest(); save_state(); }
      return;
    }
  }
  if (version == 8) {
    PersistedStateV8 old;
    if (persist_read_data(STORAGE_KEY_STATE, &old, sizeof old) == sizeof old && sync_queue_valid(&old.outbox) && (!old.pending_valid || sync_record_valid(&old.pending_record))) {
      memset(&s_state, 0, sizeof s_state); memcpy(&s_state, &old, sizeof old); s_state.schema_version = STORAGE_SCHEMA_VERSION;
      s_state.completion_blocked = 0; s_state.selected_reps = 5; save_state(); return;
    }
  }
  if (persist_exists(STORAGE_KEY_STATE) &&
      persist_read_data(STORAGE_KEY_STATE, &s_state, sizeof(s_state)) == sizeof(s_state) &&
      s_state.schema_version == STORAGE_SCHEMA_VERSION &&
      s_state.completion_blocked <= 1 && s_state.selected_reps <= 5 &&
      s_state.next_workout <= WORKOUT_B && s_state.active_workout <= WORKOUT_B &&
      (!s_state.active || (s_state.exercise_index < 3 &&
       s_state.set_index < WORKOUTS[s_state.active_workout][s_state.exercise_index].sets)) &&
      s_state.active <= 1 && s_state.rest_active <= 1 && s_state.halfway_alerted <= 1 && valid_advisory_state() &&
      (!s_state.rest_active || (s_state.active &&
       s_state.exercise_index < 3 && s_state.set_index < WORKOUTS[s_state.active_workout][s_state.exercise_index].sets))) {
    for (size_t n = 0; n < 5; n++) if (!weight_valid(s_state.weights[n])) s_state.weights[n] = DEFAULT_WEIGHTS[n];
    for (size_t e = 0; e < 3; e++) for (size_t n = 0; n < 5; n++)
      if (!repetition_valid(s_state.work_reps[e][n])) s_state.work_reps[e][n] = 0;
    if (s_state.active) for (size_t e = 0; e < 3; e++)
      if (!weight_valid(s_state.active_weights[e])) s_state.active_weights[e] = future_weight(s_state.active_workout, e);
    for (size_t n = 0; n < PLATE_MAX_SIZES; n++) if (s_state.inventory_counts[n] > 2) s_state.inventory_counts[n] = DEFAULT_COUNTS[n];
    { PlateInventory inventory = current_inventory();
      if (s_state.warmup_active && (!warmup_plan_valid(&s_state.warmup_plan,
          current_weight(s_state.active_workout, s_state.exercise_index), &inventory) ||
          s_state.warmup_index >= s_state.warmup_plan.count)) { clear_warmup(); save_state(); }
    }
    if (s_state.rest_active) {
      if (rest_values_valid(time(NULL))) start_rest_services();
      else { clear_rest(); save_state(); }
    }
    uint32_t highest = sync_highest_retained_id(&s_state.outbox, &s_state.pending_record, s_state.pending_valid);
    if (highest > s_state.next_record_id) { s_state.next_record_id = highest; save_state(); }
    s_selected_reps = s_state.selected_reps; return;
  }
  initialize_state();
}

static void update_display(void) {
  if (s_plateau) {
    text_layer_set_text(s_title_layer, "Plateau likely");
    text_layer_set_text(s_exercise_layer, "Check form/rest\nSmaller jumps\nReview program");
    text_layer_set_text(s_hint_layer, "Select: dismiss");
    return;
  }
  if (s_deload) {
    char current[16], proposed[16];
    weight_format(s_state.weights[s_setup_item], current, sizeof current);
    weight_format(s_deload_weight, proposed, sizeof proposed);
    text_layer_set_text(s_title_layer, "Deload?");
    if (s_deload_adjusting)
      snprintf(s_exercise_text, sizeof s_exercise_text, "%s\n%s\n%s\nAdjust", SETUP_WEIGHT_NAMES[s_setup_item], current, proposed);
    else
      snprintf(s_exercise_text, sizeof s_exercise_text, "%s\n%s\n%s", SETUP_WEIGHT_NAMES[s_setup_item], current, proposed);
    text_layer_set_text(s_exercise_layer, s_exercise_text);
    text_layer_set_text(s_hint_layer, s_deload_adjusting ? "Sel: yes Back: no" : "Sel: yes Dn: no");
    return;
  }
  if (s_setup) {
    if (s_setup_item < 5) {
      char weight[16]; weight_format(s_state.weights[s_setup_item], weight, sizeof weight);
      snprintf(s_exercise_text, sizeof s_exercise_text, "%s\n%s\nQuarter lb", SETUP_WEIGHT_NAMES[s_setup_item], weight);
      text_layer_set_text(s_title_layer, "Set Weight");
    } else {
      PlateInventory inventory = current_inventory(); size_t n = s_setup_item - 5; char size[16]; weight_format(inventory.plates[n].size, size, sizeof size);
      snprintf(s_exercise_text, sizeof s_exercise_text, "%s\nCount / side: %d", size, s_state.inventory_counts[n]);
      text_layer_set_text(s_title_layer, "Set Plates");
    }
    text_layer_set_text(s_exercise_layer, s_exercise_text);
    snprintf(s_hint_text, sizeof s_hint_text, "Sel: next  Up/Down");
    text_layer_set_text(s_hint_layer, s_hint_text);
    return;
  }
  if (s_weights_adjusted) { text_layer_set_text(s_title_layer, "Weights adjusted"); s_weights_adjusted = false; }
  WorkoutType workout = s_state.active ? s_state.active_workout : s_state.next_workout;

  if (s_saved) {
    s_feedback[0] = 0;
    text_layer_set_text(s_title_layer, "Workout Saved");
    text_layer_set_text(s_exercise_layer, workout_name(s_state.next_workout));
    text_layer_set_text(s_hint_layer, "Select: start");
    return;
  }

  if (s_state.rest_active) {
    s_feedback[0] = 0;
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

  if (s_state.active && s_state.warmup_active && !s_show_plates) {
    WarmupSet set = s_state.warmup_plan.sets[s_state.warmup_index];
    snprintf(s_exercise_text, sizeof s_exercise_text, "%s\nWarmup\nWarm %d of %d\n%ld lb\n5 reps",
             WORKOUTS[workout][s_state.exercise_index].name, s_state.warmup_index + 1,
             s_state.warmup_plan.count, (long)(set.weight / 4));
    text_layer_set_text(s_title_layer, s_feedback[0] ? s_feedback : "Warmup"); s_feedback[0] = 0;
    text_layer_set_text(s_exercise_layer, s_exercise_text);
    text_layer_set_text(s_hint_layer, "Sel: done Dn: skip"); return;
  }

  if (s_state.active && !s_state.warmup_active && !s_show_plates) {
    const ExerciseDefinition *current = &WORKOUTS[workout][s_state.exercise_index];
    char weight[16]; weight_format(current_weight(workout, s_state.exercise_index), weight, sizeof weight);
    snprintf(s_exercise_text, sizeof(s_exercise_text), "%s\nSet %d of %d\n%s\nReps: %d",
             current->name, s_state.set_index + 1, current->sets, weight, s_selected_reps);
    text_layer_set_text(s_title_layer, s_feedback[0] ? s_feedback : workout_name(workout));
    s_feedback[0] = 0;
    snprintf(s_hint_text, sizeof(s_hint_text), "Up:plates Dn:reps");
    text_layer_set_text(s_exercise_layer, s_exercise_text);
    text_layer_set_text(s_hint_layer, s_hint_text);
    return;
  }

  if (s_show_plates && s_state.active) {
    PlateInventory inventory = current_inventory();
    Weight displayed_weight = s_state.warmup_active ? s_state.warmup_plan.sets[s_state.warmup_index].weight :
      current_weight(s_state.active_workout, s_state.exercise_index);
    PlateLoad load = calculate_plate_load(displayed_weight, &inventory);
    char side[64]; format_plate_side(&inventory, &load, side, sizeof side);
    char actual[16]; weight_format(load.actual_total, actual, sizeof actual);
    snprintf(s_exercise_text, sizeof s_exercise_text, "Plates / Side\n%s\n%s%s", actual,
             side, load.exact ? "" : "\nRounded down");
    text_layer_set_text(s_title_layer, "Plates / Side"); text_layer_set_text(s_exercise_layer, s_exercise_text);
    text_layer_set_text(s_hint_layer, "Select: back"); return;
  }

  text_layer_set_text(s_title_layer, s_weights_adjusted ? "Weights adjusted" : workout_name(workout));
  s_weights_adjusted = false;
  if (!s_state.active) {
    char w0[16], w1[16], w2[16];
    weight_format(current_weight(workout, 0), w0, sizeof w0);
    weight_format(current_weight(workout, 1), w1, sizeof w1);
    weight_format(current_weight(workout, 2), w2, sizeof w2);
    snprintf(s_exercise_text, sizeof(s_exercise_text), "%s %s\n%s %s\n%s %s",
             WORKOUTS[workout][0].name, w0, WORKOUTS[workout][1].name, w1,
             WORKOUTS[workout][2].name, w2);
    snprintf(s_hint_text, sizeof(s_hint_text), "Sel:start Dn:setup");
  } else {
    const ExerciseDefinition *current = &WORKOUTS[workout][s_state.exercise_index];
    char weight[16]; weight_format(current_weight(workout, s_state.exercise_index), weight, sizeof weight);
    snprintf(s_exercise_text, sizeof(s_exercise_text), "%s\nSet %d of %d\n%s\n5 reps",
             current->name, s_state.set_index + 1, current->sets, weight);
    snprintf(s_hint_text, sizeof(s_hint_text), s_confirm_abandon ? "Select: abandon" : "Up: plates");
  }
  if (s_state.outbox.count >= SYNC_QUEUE_CAPACITY) snprintf(s_hint_text, sizeof s_hint_text, "Sync Required");
  else if (s_state.outbox.count > 0) snprintf(s_hint_text, sizeof s_hint_text, "Not Synced");
  text_layer_set_text(s_exercise_layer, s_exercise_text);
  text_layer_set_text(s_hint_layer, s_hint_text);
}

static void complete_set(void) {
  if (s_state.completion_blocked) { snprintf(s_feedback, sizeof s_feedback, "Sync Required"); update_display(); return; }
  if (s_state.warmup_active) {
    if (++s_state.warmup_index < s_state.warmup_plan.count) { save_state(); update_display(); return; }
    clear_warmup(); save_state();
    time_t now = time(NULL); s_state.rest_active = 1; s_state.rest_start = (int32_t)now;
    s_state.rest_end = (int32_t)(now + REST_SECONDS); s_state.halfway_alerted = 0;
    save_state(); start_rest_services(); update_display(); return;
  }
  const ExerciseDefinition *current = &WORKOUTS[s_state.active_workout][s_state.exercise_index];
  if (s_state.exercise_index == 2 && s_state.set_index == current->sets - 1 && s_state.outbox.count >= SYNC_QUEUE_CAPACITY && s_state.pending_valid) {
    s_state.completion_blocked = 1; save_state(); snprintf(s_feedback, sizeof s_feedback, "Sync Required"); update_display(); return;
  }
  s_state.work_reps[s_state.exercise_index][s_state.set_index] = s_selected_reps;
  save_state();
  s_selected_reps = 5;
  s_state.selected_reps = 5;
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
    bool success = exercise_succeeded(s_state.work_reps[s_state.exercise_index], current->sets);
    size_t weight_index = exercise_weight_index(s_state.active_workout, s_state.exercise_index);
    Weight old_weight = s_state.active_weights[s_state.exercise_index];
    PlateInventory inventory = current_inventory();
    s_state.weights[weight_index] = success ? successful_target(old_weight, &inventory) : failed_target(old_weight);
    s_state.failure_streaks[weight_index] = failure_streak_after(success, s_state.failure_streaks[weight_index]);
    if (!success && deload_after_failure(s_state.failure_streaks[weight_index])) { s_state.deload_pending[weight_index] = 1; s_state.failure_reviewed[weight_index] = 0; s_state.plateau_reviewed[weight_index] = 0; }
    snprintf(s_feedback, sizeof s_feedback, success ? "Weight increased" : "Repeat weight");
    s_state.set_index = 0;
    s_state.exercise_index++;
    if (s_state.exercise_index >= 3) {
      s_state.active = 0;
      s_state.last_completed = (int32_t)time(NULL);
      memset(s_state.gap_reviewed, 0, sizeof s_state.gap_reviewed);
      SyncRecord record = {0};
      if (!allocate_record_id(&s_state, &record.id)) { s_state.completion_blocked = 1; s_state.active = 1; s_state.exercise_index = 2; s_state.set_index = 0; save_state(); snprintf(s_feedback, sizeof s_feedback, "Sync Required"); update_display(); return; }
      record.schema_version = SYNC_RECORD_VERSION; record.workout = s_state.active_workout;
      record.completed_at = s_state.last_completed; record.complete = 1; record.rep_count = s_state.active_workout == WORKOUT_A ? 15 : 11;
      for (uint8_t e = 0, offset = 0; e < 3; e++) { uint8_t sets = WORKOUTS[s_state.active_workout][e].sets; record.exercise_ids[e] = s_state.active_workout == WORKOUT_A ? e : (e == 0 ? 0 : (e == 1 ? 3 : 4)); record.weights[e] = s_state.active_weights[e]; memcpy(record.reps + offset, s_state.work_reps[e], sets); offset += sets; }
      for (uint8_t e = 0; e < 5; e++) if (s_state.deload_pending[e]) record.deload_mask |= (uint8_t)(1u << e);
      SyncPushResult result = sync_queue_push_result(&s_state.outbox, &record);
      if (result == SYNC_PUSH_FULL) { s_state.pending_record = record; s_state.pending_valid = 1; snprintf(s_feedback, sizeof s_feedback, "Sync Required"); }
      else if (result == SYNC_PUSH_ADDED || result == SYNC_PUSH_IDENTICAL) { save_state(); send_oldest(); }
      else { s_state.completion_blocked = 1; s_state.active = 1; s_state.exercise_index = 2; s_state.set_index = 0; save_state(); snprintf(s_feedback, sizeof s_feedback, "Sync Required"); update_display(); return; }
      s_state.next_workout = s_state.active_workout == WORKOUT_A ? WORKOUT_B : WORKOUT_A;
      s_saved = true;
    } else generate_warmup();
  }
  save_state();
  update_display();
}

static void select_click(ClickRecognizerRef recognizer, void *context) {
  if (s_plateau) { s_state.plateau_reviewed[s_plateau_exercise] = 1; s_plateau = false; save_state(); }
  if (s_deload) {
    s_state.weights[s_setup_item] = s_deload_weight;
    s_state.deload_pending[s_setup_item] = 0;
    s_state.failure_streaks[s_setup_item] = 0;
    s_state.failure_reviewed[s_setup_item] = 1;
    if (s_state.accepted_deloads[s_setup_item] != UINT8_MAX) s_state.accepted_deloads[s_setup_item]++;
    s_deload = false; s_deload_adjusting = false; save_state(); update_display(); return;
  }
  if (s_setup) {
    s_setup_item = (s_setup_item + 1) % 12;
    update_display();
    return;
  }
  if (s_show_plates) { s_show_plates = false; update_display(); return; }
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
    for (uint8_t n = 0; n < 5; n++) {
      DeloadState d = { .failure_streak = s_state.failure_streaks[n], .accepted_deloads = s_state.accepted_deloads[n], .failure_reviewed = s_state.failure_reviewed[n], .plateau_reviewed = s_state.plateau_reviewed[n] };
      if (plateau_advisory_due(&d)) { s_plateau = true; s_plateau_exercise = n; update_display(); return; }
    }
    uint8_t indices[3];
    deload_workout_order(s_state.next_workout == WORKOUT_B, indices);
    for (uint8_t n = 0; n < 3; n++) { begin_deload(indices[n]); if (s_deload) return; }
    s_state.active_workout = s_state.next_workout;
    s_state.exercise_index = 0;
    s_state.set_index = 0;
    for (uint8_t n = 0; n < 3; n++) {
      s_state.active_weights[n] = current_weight(s_state.next_workout, n);
      memset(s_state.work_reps[n], 0, sizeof s_state.work_reps[n]);
    }
    s_state.active = 1;
    generate_warmup();
    save_state();
    update_display();
  } else {
    complete_set();
  }
}

static void up_click(ClickRecognizerRef recognizer, void *context) {
  if (s_deload) {
    PlateInventory inventory = current_inventory();
    s_deload_adjusting = true;
    s_deload_weight = next_achievable_total(s_deload_weight, &inventory);
    update_display(); return;
  }
  if (s_setup) {
    if (s_setup_item < 5) { PlateInventory inventory = current_inventory(); Weight old = s_state.weights[s_setup_item]; Weight next = next_achievable_total(old, &inventory); s_state.weights[s_setup_item] = next; s_state.failure_streaks[s_setup_item] = failure_streak_after_manual_weight_change(s_state.failure_streaks[s_setup_item], old, next); }
    else if (s_state.inventory_counts[s_setup_item - 5] < 2) {
      s_state.inventory_counts[s_setup_item - 5]++;
    }
    save_state(); update_display(); return;
  }
  if (s_state.active && !s_state.rest_active && !s_confirm_abandon) { s_show_plates = !s_show_plates; update_display(); }
}

static void down_click(ClickRecognizerRef recognizer, void *context) {
  if (s_deload) {
    PlateInventory inventory = current_inventory();
    if (s_deload_adjusting) s_deload_weight = previous_achievable_total(s_deload_weight, &inventory);
    else { s_state.deload_pending[s_setup_item] = 0; s_state.failure_reviewed[s_setup_item] = 1; s_deload = false; save_state(); }
    update_display(); return;
  }
  if (!s_setup) {
    if (s_state.active && s_state.warmup_active && !s_state.rest_active) {
      if (++s_state.warmup_index >= s_state.warmup_plan.count) clear_warmup();
      save_state(); update_display();
    } else if (s_state.active && !s_state.rest_active && !s_show_plates && !s_confirm_abandon) {
      s_selected_reps = s_selected_reps == 0 ? 5 : s_selected_reps - 1; s_state.selected_reps = s_selected_reps; save_state(); update_display();
    } else if (!s_state.active && !s_saved) { s_setup = true; s_setup_item = 0; update_display(); }
    return;
  }
  if (s_setup_item < 5) {
    { PlateInventory inventory = current_inventory(); Weight old = s_state.weights[s_setup_item]; Weight next = previous_achievable_total(old, &inventory); s_state.weights[s_setup_item] = next; s_state.failure_streaks[s_setup_item] = failure_streak_after_manual_weight_change(s_state.failure_streaks[s_setup_item], old, next); }
  } else if (s_state.inventory_counts[s_setup_item - 5] > 0) {
    s_state.inventory_counts[s_setup_item - 5]--;
  }
  { PlateInventory inventory = current_inventory(); bool changed = false;
    for (size_t n = 0; n < 5; n++) { Weight old = s_state.weights[n]; s_state.weights[n] = normalize_weight_down(old, &inventory); if (old != s_state.weights[n]) { changed = true; s_state.failure_streaks[n] = 0; } }
    save_state();
    s_weights_adjusted = changed;
  }
  update_display();
}

static void back_long_click(ClickRecognizerRef recognizer, void *context) {
  if (s_deload) { s_deload = false; s_deload_adjusting = false; update_display(); return; }
  if (s_setup) { s_setup = false; update_display(); return; }
  if (s_state.active) {
    if (s_state.rest_active) { clear_rest(); save_state(); }
    clear_warmup(); s_state.completion_blocked = 0; s_state.selected_reps = 5; s_selected_reps = 5; save_state(); s_confirm_abandon = true;
    update_display();
  }
}

static void up_long_click(ClickRecognizerRef recognizer, void *context) {
  if (!s_state.active && !s_saved) { s_setup = true; s_setup_item = 0; update_display(); }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click);
  window_single_click_subscribe(BUTTON_ID_UP, up_click);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click);
  window_long_click_subscribe(BUTTON_ID_BACK, 1000, back_long_click, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, 1000, up_long_click, NULL);
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
  sync_machine_init(&s_sync_machine, sync_queue_peek(&s_state.outbox) ? sync_queue_peek(&s_state.outbox)->id : 0);
  if (!sync_queue_valid(&s_state.outbox)) { s_state.outbox.count = 0; save_state(); }
  if (s_state.pending_valid && !sync_record_valid(&s_state.pending_record)) { s_state.pending_valid = 0; save_state(); }
  app_message_register_inbox_received(sync_received); app_message_register_outbox_sent(sync_sent);
  /* Legacy readiness hook is intentionally disabled; Pebble has no readiness callback. */
#if 0
  AppMessageResult app_result = app_message_open(128, 128); s_sync_ready = app_result == APP_MSG_OK;
  if (s_sync_ready) send_oldest();
  app_message_register_outbox_failed(sync_failed); app_message_registerด_outbox_sent(sync_sent);
  app_message_open(128, 128); app_message_register_outbox_ready(sync_ready);
#endif
  app_message_register_outbox_failed(sync_failed);
  AppMessageResult app_result = app_message_open(128, 128); s_sync_ready = app_result == APP_MSG_OK;
  if (s_state.pending_valid && sync_queue_push(&s_state.outbox, &s_state.pending_record)) { s_state.pending_valid = 0; save_state(); }
  if (s_sync_ready) send_oldest();
  s_window = window_create();
  window_set_background_color(s_window, palette_background());
  window_set_click_config_provider(s_window, click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers){.load = window_load, .unload = window_unload});
  window_stack_push(s_window, true);
}

static void deinit(void) { stop_rest_services(); if (s_sync_ack_timer) { app_timer_cancel(s_sync_ack_timer); s_sync_ack_timer = NULL; } window_destroy(s_window); }

int main(void) {
  init();
  app_event_loop();
  deinit();
}
