#ifndef PERSISTENCE_H
#define PERSISTENCE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define PERSISTENCE_LIMIT 256
typedef struct { bool (*exists)(uint32_t,void*); int (*get_size)(uint32_t,void*); int (*read)(uint32_t,void*,size_t,void*); int (*write)(uint32_t,const void*,size_t,void*); } PersistenceAdapter;
typedef struct { uint32_t generation; uint8_t slot; uint8_t valid; } PersistenceMetadata;
typedef enum { PERSIST_OK=0, PERSIST_INVALID=-1, PERSIST_IO=-2, PERSIST_TOO_LARGE=-3 } PersistenceResult;
PersistenceResult persistence_save(const PersistenceAdapter*,size_t,const void*,size_t,const void*,void*);
PersistenceResult persistence_load(const PersistenceAdapter*,size_t,void*,size_t,void*,void*,PersistenceMetadata*);
uint32_t persistence_last_generation(void);
#endif
