#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  int sig=(a&0x8000000)^(b&0x80000000);
  a=a&0x7fffffff,b=b&0x7fffffff;
  FLOAT result=((int64_t)a*(int64_t)b)>>16;
  result=result|sig;
  return result;
  
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  assert(b!=0);
  int sig=(a&0x8000000)^(b&0x80000000);
  a=a&0x7fffffff,b=b&0x7fffffff;
  int in=a/b;
  int fl=a%b;
  FLOAT res=in<<16;
  for(int i=0;i<16;i++)
  {
    fl<<=1;
    if(fl>=b)
    {
      fl-=b;
      res+=1<<(16-i-1);
    }
  }
  res=res|sig;
  return res;


}

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */

  uint32_t content=*(uint32_t*)&a;
  uint32_t sig=(content&0x80000000);
  uint32_t exp=(content^sig)>>23;
  uint32_t m=(content&0x007fffff);
  if(exp==0)
  {
    return 0;
  }
  else if(exp==255)
  {
    assert(0);
  }
  else
  {
    m+=1<<23;
    m>>=(23-16);
    if(exp>=127)
    {
      m<<=(exp-127);
    }
    else
    {
      m>>=(127-exp);
    }
  }
  m=m|sig;
  return m;



}

FLOAT Fabs(FLOAT a) {
  return (a>0?a:-a);
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
