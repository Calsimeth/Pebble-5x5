#include <pebble.h>
#include "plates.h"
#include "warmups.h"
#include "progression.h"
#include "deload.h"
#include "sync.h"
#include "sync_state.h"
#include "sync_adapter.h"
#include "sync_completion.h"
#include "workout_completion.h"
#include "history_progress.h"
#include "query_controller.h"
#include "workout_view.h"
#include "rest_state.h"
#include "migration.h"

enum {
  STORAGE_KEY_STATE = 1,
  STORAGE_SCHEMA_VERSION = WORKOUT_STORAGE_SCHEMA,
  REST_SECONDS = 180,
};

typedef struct {
  const char *name;
  uint8_t sets;
} ExerciseDefinition;

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
static Layer *s_history_progress_layer;
static Layer *s_workout_layer;
static PersistedState s_state;
static bool s_saved;
static bool s_confirm_abandon;
static bool s_show_plates;
static bool s_setup;
typedef enum { SETUP_MENU, SETUP_WEIGHTS, SETUP_PLATES } SetupMode;
static SetupMode s_setup_mode;
static uint8_t s_setup_menu_index;
static uint8_t s_weight_index;
static uint8_t s_plate_index;
typedef enum { SCREEN_HOME, SCREEN_SETUP, SCREEN_WORKOUT, SCREEN_WORKOUT_SELECT, SCREEN_HISTORY, SCREEN_PROGRESS_PICKER, SCREEN_PROGRESS_GRAPH } ScreenState;
static ScreenState s_screen = SCREEN_HOME;
static uint8_t s_home_item;
static WorkoutType s_selected_workout;
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
static const uint32_t REST_DURATIONS[] = {800, 200, 800, 200, 800};
static const VibePattern REST_COMPLETE_PATTERN = { .durations = REST_DURATIONS, .num_segments = 5 };
static bool s_sync_ready;
static uint16_t s_query_id;
static int s_calendar_year, s_calendar_month;
static uint8_t s_progress_exercise, s_progress_page;
static bool s_query_connected;
static CalendarResponse s_calendar;
static ProgressAssembly s_progress_data;
static QueryController s_query_controller;
static bool s_calendar_valid;
static char s_deferred_query[24];
static AppTimer *s_query_timer;
#define MESSAGE_KEY_type 10006
#define MESSAGE_KEY_id 10007
#define MESSAGE_KEY_year 10008
#define MESSAGE_KEY_month 10009
#define MESSAGE_KEY_exercise 10010
#define MESSAGE_KEY_page 10011
#define MESSAGE_KEY_calendar_id 10012
#define MESSAGE_KEY_progress_id 10019
#define MESSAGE_KEY_progress_exercise 10020
#define MESSAGE_KEY_progress_total 10022
#define MESSAGE_KEY_progress_point_count 10025
#define MESSAGE_KEY_calendar_mask 10016
#define MESSAGE_KEY_calendar_mask_a 10017
#define MESSAGE_KEY_calendar_mask_b 10018
#define MESSAGE_KEY_progress_chunk_index 10023
#define MESSAGE_KEY_progress_chunk_count 10024
#define MESSAGE_KEY_progress_t0 10026
#define MESSAGE_KEY_progress_w0 10031
static bool s_sync_in_flight;
static AppTimer *s_sync_ack_timer;
static SyncAdapter s_sync_adapter;
static DictionaryIterator *s_sync_iterator;
static char s_sync_wire[128];
static void send_oldest(void);
static void retry_sync(void *context);
static bool sync_begin_adapter(void *context) { (void)context; return app_message_outbox_begin(&s_sync_iterator) == APP_MSG_OK; }
static bool sync_write_adapter(void *context) { (void)context; return dict_write_cstring(s_sync_iterator, MESSAGE_KEY_message, s_sync_wire) == DICT_OK; }
static bool sync_send_adapter(void *context) { (void)context; return app_message_outbox_send() == APP_MSG_OK; }
static bool sync_timer_adapter(uint32_t seconds, void *context) {
  (void)context;
  if (s_sync_ack_timer) return true;
  s_sync_ack_timer = app_timer_register(seconds * 1000, retry_sync, NULL);
  return s_sync_ack_timer != NULL;
}
static void sync_cancel_adapter(void *context) { (void)context; if (s_sync_ack_timer) { app_timer_cancel(s_sync_ack_timer); s_sync_ack_timer = NULL; } }
static void retry_sync(void *context) {
  (void)context;
  s_sync_ack_timer = NULL;
  if (s_sync_adapter.machine.state == SYNC_WAITING_ACK) {
    s_sync_in_flight = false;
    sync_adapter_timeout(&s_sync_adapter);
  } else {
    sync_adapter_retry_elapsed(&s_sync_adapter);
    s_sync_in_flight = false;
    send_oldest();
  }
}
static const Weight DEFAULT_WEIGHTS[5] = {WEIGHT_LB(45), WEIGHT_LB(45), WEIGHT_LB(65), WEIGHT_LB(45), WEIGHT_LB(95)};
static const PlateCounts DEFAULT_COUNTS = {2, 0, 1, 0, 1, 1, 1};
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

static Weight current_weight(uint8_t workout, uint8_t exercise) {
  if (s_state.active && exercise < 3) return s_state.active_weights[exercise];
  size_t index = workout_weight_index(workout, exercise);
  return weight_valid(s_state.weights[index]) ? s_state.weights[index] : DEFAULT_WEIGHTS[index];
}

static Weight future_weight(uint8_t workout, uint8_t exercise) {
  size_t index = workout_weight_index(workout, exercise);
  return weight_valid(s_state.weights[index]) ? s_state.weights[index] : DEFAULT_WEIGHTS[index];
}

// Black canvas, white content, and red accents (white on Flint).
static GColor palette_background(void) { return GColorBlack; }
static GColor palette_primary_text(void) { return GColorWhite; }
static GColor palette_accent(void) { return PBL_IF_COLOR_ELSE(GColorRed, GColorWhite); }

