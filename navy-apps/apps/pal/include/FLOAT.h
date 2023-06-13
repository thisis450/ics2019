#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "assert.h"

typedef int FLOAT;

static inline int F2int(FLOAT a) {
  int sig=a&0x80000000;
    a=a&0x7fffffff;
    a=a>>16;
    a=a|sig;
    return a;
}

static inline FLOAT int2F(int a) {
  int sig=a&0x80000000;
  a=a&0x7fffffff;
  a=a<<16;
  a=a|sig;
  return a;

}

static inline FLOAT F_mul_int(FLOAT a, int b) {
  int sig=(a&0x80000000)^(b&0x80000000);
  a=a&0x7fffffff,b=b&0x7fffffff;
  a=a*b;
  a=a|sig;
  return a;
  
}

static inline FLOAT F_div_int(FLOAT a, int b) {
int sig=(a&0x80000000)^(b&0x80000000);
a=a&0x7fffffff,b=b&0x7fffffff;
a=a/b;
a=a|sig;
return a;
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT Fsqrt(FLOAT);
FLOAT Fpow(FLOAT, FLOAT);

#endif
