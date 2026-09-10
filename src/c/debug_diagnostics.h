#ifndef DEBUG_DIAGNOSTICS_H
#define DEBUG_DIAGNOSTICS_H
#ifdef STRONGLIFTS_DEBUG
#include <stdint.h>
void debug_diag_boot(void); void debug_diag_shutdown(void); void debug_diag_event(uint8_t,uint32_t,uint32_t); void debug_diag_clear(void); void debug_diag_render(char *,uint8_t,uint8_t);
#else
#define debug_diag_boot() ((void)0)
#define debug_diag_shutdown() ((void)0)
#define debug_diag_event(a,b,c) ((void)0)
#define debug_diag_clear() ((void)0)
#define debug_diag_render(a,b,c) ((void)0)
#endif
#endif