static void update_display(void);
static void query_timeout(void *ctx);
static void resume_deferred_query(void);
#ifdef STRONGLIFTS_VISUAL_FIXTURES
static void load_visual_fixture(void) {
  s_calendar_year=2026; s_calendar_month=8; s_screen=SCREEN_HISTORY; s_query_connected=true;
  s_calendar=(CalendarResponse){1,2026,8,31,(1u<<2)|(1u<<9)|(1u<<17)|(1u<<23)|(1u<<30),(1u<<2)|(1u<<17)|(1u<<30),(1u<<9)|(1u<<23)}; s_calendar_valid=true;
  progress_assembly_reset(&s_progress_data); s_progress_exercise=0;
#if STRONGLIFTS_FIXTURE_ID == 1
  s_screen=SCREEN_HISTORY;
  { ProgressPoint p[5]={{1700000000,180},{1701000000,185},{1702000000,175},{1703000000,195},{1704000000,190}}; progress_chunk_add(&s_progress_data,2,0,0,1,0,1,5,p); }
#elif STRONGLIFTS_FIXTURE_ID == 2
  s_calendar.mask=0;
#elif STRONGLIFTS_FIXTURE_ID == 3
  s_screen=SCREEN_PROGRESS_GRAPH; s_query_connected=true;
  { ProgressPoint p[5]={{1700000000,180},{1701000000,185},{1702000000,175},{1703000000,195},{1704000000,190}}; progress_chunk_add(&s_progress_data,2,0,0,1,0,1,5,p); }
#elif STRONGLIFTS_FIXTURE_ID == 4
  s_screen=SCREEN_PROGRESS_GRAPH; s_query_connected=true; progress_chunk_add(&s_progress_data,2,0,0,0,0,1,0,0);
#elif STRONGLIFTS_FIXTURE_ID == 5
  s_screen=SCREEN_HISTORY; s_calendar_valid=false; s_query_connected=false; s_query_controller.state=QUERY_WAITING_RESPONSE;
#elif STRONGLIFTS_FIXTURE_ID == 6
  s_screen=SCREEN_HISTORY; s_calendar_valid=false; s_query_connected=false; s_query_controller.state=QUERY_FAILED;
#endif
}
#endif
static void query_cancel(void) { if(s_query_timer){app_timer_cancel(s_query_timer);s_query_timer=NULL;} query_controller_fail(&s_query_controller);s_query_connected=false;s_calendar_valid=false;s_deferred_query[0]=0;send_oldest(); }
static void history_progress_draw(Layer *layer, GContext *ctx) {
  GRect b=layer_get_bounds(layer); graphics_context_set_stroke_color(ctx,palette_primary_text());
  graphics_context_set_text_color(ctx,GColorWhite); graphics_context_set_fill_color(ctx,palette_accent());
  if(s_screen==SCREEN_HISTORY && s_calendar_valid) {
    graphics_draw_text(ctx,"History",fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),GRect(0,0,b.size.w,22),GTextOverflowModeFill,GTextAlignmentCenter,NULL);
    struct tm tm={0}; tm.tm_year=s_calendar_year-1900;tm.tm_mon=s_calendar_month-1;tm.tm_mday=1; mktime(&tm); int first=tm.tm_wday;
    char header[16];snprintf(header,sizeof header,"%d/%d",s_calendar_month,s_calendar_year);graphics_draw_text(ctx,header,fonts_get_system_font(FONT_KEY_GOTHIC_14),GRect(0,18,b.size.w,18),GTextOverflowModeFill,GTextAlignmentCenter,NULL);
    int left=PBL_IF_ROUND_ELSE(30,0), bottom=PBL_IF_ROUND_ELSE(22,0), grid_width=b.size.w-left*2, cw=grid_width/7, top=44, ch=(b.size.h-top-bottom)/6;
    static const char *weekdays[] = {"S","M","T","W","T","F","S"};
    for(int x=0;x<7;x++) graphics_draw_text(ctx,weekdays[x],fonts_get_system_font(FONT_KEY_GOTHIC_14),GRect(left+x*cw,32,cw,14),GTextOverflowModeFill,GTextAlignmentCenter,NULL);
    for(int x=0;x<=7;x++) graphics_draw_line(ctx,GPoint(left+x*cw,top),GPoint(left+x*cw,top+ch*6));
    for(int y=0;y<=6;y++) graphics_draw_line(ctx,GPoint(left,top+y*ch),GPoint(left+grid_width,top+y*ch));
    for(int d=1;d<=s_calendar.days;d++){int n=first+d-1,x=n%7,y=n/7;uint32_t bit=1u<<(d-1);bool marked=(s_calendar.mask&bit)!=0;char v[4];snprintf(v,sizeof v,"%d",d);if(marked){graphics_context_set_fill_color(ctx,palette_accent());graphics_fill_rect(ctx,GRect(left+x*cw+1,top+y*ch+1,cw-2,ch-2),0,GCornerNone);graphics_context_set_text_color(ctx,PBL_IF_COLOR_ELSE(GColorWhite,GColorBlack));}else graphics_context_set_text_color(ctx,GColorWhite);graphics_draw_text(ctx,v,fonts_get_system_font(FONT_KEY_GOTHIC_14),GRect(left+x*cw+1,top+y*ch+1,cw-2,ch-2),GTextOverflowModeFill,GTextAlignmentCenter,NULL);if(marked){char which[3];bool a=(s_calendar.mask_a&bit)!=0,bm=(s_calendar.mask_b&bit)!=0;snprintf(which,sizeof which,"%s",a&&bm?"AB":(bm?"B":"A"));graphics_draw_text(ctx,which,fonts_get_system_font(FONT_KEY_GOTHIC_09),GRect(left+x*cw+1,top+y*ch+ch-9,cw-2,9),GTextOverflowModeFill,GTextAlignmentCenter,NULL);}}
  } else if(s_screen==SCREEN_PROGRESS_GRAPH && progress_assembly_complete(&s_progress_data) && s_progress_data.total_points) {
    int32_t min=INT32_MAX,max=INT32_MIN;for(uint8_t i=0;i<s_progress_data.total_points;i++){if(s_progress_data.points[i].w<min)min=s_progress_data.points[i].w;if(s_progress_data.points[i].w>max)max=s_progress_data.points[i].w;}
    if(min!=INT32_MAX){graphics_context_set_stroke_color(ctx,palette_accent());for(uint8_t i=1;i<s_progress_data.total_points;i++){int x0=(i-1)*b.size.w/(s_progress_data.total_points-1),x1=i*b.size.w/(s_progress_data.total_points-1);int y0=graph_coordinate(s_progress_data.points[i-1].w,min,max,62)+28,y1=graph_coordinate(s_progress_data.points[i].w,min,max,62)+28;graphics_draw_line(ctx,GPoint(x0,y0),GPoint(x1,y1));}char stats[64],range[32];struct tm *lo=localtime((time_t *)&s_progress_data.points[0].t),*hi=localtime((time_t *)&s_progress_data.points[s_progress_data.total_points-1].t);strftime(range,sizeof range,"%m/%d",lo);if(hi) {char tail[8];strftime(tail,sizeof tail,"-%m/%d",hi);strncat(range,tail,sizeof(range)-strlen(range)-1);}snprintf(stats,sizeof stats,"%s Min %ld Max %ld lb",range,(long)min,(long)max);graphics_draw_text(ctx,stats,fonts_get_system_font(FONT_KEY_GOTHIC_14),GRect(0,94,b.size.w,16),GTextOverflowModeFill,GTextAlignmentCenter,NULL);graphics_draw_text(ctx,"Page 1/1",fonts_get_system_font(FONT_KEY_GOTHIC_14),GRect(0,110,b.size.w,16),GTextOverflowModeFill,GTextAlignmentCenter,NULL);}
    graphics_draw_text(ctx,SETUP_WEIGHT_NAMES[s_progress_exercise],fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),GRect(0,0,b.size.w,22),GTextOverflowModeFill,GTextAlignmentCenter,NULL);
    graphics_draw_text(ctx,"lb",fonts_get_system_font(FONT_KEY_GOTHIC_14),GRect(0,b.size.h-18,b.size.w,18),GTextOverflowModeFill,GTextAlignmentCenter,NULL);
  } else {
    const char *label = !s_query_connected ? (s_query_controller.state==QUERY_FAILED ? "Phone Needed" : "Loading") : (s_screen==SCREEN_HISTORY ? "No History" : "No Progress");
    graphics_context_set_fill_color(ctx,GColorWhite); graphics_draw_text(ctx,label,fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),GRect(0,30,b.size.w,26),GTextOverflowModeFill,GTextAlignmentCenter,NULL);
  }
}
static void back_click(ClickRecognizerRef recognizer, void *context);
static void query_send(const char *type) {
  DictionaryIterator *it;
  if (sync_queue_peek(&s_state.outbox) || s_sync_in_flight || s_sync_adapter.machine.state != SYNC_IDLE) { snprintf(s_deferred_query,sizeof s_deferred_query,"%s",type); query_controller_defer(&s_query_controller,true); return; }
  if (!query_controller_begin(&s_query_controller,(uint16_t)(s_query_id+1))) return;
  if (!s_sync_ready || app_message_outbox_begin(&it) != APP_MSG_OK) { query_cancel(); return; }
  if (++s_query_id == 0) s_query_id=1;
  dict_write_cstring(it,MESSAGE_KEY_type,type); dict_write_uint16(it,MESSAGE_KEY_id,s_query_id);
  if (type[0]=='c') { dict_write_uint16(it,MESSAGE_KEY_year,s_calendar_year); dict_write_uint8(it,MESSAGE_KEY_month,s_calendar_month); }
  else { dict_write_uint8(it,MESSAGE_KEY_exercise,s_progress_exercise); dict_write_uint8(it,MESSAGE_KEY_page,s_progress_page); }
  s_query_connected=app_message_outbox_send()==APP_MSG_OK;
  if(s_query_connected){s_query_controller.state=QUERY_WAITING_RESPONSE;s_query_timer=app_timer_register(5000,query_timeout,NULL);if(!s_query_timer)query_cancel();}else query_cancel();
}
static void resume_deferred_query(void) { if(s_deferred_query[0] && !sync_queue_peek(&s_state.outbox) && !s_sync_in_flight && s_sync_adapter.machine.state==SYNC_IDLE){char q[24];snprintf(q,sizeof q,"%s",s_deferred_query);s_deferred_query[0]=0;query_send(q);} }
static void query_timeout(void *ctx){(void)ctx;s_query_timer=NULL;query_controller_fail(&s_query_controller);s_query_connected=false;s_calendar_valid=false;s_deferred_query[0]=0;send_oldest();update_display();}

