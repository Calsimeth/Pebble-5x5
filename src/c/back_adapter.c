#include "back_adapter.h"
void back_adapter_init(BackAdapter *a, BackAdapterAction s, BackAdapterAction l, BackAdapterAction r, void *c) { if(a)*a=(BackAdapter){s,l,r,c}; }
void back_adapter_single(BackAdapter *a) { if(a&&a->single)a->single(a->context); }
void back_adapter_long(BackAdapter *a) { if(a&&a->long_press)a->long_press(a->context); }
void back_adapter_long_release(BackAdapter *a) { if(a&&a->long_release)a->long_release(a->context); }
