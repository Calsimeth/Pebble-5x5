#ifndef QUERY_CONTROLLER_H
#define QUERY_CONTROLLER_H
#include <stdbool.h>
typedef enum { QUERY_IDLE, QUERY_DEFERRED, QUERY_SENDING, QUERY_WAITING_RESPONSE, QUERY_ASSEMBLING, QUERY_COMPLETE, QUERY_FAILED } QueryState;
typedef struct { QueryState state; unsigned short id; bool owns_outbox; } QueryController;
void query_controller_init(QueryController *q);
bool query_controller_defer(QueryController *q, bool sync_busy);
bool query_controller_begin(QueryController *q, unsigned short id);
bool query_controller_response(QueryController *q, bool valid, bool complete);
void query_controller_fail(QueryController *q);
bool query_controller_can_sync(const QueryController *q);
bool query_controller_deferred_resume(QueryController *q);
bool query_controller_deferred_timeout(QueryController *q);
#endif