static void workout_layer_update(Layer *layer, GContext *ctx) {
  if (!s_state.active || s_state.warmup_active || s_screen != SCREEN_WORKOUT) return;
  GRect b = layer_get_bounds(layer); uint8_t sets = WORKOUTS[s_state.active_workout][s_state.exercise_index].sets;
  WorkoutCircleLayout circles = workout_circle_layout(b.size.w, b.size.h, sets);
  WorkoutViewModel view = {.set_count=sets, .completed_count=s_state.set_index, .selected_reps=s_selected_reps};
  memcpy(view.completed_reps, s_state.work_reps[s_state.exercise_index], sizeof view.completed_reps);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, WORKOUTS[s_state.active_workout][s_state.exercise_index].name,
    fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(6, 2, b.size.w / 2, 32), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  char weight[16], goal[24]; weight_format(current_weight(s_state.active_workout, s_state.exercise_index), weight, sizeof weight);
  snprintf(goal, sizeof goal, "%dx5 %s", sets, weight);
  graphics_draw_text(ctx, goal, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(b.size.w / 2, 6, b.size.w / 2 - 6, 28), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
  if (s_state.rest_active) {
    uint32_t elapsed = rest_elapsed(s_state.rest_start, (int32_t)time(NULL));
    char timer[12]; snprintf(timer, sizeof timer, "%lu:%02lu", (unsigned long)(elapsed / 60), (unsigned long)(elapsed % 60));
    graphics_draw_text(ctx, timer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GRect(0, 38, b.size.w, 36), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
  for (uint8_t n = 0; n < sets; n++) {
    int16_t x = circles.x + n * (circles.diameter + circles.gap) + circles.diameter / 2;
    int16_t y = circles.y + circles.diameter / 2;
    bool done = n < s_state.set_index;
    if (done) {
      graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorWhite)); graphics_fill_circle(ctx, GPoint(x, y), circles.diameter / 2);
      graphics_context_set_text_color(ctx, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
    } else {
      graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack));
      graphics_fill_circle(ctx, GPoint(x, y), circles.diameter / 2);
      graphics_context_set_stroke_color(ctx, GColorWhite); graphics_draw_circle(ctx, GPoint(x, y), circles.diameter / 2);
      graphics_context_set_text_color(ctx, PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite));
    }
    char reps[4]; snprintf(reps, sizeof reps, "%d", workout_view_display_reps(&view, n));
    graphics_draw_text(ctx, reps, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(x - circles.diameter / 2, y - 11, circles.diameter, 24), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
}

static void set_workout_layer_visible(bool visible) {
  if (!s_workout_layer) return;
  layer_set_hidden(s_workout_layer, !visible);
  layer_set_hidden(text_layer_get_layer(s_title_layer), visible);
  layer_set_hidden(text_layer_get_layer(s_exercise_layer), visible);
  layer_set_hidden(text_layer_get_layer(s_hint_layer), visible);
  if (visible) layer_mark_dirty(s_workout_layer);
}

static const char *home_label(uint8_t item) {
  if (item == 0) return s_state.active ? "Continue" : "New Workout";
  if (item == 1) return "Setup";
  if (item == 2) return "History";
  return "Progress";
}

static void show_home(void) { s_screen = SCREEN_HOME; s_setup = false; s_show_plates = false; update_display(); }
static void show_workout(void) { s_screen = SCREEN_WORKOUT; s_setup = false; update_display(); }

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
  s_state.completion_alerted = 0;
  s_state.rest_elapsed = 0;
}

