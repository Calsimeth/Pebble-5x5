#pragma once
#include <stdbool.h>
typedef enum { BACK_GESTURE_IDLE, BACK_GESTURE_DOWN, BACK_GESTURE_LONG } BackGestureState;
typedef void (*BackGestureAction)(void *context);
typedef struct { BackGestureState state; BackGestureAction short_action; BackGestureAction long_action; void *context; } BackGesture;
void back_gesture_init(BackGesture *, BackGestureAction, BackGestureAction, void *);
void back_gesture_down(BackGesture *); void back_gesture_timer(BackGesture *); void back_gesture_up(BackGesture *);
void back_gesture_single(BackGesture *); void back_gesture_reset(BackGesture *);
