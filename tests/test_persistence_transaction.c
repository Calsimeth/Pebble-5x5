#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/c/persistence.h"
typedef struct { unsigned char data[32][256]; size_t size[32]; unsigned op,fail; } Store;
static bool exists(uint32_t k,void*v){Store*s=v;return k<32&&s->size[k]!=0;}
static int gs(uint32_t k,void*v){Store*s=v;return k<32?(int)s->size[k]:-1;}
static int rd(uint32_t k,void*p,size_t n,void*v){Store*s=v;if(++s->op==s->fail)return -1;if(k>=32||s->size[k]!=n)return -1;memcpy(p,s->data[k],n);return(int)n;}
static int wr(uint32_t k,const void*p,size_t n,void*v){Store*s=v;if(++s->op==s->fail)return -1;if(k>=32||n>256)return-1;memcpy(s->data[k],p,n);s->size[k]=n;return(int)n;}
static const PersistenceAdapter a={exists,gs,rd,wr};
int main(void){Store s={0};unsigned char c[8]={1,2,3},q[9]={4,5,6},outc[8],outq[9];PersistenceMetadata m;
 s.fail=0;s.op=0;assert(persistence_save(&a,8,c,9,q,&s)==PERSIST_OK);assert(persistence_load(&a,8,outc,9,outq,&s,&m)==PERSIST_OK&&m.generation==1);
 for(unsigned f=1;f<=4;f++){Store t=s;t.fail=f;t.op=0;assert(persistence_save(&a,8,c,9,q,&t)==PERSIST_IO);assert(persistence_load(&a,8,outc,9,outq,&t,&m)==PERSIST_OK);assert(outc[0]==1&&outq[0]==4&&m.generation==1);}
 s.fail=0;s.op=0;assert(persistence_save(&a,8,c,9,q,&s)==PERSIST_OK);assert(persistence_load(&a,8,outc,9,outq,&s,&m)==PERSIST_OK&&m.generation==2);
 puts("transaction matrix passed: metadata read, core write, sync write, commit write");return 0;}