static bool rest_values_valid(time_t now) {
  return s_state.rest_active && s_state.rest_start > 0 &&
      now >= s_state.rest_start && s_state.rest_elapsed <= (uint32_t)(now - s_state.rest_start);
}

static void rest_tick(struct tm *tick_time, TimeUnits units_changed) {
  time_t now = time(NULL);
  if (!rest_values_valid(now)) {
    clear_rest();
    save_state();
    update_display();
    return;
  }
  RestState rest = { .active=s_state.rest_active, .halfway_alerted=s_state.halfway_alerted, .completion_alerted=s_state.completion_alerted, .start=s_state.rest_start, .elapsed=s_state.rest_elapsed };
  int alerts = rest_alerts_due(&rest, (int32_t)now);
  s_state.rest_elapsed = rest.elapsed; s_state.halfway_alerted = rest.halfway_alerted; s_state.completion_alerted = rest.completion_alerted;
  if (alerts & 1) {
    vibes_short_pulse();
    save_state();
  }
  if (alerts & 2) {
    vibes_enqueue_custom_pattern(REST_COMPLETE_PATTERN);
    save_state();
  }
  update_display();
}

static void start_rest_services(void) {
  stop_rest_services();
  tick_timer_service_subscribe(SECOND_UNIT, rest_tick);
}

static const char *workout_name(WorkoutType workout) {
  return workout == WORKOUT_B ? "Workout B" : "Workout A";
}

static void save_state(void) {
  s_state.schema_version = STORAGE_SCHEMA_VERSION;
  persist_write_data(STORAGE_KEY_STATE, &s_state, sizeof(s_state));
}

