#include <assert.h>
#include "../src/c/back_gesture.h"
static int s,l; static void sa(void*c){(void)c;s++;} static void la(void*c){(void)c;l++;}
int main(void){BackGesture g;back_gesture_init(&g,sa,la,0);s=l=0;back_gesture_down(&g);back_gesture_up(&g);back_gesture_single(&g);assert(s==1&&l==0);s=l=0;back_gesture_down(&g);back_gesture_timer(&g);back_gesture_up(&g);back_gesture_single(&g);assert(s==0&&l==1);back_gesture_timer(&g);back_gesture_up(&g);assert(s==0&&l==1);back_gesture_down(&g);back_gesture_up(&g);back_gesture_single(&g);assert(s==1&&l==1);back_gesture_down(&g);back_gesture_reset(&g);back_gesture_timer(&g);assert(s==1&&l==1);return 0;}
