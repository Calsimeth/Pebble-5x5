#include "persistence.h"
enum { CORE0=10, SYNC0=11, CORE1=12, SYNC1=13, META=14 };
static uint32_t key(uint32_t b,uint8_t s){return b+s*2u;}
PersistenceResult persistence_save(const PersistenceAdapter*a,size_t cs,const void*c,size_t ss,const void*s,void*x){
 if(!a||!a->write||!c||!s||cs>PERSISTENCE_LIMIT||ss>PERSISTENCE_LIMIT)return PERSIST_TOO_LARGE;
 PersistenceMetadata m={0};uint8_t slot=0;if(a->read&&a->read(META,&m,sizeof m,x)==(int)sizeof m&&m.valid)slot=m.slot^1u;
 if(a->write(key(CORE0,slot),c,cs,x)!=(int)cs||a->write(key(SYNC0,slot),s,ss,x)!=(int)ss)return PERSIST_IO;
 PersistenceMetadata n={(m.valid?m.generation:0u)+1u,slot,1};return a->write(META,&n,sizeof n,x)==(int)sizeof n?PERSIST_OK:PERSIST_IO;
}
PersistenceResult persistence_load(const PersistenceAdapter*a,size_t cs,void*c,size_t ss,void*s,void*x,PersistenceMetadata*out){
 PersistenceMetadata m={0};if(!a||!a->read||!c||!s||cs>PERSISTENCE_LIMIT||ss>PERSISTENCE_LIMIT)return PERSIST_INVALID;
 if(a->read(META,&m,sizeof m,x)!=(int)sizeof m||!m.valid||m.slot>1)return PERSIST_INVALID;
 if(a->read(key(CORE0,m.slot),c,cs,x)!=(int)cs||a->read(key(SYNC0,m.slot),s,ss,x)!=(int)ss)return PERSIST_IO;
 if(out)*out=m;
 return PERSIST_OK;
}