static void sync_failed(DictionaryIterator *i, AppMessageResult result, void *ctx) { (void)i; (void)result; (void)ctx; s_sync_ready = false; s_sync_in_flight = false; sync_adapter_transport(&s_sync_adapter, false); }
static void sync_sent(DictionaryIterator *i, void *ctx) { (void)i; (void)ctx; }
static void sync_received(DictionaryIterator *i, void *ctx) {
  (void)ctx; Tuple *id = dict_find(i, MESSAGE_KEY_ack);
  if (!id) return;
  SyncRecord *r = (SyncRecord *)sync_queue_peek(&s_state.outbox);
  if (r && id->value->uint32 == r->id && sync_adapter_ack(&s_sync_adapter, r->id)) {
    uint32_t acknowledged_id = id->value->uint32;
    SyncPushResult result = workout_completion_handle_ack(&s_state, acknowledged_id);
    s_sync_in_flight = false;
    if (result == SYNC_PUSH_ADDED || result == SYNC_PUSH_IDENTICAL) { send_oldest(); resume_deferred_query(); }
    else if (result == SYNC_PUSH_FULL || result == SYNC_PUSH_CONFLICT) s_state.completion_blocked = 1;
    save_state(); update_display();
  }
}
static void query_received(DictionaryIterator *i) {
  Tuple *c=dict_find(i,MESSAGE_KEY_calendar_id), *p=dict_find(i,MESSAGE_KEY_progress_id);
  if (c) { Tuple *y=dict_find(i,MESSAGE_KEY_calendar_year),*m=dict_find(i,MESSAGE_KEY_calendar_month),*d=dict_find(i,MESSAGE_KEY_calendar_days),*x=dict_find(i,MESSAGE_KEY_calendar_mask),*a=dict_find(i,MESSAGE_KEY_calendar_mask_a),*b=dict_find(i,MESSAGE_KEY_calendar_mask_b); CalendarResponse r={c->value->uint16,y?y->value->uint16:0,m?m->value->uint8:0,d?d->value->uint8:0,x?x->value->uint32:0,a?a->value->uint32:(x?x->value->uint32:0),b?b->value->uint32:0}; if(calendar_response_valid(&r,s_query_id,s_calendar_year,s_calendar_month)){s_calendar=r;s_calendar_valid=true;s_query_connected=true;query_controller_response(&s_query_controller,true,true);if(s_query_timer){app_timer_cancel(s_query_timer);s_query_timer=NULL;}send_oldest();update_display();} return; }
  if (p) { Tuple *ex=dict_find(i,MESSAGE_KEY_progress_exercise),*pg=dict_find(i,MESSAGE_KEY_progress_page),*tt=dict_find(i,MESSAGE_KEY_progress_total),*ix=dict_find(i,MESSAGE_KEY_progress_chunk_index),*cc=dict_find(i,MESSAGE_KEY_progress_chunk_count),*pc=dict_find(i,MESSAGE_KEY_progress_point_count); ProgressPoint pts[5];uint8_t n=pc?pc->value->uint8:0;for(uint8_t z=0;z<n&&z<5;z++){Tuple *t=dict_find(i,MESSAGE_KEY_progress_t0+z),*w=dict_find(i,MESSAGE_KEY_progress_w0+z);if(!t||!w){n=6;break;}pts[z]=(ProgressPoint){t->value->int32,w->value->uint16};}if(ex&&pg&&tt&&ix&&cc&&n<=5&&progress_chunk_add(&s_progress_data,p->value->uint16,ex->value->uint8,pg->value->uint8,tt->value->uint8,ix->value->uint8,cc->value->uint8,n,pts)){if(progress_assembly_complete(&s_progress_data)){s_query_connected=true;query_controller_response(&s_query_controller,true,true);if(s_query_timer){app_timer_cancel(s_query_timer);s_query_timer=NULL;}send_oldest();update_display();}} }
}
static void inbox_received(DictionaryIterator *i, void *ctx) { sync_received(i,ctx); query_received(i); }
static void send_oldest(void) {
  if (!query_controller_can_sync(&s_query_controller)) return;
  const SyncRecord *r = sync_queue_peek(&s_state.outbox); if (!r || !s_sync_ready || s_sync_in_flight) return;
  int n = sync_record_to_json(r, s_sync_wire, sizeof s_sync_wire); if (n <= 0) return;
  s_sync_adapter.machine.head_id = r->id;
  s_sync_in_flight = sync_adapter_start(&s_sync_adapter);
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
        size_t index = workout_weight_index(s_state.active_workout, n);
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
  if (version == 9) {
    PersistedStateV9 old;
    if (persist_read_data(STORAGE_KEY_STATE, &old, sizeof old) == sizeof old &&
        migrate_v9_to_v10(&old, &s_state, (int32_t)time(NULL))) {
      save_state();
      if (s_state.rest_active) start_rest_services();
      return;
    }
  }
  if (persist_exists(STORAGE_KEY_STATE) &&
      persist_read_data(STORAGE_KEY_STATE, &s_state, sizeof(s_state)) == sizeof(s_state) &&
      s_state.schema_version == STORAGE_SCHEMA_VERSION && workout_state_valid(&s_state) &&
      s_state.completion_blocked <= 1 && s_state.selected_reps <= 5 &&
      s_state.next_workout <= WORKOUT_B && s_state.active_workout <= WORKOUT_B &&
      (!s_state.active || (s_state.exercise_index < 3 &&
       s_state.set_index < WORKOUTS[s_state.active_workout][s_state.exercise_index].sets)) &&
      s_state.active <= 1 && s_state.rest_active <= 1 && s_state.halfway_alerted <= 1 && s_state.completion_alerted <= 1 && valid_advisory_state() &&
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
  bool dedicated = s_screen == SCREEN_HISTORY || s_screen == SCREEN_PROGRESS_GRAPH;
  if (s_title_layer) layer_set_hidden(text_layer_get_layer(s_title_layer), dedicated);
  if (s_exercise_layer) layer_set_hidden(text_layer_get_layer(s_exercise_layer), dedicated);
  if (s_hint_layer) layer_set_hidden(text_layer_get_layer(s_hint_layer), dedicated);
  if (s_history_progress_layer) layer_set_hidden(s_history_progress_layer, !dedicated);
  if (dedicated) { text_layer_set_text(s_title_layer, ""); text_layer_set_text(s_exercise_layer, ""); text_layer_set_text(s_hint_layer, ""); }
  set_workout_layer_visible(s_state.active && !s_state.warmup_active && s_screen == SCREEN_WORKOUT);
  if (s_screen == SCREEN_HOME) {
    text_layer_set_font(s_exercise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(s_title_layer, "StrongLifts");
    snprintf(s_exercise_text, sizeof s_exercise_text, "%s%s\n%s%s\n%s%s\n%s%s",
             s_home_item == 0 ? "> " : "  ", home_label(0),
             s_home_item == 1 ? "> " : "  ", home_label(1),
             s_home_item == 2 ? "> " : "  ", home_label(2),
             s_home_item == 3 ? "> " : "  ", home_label(3));
    text_layer_set_text(s_exercise_layer, s_exercise_text);
    snprintf(s_hint_text, sizeof s_hint_text, "Select: open");
    text_layer_set_text(s_hint_layer, s_hint_text);
    return;
  }
  if (s_screen == SCREEN_WORKOUT_SELECT) {
    text_layer_set_font(s_exercise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text(s_title_layer, "New Workout");
    snprintf(s_exercise_text, sizeof s_exercise_text, "%s%s\n%s%s",
             s_selected_workout == WORKOUT_A ? "> " : "  ", "Workout A",
             s_selected_workout == WORKOUT_B ? "> " : "  ", "Workout B");
    text_layer_set_text(s_exercise_layer, s_exercise_text);
    text_layer_set_text(s_hint_layer, "Up/Down: choose");
    return;
  }
  if (s_screen == SCREEN_HISTORY || s_screen == SCREEN_PROGRESS_GRAPH) {
    if (s_history_progress_layer) layer_mark_dirty(s_history_progress_layer);
    return;
  }
  if (s_screen == SCREEN_PROGRESS_PICKER) {
    text_layer_set_font(s_exercise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text(s_title_layer, s_screen == SCREEN_HISTORY ? "History" : (s_screen == SCREEN_PROGRESS_PICKER ? "Progress Picker" : "Progress Graph"));
    if (s_screen == SCREEN_PROGRESS_PICKER) { snprintf(s_exercise_text,sizeof s_exercise_text,"%s",SETUP_WEIGHT_NAMES[s_progress_exercise]); text_layer_set_text(s_exercise_layer,s_exercise_text); }
    else if (!s_query_connected) text_layer_set_text(s_exercise_layer, "Phone Needed");
    else if (s_screen == SCREEN_HISTORY && !s_calendar_valid) text_layer_set_text(s_exercise_layer, "No History");
    else if (s_screen == SCREEN_HISTORY) { snprintf(s_exercise_text,sizeof s_exercise_text,"%d/%d",s_calendar_month,s_calendar_year); text_layer_set_text(s_exercise_layer,s_exercise_text); }
    else if (s_screen == SCREEN_PROGRESS_GRAPH && progress_assembly_complete(&s_progress_data)) text_layer_set_text(s_exercise_layer, SETUP_WEIGHT_NAMES[s_progress_exercise]);
    else text_layer_set_text(s_exercise_layer, "Loading");
    text_layer_set_text(s_hint_layer, "Back: return");
    return;
  }
  text_layer_set_font(s_exercise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
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
    text_layer_set_font(s_exercise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    if (s_setup_mode == SETUP_MENU) {
      snprintf(s_exercise_text, sizeof s_exercise_text, "%s%s\n%s%s",
               s_setup_menu_index == 0 ? "> " : "  ", "Exercise Weights",
               s_setup_menu_index == 1 ? "> " : "  ", "Plate Inventory");
      text_layer_set_text(s_title_layer, "Setup");
      text_layer_set_text(s_hint_layer, "Up/Down: choose");
      text_layer_set_text(s_exercise_layer, s_exercise_text);
      return;
    }
    if (s_setup_mode == SETUP_WEIGHTS) {
      char weight[16]; weight_format(s_state.weights[s_weight_index], weight, sizeof weight);
      snprintf(s_exercise_text, sizeof s_exercise_text, "%s\n%s", SETUP_WEIGHT_NAMES[s_weight_index], weight);
      text_layer_set_text(s_title_layer, "Set Weight");
    } else {
      PlateInventory inventory = current_inventory(); char size[16]; weight_format(inventory.plates[s_plate_index].size, size, sizeof size);
      snprintf(s_exercise_text, sizeof s_exercise_text, "%s\nCount / side: %d", size, s_state.inventory_counts[s_plate_index]);
      text_layer_set_text(s_title_layer, "Set Plates");
    }
    text_layer_set_text(s_exercise_layer, s_exercise_text);
    snprintf(s_hint_text, sizeof s_hint_text, "Up/Down: adjust");
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
    uint32_t elapsed = (uint32_t)(time(NULL) - s_state.rest_start);
    snprintf(s_exercise_text, sizeof(s_exercise_text), "%lu:%02lu\n%s\nSet %d of %d",
             (unsigned long)(elapsed / 60), (unsigned long)(elapsed % 60),
             WORKOUTS[s_state.active_workout][s_state.exercise_index].name,
             s_state.set_index + 1, WORKOUTS[s_state.active_workout][s_state.exercise_index].sets);
    text_layer_set_text(s_title_layer, "");
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
    snprintf(s_exercise_text, sizeof(s_exercise_text), "%s\nSet %d of %d\n%s\n%d reps",
             current->name, s_state.set_index + 1, current->sets, weight, s_state.selected_reps);
    snprintf(s_hint_text, sizeof(s_hint_text), s_confirm_abandon ? "Select: abandon" : "Up: plates");
  }
  WorkoutSyncStatus sync_status = sync_completion_status(s_state.outbox.count, s_state.completion_blocked);
  if (sync_status == WORKOUT_SYNC_REQUIRED) snprintf(s_hint_text, sizeof s_hint_text, "Sync Required");
  else if (sync_status == WORKOUT_SYNC_NOT_SYNCED) snprintf(s_hint_text, sizeof s_hint_text, "Not Synced");
  text_layer_set_text(s_exercise_layer, s_exercise_text);
  text_layer_set_text(s_hint_layer, s_hint_text);
}

static void complete_set(void) {
  if (s_state.completion_blocked) { snprintf(s_feedback, sizeof s_feedback, "Sync Required"); update_display(); return; }
  if (s_state.warmup_active) {
    if (++s_state.warmup_index < s_state.warmup_plan.count) { save_state(); update_display(); return; }
    clear_warmup(); save_state();
    update_display(); return;
  }
  const ExerciseDefinition *current = &WORKOUTS[s_state.active_workout][s_state.exercise_index];
  if (s_state.exercise_index == 2 && s_state.set_index == current->sets - 1) {
    CompletionResult completion = workout_completion_attempt(&s_state, s_selected_reps, (int32_t)time(NULL));
    save_state();
    if (completion == COMPLETION_BLOCKED || completion == COMPLETION_ERROR) snprintf(s_feedback, sizeof s_feedback, "Sync Required");
    else { snprintf(s_feedback, sizeof s_feedback, "Workout saved"); send_oldest(); }
    update_display();
    return;
  }
  SyncCompletionRollback rollback;
  if (s_state.exercise_index == 2 && s_state.set_index == current->sets - 1)
    sync_completion_snapshot(&rollback, s_state.weights, s_state.failure_streaks,
      s_state.deload_pending, s_state.failure_reviewed, s_state.plateau_reviewed,
      s_state.gap_reviewed, s_state.active, s_state.exercise_index, s_state.set_index,
      s_state.next_workout, s_state.last_completed, s_state.pending_valid,
      &s_state.pending_record);
  if (!sync_completion_log_set(s_state.work_reps[s_state.exercise_index] + s_state.set_index,
      &s_selected_reps, &s_state.completion_blocked, s_state.outbox.count, s_state.pending_valid,
      s_state.exercise_index, s_state.set_index, 2, current->sets - 1)) {
    save_state(); snprintf(s_feedback, sizeof s_feedback, "Sync Required"); update_display(); return;
  }
  s_state.selected_reps = s_selected_reps;
  save_state();
  s_state.set_index++;
  bool next_set_same_exercise = s_state.set_index < current->sets;
  if (next_set_same_exercise) {
    time_t now = time(NULL);
    s_state.rest_active = 1;
    s_state.rest_start = (int32_t)now;
    s_state.rest_end = 0;
    s_state.rest_elapsed = 0;
    s_state.halfway_alerted = 0;
    s_state.completion_alerted = 0;
    save_state();
    start_rest_services();
    update_display();
    return;
  }
  if (s_state.set_index >= current->sets) {
    /* History placement is the commit point for a completed workout.  Keep a
     * complete rollback image while constructing and queueing the record so
     * allocation, encoding, or collision failures cannot partially commit
     * progression or advisory state. */
    /* Keep the local aliases while the failure branches are collapsed into
     * the shared restore helper in a subsequent cleanup. */
    Weight weights_before[5]; uint8_t streaks_before[5], pending_before[5];
    uint8_t failure_reviewed_before[5], plateau_reviewed_before[5], gap_reviewed_before[5];
    uint8_t active_before = s_state.active, exercise_before = s_state.exercise_index;
    uint8_t set_before = s_state.set_index, next_workout_before = s_state.next_workout;
    int32_t last_completed_before = s_state.last_completed;
    uint8_t pending_valid_before = s_state.pending_valid;
    SyncRecord pending_record_before = s_state.pending_record;
    memcpy(weights_before, s_state.weights, sizeof weights_before);
    memcpy(streaks_before, s_state.failure_streaks, sizeof streaks_before);
    memcpy(pending_before, s_state.deload_pending, sizeof pending_before);
    memcpy(failure_reviewed_before, s_state.failure_reviewed, sizeof failure_reviewed_before);
    memcpy(plateau_reviewed_before, s_state.plateau_reviewed, sizeof plateau_reviewed_before);
    memcpy(gap_reviewed_before, s_state.gap_reviewed, sizeof gap_reviewed_before);
    bool success = exercise_succeeded(s_state.work_reps[s_state.exercise_index], current->sets);
    size_t weight_index = workout_weight_index(s_state.active_workout, s_state.exercise_index);
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
      if (!allocate_record_id(&s_state, &record.id)) { memcpy(s_state.weights, weights_before, sizeof weights_before); memcpy(s_state.failure_streaks, streaks_before, sizeof streaks_before); memcpy(s_state.deload_pending, pending_before, sizeof pending_before); memcpy(s_state.failure_reviewed, failure_reviewed_before, sizeof failure_reviewed_before); memcpy(s_state.plateau_reviewed, plateau_reviewed_before, sizeof plateau_reviewed_before); memcpy(s_state.gap_reviewed, gap_reviewed_before, sizeof gap_reviewed_before); s_state.active=active_before; s_state.exercise_index=exercise_before; s_state.set_index=set_before; s_state.next_workout=next_workout_before; s_state.last_completed=last_completed_before; s_state.completion_blocked=1; s_state.pending_valid=pending_valid_before; s_state.pending_record=pending_record_before; save_state(); snprintf(s_feedback, sizeof s_feedback, "Sync Required"); update_display(); return; }
      uint8_t sets[3]; for (uint8_t e=0;e<3;e++) sets[e]=WORKOUTS[s_state.active_workout][e].sets;
      uint8_t deload_mask=0; for (uint8_t e=0;e<5;e++) if (s_state.deload_pending[e]) deload_mask |= (uint8_t)(1u << e);
      uint16_t snapshot[3]; for (uint8_t e=0;e<3;e++) snapshot[e]=(uint16_t)s_state.active_weights[e];
      if (!sync_completion_build_record(&record, record.id, s_state.active_workout, s_state.last_completed, snapshot, s_state.work_reps, sets, deload_mask)) { memcpy(s_state.weights, weights_before, sizeof weights_before); memcpy(s_state.failure_streaks, streaks_before, sizeof streaks_before); memcpy(s_state.deload_pending, pending_before, sizeof pending_before); memcpy(s_state.failure_reviewed, failure_reviewed_before, sizeof failure_reviewed_before); memcpy(s_state.plateau_reviewed, plateau_reviewed_before, sizeof plateau_reviewed_before); memcpy(s_state.gap_reviewed, gap_reviewed_before, sizeof gap_reviewed_before); s_state.active=active_before; s_state.exercise_index=exercise_before; s_state.set_index=set_before; s_state.next_workout=next_workout_before; s_state.last_completed=last_completed_before; s_state.completion_blocked=1; s_state.pending_valid=pending_valid_before; s_state.pending_record=pending_record_before; save_state(); snprintf(s_feedback,sizeof s_feedback,"Sync Required"); update_display(); return; }
      SyncPushResult result = sync_queue_push_result(&s_state.outbox, &record);
      if (result == SYNC_PUSH_FULL) { s_state.pending_record = record; s_state.pending_valid = 1; snprintf(s_feedback, sizeof s_feedback, "Sync Required"); }
      else if (result == SYNC_PUSH_ADDED || result == SYNC_PUSH_IDENTICAL) { save_state(); send_oldest(); }
      else { memcpy(s_state.weights, weights_before, sizeof weights_before); memcpy(s_state.failure_streaks, streaks_before, sizeof streaks_before); memcpy(s_state.deload_pending, pending_before, sizeof pending_before); memcpy(s_state.failure_reviewed, failure_reviewed_before, sizeof failure_reviewed_before); memcpy(s_state.plateau_reviewed, plateau_reviewed_before, sizeof plateau_reviewed_before); memcpy(s_state.gap_reviewed, gap_reviewed_before, sizeof gap_reviewed_before); s_state.active=active_before; s_state.exercise_index=exercise_before; s_state.set_index=set_before; s_state.next_workout=next_workout_before; s_state.last_completed=last_completed_before; s_state.completion_blocked=1; s_state.pending_valid=pending_valid_before; s_state.pending_record=pending_record_before; save_state(); snprintf(s_feedback, sizeof s_feedback, "Sync Required"); update_display(); return; }
      s_state.next_workout = s_state.active_workout == WORKOUT_A ? WORKOUT_B : WORKOUT_A;
      s_saved = true;
    } else generate_warmup();
  }
  save_state();
  update_display();
}

static void select_click(ClickRecognizerRef recognizer, void *context) {
  if (s_screen == SCREEN_HOME) {
    if (s_home_item == 0) {
      if (s_state.active) { show_workout(); return; }
      s_selected_workout = s_state.next_workout; s_screen = SCREEN_WORKOUT_SELECT; update_display(); return;
    }
    if (s_home_item == 1) { s_screen = SCREEN_SETUP; s_setup = true; s_setup_mode = SETUP_MENU; s_weight_index = 0; s_plate_index = 0; update_display(); return; }
    s_screen = s_home_item == 2 ? SCREEN_HISTORY : SCREEN_PROGRESS_PICKER; s_query_connected=false;
    if (s_screen==SCREEN_HISTORY) { time_t now=time(NULL); struct tm *tm=localtime(&now); s_calendar_year=tm->tm_year+1900; s_calendar_month=tm->tm_mon+1; query_send("calendar_request"); }
    else { s_progress_exercise=0; s_progress_page=0; update_display(); } return;
  }
  if (s_screen == SCREEN_PROGRESS_PICKER) { s_screen=SCREEN_PROGRESS_GRAPH; s_progress_page=0; progress_assembly_reset(&s_progress_data); s_query_connected=false; query_send("progress_request"); update_display(); return; }
  if (s_screen == SCREEN_WORKOUT_SELECT) { s_state.next_workout = s_selected_workout; s_screen = SCREEN_WORKOUT; }
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
    if (s_setup_mode == SETUP_MENU) {
      if (s_setup_menu_index == 0) { s_setup_mode = SETUP_WEIGHTS; s_weight_index = 0; }
      else { s_setup_mode = SETUP_PLATES; s_plate_index = 0; }
    } else if (s_setup_mode == SETUP_WEIGHTS) {
      if (++s_weight_index >= 5) s_setup_mode = SETUP_MENU;
    } else {
      if (++s_plate_index >= PLATE_MAX_SIZES) s_setup_mode = SETUP_MENU;
    }
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
    complete_set();
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
    s_screen = SCREEN_WORKOUT;
    generate_warmup();
    save_state();
    update_display();
  } else {
    complete_set();
  }
  if (s_history_progress_layer) layer_mark_dirty(s_history_progress_layer);
}

static void up_click(ClickRecognizerRef recognizer, void *context) {
  if (s_screen == SCREEN_HOME) { if (s_home_item > 0) s_home_item--; update_display(); return; }
  if (s_screen == SCREEN_WORKOUT_SELECT) { s_selected_workout = s_selected_workout == WORKOUT_A ? WORKOUT_B : WORKOUT_A; update_display(); return; }
  if (s_screen == SCREEN_HISTORY) { if (--s_calendar_month<1){s_calendar_month=12;s_calendar_year--;} query_send("calendar_request"); update_display(); return; }
  if (s_screen == SCREEN_PROGRESS_PICKER) { if(s_progress_exercise) s_progress_exercise--; update_display(); return; }
  if (s_screen == SCREEN_PROGRESS_GRAPH) { if (s_progress_page) s_progress_page--; query_send("progress_request"); update_display(); return; }
  if (s_deload) {
    PlateInventory inventory = current_inventory();
    s_deload_adjusting = true;
    s_deload_weight = next_achievable_total(s_deload_weight, &inventory);
    update_display(); return;
  }
  if (s_setup) {
    if (s_setup_mode == SETUP_MENU) s_setup_menu_index = s_setup_menu_index == 0 ? 1 : 0;
    else if (s_setup_mode == SETUP_WEIGHTS) { PlateInventory inventory = current_inventory(); Weight old = s_state.weights[s_weight_index]; Weight next = next_achievable_total(old, &inventory); s_state.weights[s_weight_index] = next; s_state.failure_streaks[s_weight_index] = failure_streak_after_manual_weight_change(s_state.failure_streaks[s_weight_index], old, next); }
    else if (s_state.inventory_counts[s_plate_index] < 2) {
      s_state.inventory_counts[s_plate_index]++;
    }
    save_state(); update_display(); return;
  }
  if (s_state.active && !s_confirm_abandon && !s_state.warmup_active) {
    s_selected_reps = workout_view_rep_up(s_selected_reps); s_state.selected_reps = s_selected_reps; save_state(); update_display();
  }
}

static void down_click(ClickRecognizerRef recognizer, void *context) {
  if (s_screen == SCREEN_HOME) { if (s_home_item < 3) s_home_item++; update_display(); return; }
  if (s_screen == SCREEN_WORKOUT_SELECT) { s_selected_workout = s_selected_workout == WORKOUT_A ? WORKOUT_B : WORKOUT_A; update_display(); return; }
  if (s_screen == SCREEN_HISTORY) { if (++s_calendar_month>12){s_calendar_month=1;s_calendar_year++;} query_send("calendar_request"); update_display(); return; }
  if (s_screen == SCREEN_PROGRESS_PICKER) { if(s_progress_exercise<4) s_progress_exercise++; update_display(); return; }
  if (s_screen == SCREEN_PROGRESS_GRAPH) { if (s_progress_page<255) s_progress_page++; query_send("progress_request"); update_display(); return; }
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
    } else if (s_state.active && !s_state.warmup_active && !s_show_plates && !s_confirm_abandon) {
      s_selected_reps = workout_view_rep_down(s_selected_reps); s_state.selected_reps = s_selected_reps; save_state(); update_display();
    } else if (!s_state.active && !s_saved) { s_setup = true; s_setup_item = 0; update_display(); }
    return;
  }
  if (s_setup_mode == SETUP_MENU) s_setup_menu_index = s_setup_menu_index == 0 ? 1 : 0;
  else if (s_setup_mode == SETUP_WEIGHTS) {
    { PlateInventory inventory = current_inventory(); Weight old = s_state.weights[s_weight_index]; Weight next = previous_achievable_total(old, &inventory); s_state.weights[s_weight_index] = next; s_state.failure_streaks[s_weight_index] = failure_streak_after_manual_weight_change(s_state.failure_streaks[s_weight_index], old, next); }
  } else if (s_state.inventory_counts[s_plate_index] > 0) {
    s_state.inventory_counts[s_plate_index]--;
  }
  { PlateInventory inventory = current_inventory(); bool changed = false;
    for (size_t n = 0; n < 5; n++) { Weight old = s_state.weights[n]; s_state.weights[n] = normalize_weight_down(old, &inventory); if (old != s_state.weights[n]) { changed = true; s_state.failure_streaks[n] = 0; } }
    save_state();
    s_weights_adjusted = changed;
  }
  update_display();
}

static void back_long_click(ClickRecognizerRef recognizer, void *context) {
  if (s_screen == SCREEN_SETUP) { back_click(recognizer, context); return; }
  if (s_deload) { s_deload = false; s_deload_adjusting = false; update_display(); return; }
  if (s_setup) { s_setup = false; update_display(); return; }
  if (s_state.active) {
    if (s_state.rest_active) { clear_rest(); save_state(); }
    clear_warmup(); s_state.completion_blocked = 0; s_state.selected_reps = 5; s_selected_reps = 5; save_state(); s_confirm_abandon = true;
    update_display();
  }
}

static void back_click(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer; (void)context;
  if (s_screen == SCREEN_HOME) { window_stack_pop_all(true); return; }
  if (s_screen == SCREEN_SETUP) {
    if (s_setup_mode == SETUP_MENU) { show_home(); return; }
    if (s_setup_mode == SETUP_WEIGHTS) {
      if (s_weight_index == 0) s_setup_mode = SETUP_MENU; else s_weight_index--;
    } else {
      if (s_plate_index == 0) s_setup_mode = SETUP_MENU; else s_plate_index--;
    }
    update_display(); return;
  }
  if (s_screen == SCREEN_PROGRESS_GRAPH) { query_cancel(); s_screen=SCREEN_PROGRESS_PICKER; update_display(); return; }
  if (s_screen == SCREEN_PROGRESS_PICKER || s_screen == SCREEN_HISTORY) { query_cancel(); show_home(); return; }
  if (s_screen != SCREEN_WORKOUT) { show_home(); return; }
  show_home();
}

static void up_long_click(ClickRecognizerRef recognizer, void *context) {
  if (!s_state.active && !s_saved) { s_setup = true; s_setup_item = 0; update_display(); }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 350, up_click);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 350, down_click);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click);
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
  s_history_progress_layer=layer_create(bounds); layer_set_update_proc(s_history_progress_layer,history_progress_draw); layer_add_child(root,s_history_progress_layer);
  s_workout_layer = layer_create(bounds);
  layer_set_update_proc(s_workout_layer, workout_layer_update);
  layer_add_child(root, s_workout_layer);
  layer_set_hidden(s_workout_layer, true);
  update_display();
}

