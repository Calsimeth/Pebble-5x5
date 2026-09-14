#include "query_controller.h"
void query_controller_init(QueryController *q){if(q){q->state=QUERY_IDLE;q->id=0;q->owns_outbox=false;}}
bool query_controller_defer(QueryController *q,bool busy){if(!q)return false;if(busy){q->state=QUERY_DEFERRED;return true;}return false;}
bool query_controller_begin(QueryController *q,unsigned short id){if(!q||!id||q->state==QUERY_SENDING||q->state==QUERY_WAITING_RESPONSE||q->state==QUERY_ASSEMBLING)return false;q->id=id;q->state=QUERY_SENDING;q->owns_outbox=true;return true;}
bool query_controller_response(QueryController *q,bool valid,bool complete){if(!q||!q->owns_outbox||!valid)return false;if(!complete){q->state=QUERY_ASSEMBLING;return true;}q->state=QUERY_COMPLETE;q->owns_outbox=false;return true;}
void query_controller_fail(QueryController *q){if(q){q->state=QUERY_FAILED;q->owns_outbox=false;}}
bool query_controller_can_sync(const QueryController *q){return q&&!q->owns_outbox&&(q->state==QUERY_IDLE||q->state==QUERY_COMPLETE||q->state==QUERY_FAILED);}
bool query_controller_deferred_resume(QueryController *q){if(!q||q->state!=QUERY_DEFERRED)return false;q->state=QUERY_IDLE;return true;}
bool query_controller_deferred_timeout(QueryController *q){if(!q||q->state!=QUERY_DEFERRED)return false;q->state=QUERY_FAILED;q->owns_outbox=false;return true;}
