#include "back_gesture.h"
void back_gesture_init(BackGesture *g, BackGestureAction s, BackGestureAction l, void *c) { if(g)*g=(BackGesture){.short_action=s,.long_action=l,.context=c}; }
void back_gesture_down(BackGesture *g) { if(g)g->state=BACK_GESTURE_DOWN; }
void back_gesture_timer(BackGesture *g) { if(!g||g->state!=BACK_GESTURE_DOWN)return; g->state=BACK_GESTURE_LONG; if(g->long_action)g->long_action(g->context); }
void back_gesture_up(BackGesture *g) { if(g&&g->state==BACK_GESTURE_DOWN)g->state=BACK_GESTURE_IDLE; }
void back_gesture_single(BackGesture *g) { if(!g)return; if(g->state==BACK_GESTURE_LONG){g->state=BACK_GESTURE_IDLE;return;} if(g->short_action)g->short_action(g->context);g->state=BACK_GESTURE_IDLE; }
void back_gesture_reset(BackGesture *g) { if(g)g->state=BACK_GESTURE_IDLE; }