static void window_unload(Window *window) {
  layer_destroy(s_workout_layer);
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_exercise_layer);
  text_layer_destroy(s_hint_layer);
  layer_destroy(s_history_progress_layer);
}

static void init(void) {
  query_controller_init(&s_query_controller);
  load_state();
  sync_adapter_init(&s_sync_adapter, sync_queue_peek(&s_state.outbox) ? sync_queue_peek(&s_state.outbox)->id : 0,
      sync_begin_adapter, sync_write_adapter, sync_send_adapter, sync_timer_adapter,
      sync_cancel_adapter, NULL);
  if (!sync_queue_valid(&s_state.outbox)) { s_state.outbox.count = 0; save_state(); }
  if (s_state.pending_valid && !sync_record_valid(&s_state.pending_record)) { s_state.pending_valid = 0; save_state(); }
  app_message_register_inbox_received(inbox_received); app_message_register_outbox_sent(sync_sent);
  /* Legacy readiness hook is intentionally disabled; Pebble has no readiness callback. */
#if 0
  AppMessageResult app_result = app_message_open(128, 128); s_sync_ready = app_result == APP_MSG_OK;
  if (s_sync_ready) send_oldest();
  app_message_register_outbox_failed(sync_failed); app_message_registerด_outbox_sent(sync_sent);
  app_message_open(128, 128); app_message_register_outbox_ready(sync_ready);
#endif
  app_message_register_outbox_failed(sync_failed);
  AppMessageResult app_result = app_message_open(128, 128); s_sync_ready = app_result == APP_MSG_OK;
  if (s_state.pending_valid) { SyncPushResult result = sync_completion_promote(&s_state.outbox, &s_state.pending_record, true); if (result == SYNC_PUSH_ADDED || result == SYNC_PUSH_IDENTICAL) { s_state.pending_valid = 0; save_state(); } }
  if (s_sync_ready) send_oldest();
  s_window = window_create();
#ifdef STRONGLIFTS_VISUAL_FIXTURES
  load_visual_fixture();
#endif
  window_set_background_color(s_window, palette_background());
  window_set_click_config_provider(s_window, click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers){.load = window_load, .unload = window_unload});
  window_stack_push(s_window, true);
}

static void deinit(void) { query_cancel(); stop_rest_services(); sync_adapter_deinit(&s_sync_adapter); s_sync_ack_timer = NULL; window_destroy(s_window); }

int main(void) {
  init();
  app_event_loop();
  deinit();
}
