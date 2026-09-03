#pragma once
typedef void (*BackAdapterAction)(void *context);
typedef struct { BackAdapterAction single; BackAdapterAction long_press; BackAdapterAction long_release; void *context; } BackAdapter;
void back_adapter_init(BackAdapter *, BackAdapterAction, BackAdapterAction, BackAdapterAction, void *);
void back_adapter_single(BackAdapter *); void back_adapter_long(BackAdapter *); void back_adapter_long_release(BackAdapter *);
