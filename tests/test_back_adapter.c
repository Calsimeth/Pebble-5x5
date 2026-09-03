#include <assert.h>
#include "../src/c/back_adapter.h"
static int s,l,r; static void S(void*c){(void)c;s++;} static void L(void*c){(void)c;l++;} static void R(void*c){(void)c;r++;}
int main(void){BackAdapter a;back_adapter_init(&a,S,L,R,0);back_adapter_single(&a);assert(s==1&&l==0);back_adapter_long(&a);back_adapter_long_release(&a);assert(s==1&&l==1&&r==1);back_adapter_single(&a);assert(s==2);return 0;}
