#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return 0;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list sArgv;
  int ret=0,val_d,len,tmp;
  char ds[15];
  char* val_s;
  va_start(sArgv,fmt);
  for (;;)
  {
	while (*fmt!='%'&&*fmt!='\0') *out=*fmt,++fmt,++out,++ret;
	if (*fmt=='\0') {*out='\0';break;}
	if (*(fmt+1)=='d')
	{
		val_d=va_arg(sArgv,int);
		if (val_d<0) *out='-',++out,++ret;
		len=0;
		do
		{
			tmp=val_d%10;
			ds[++len]=(tmp<0?-tmp:tmp)+'0';
			val_d/=10;
		}while (val_d!=0);
		for (int i=len;i>=1;--i) *out=ds[i],++out;
		ret+=len;
		fmt=fmt+2;
	}
	else if (*(fmt+1)=='s')
	{
		val_s=va_arg(sArgv,char*);
		while (*val_s!='\0') *out=*val_s,++val_s,++out,++ret;
		fmt=fmt+2;
	}
  }
  va_end(sArgv);
  return ret==0?-1:ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif
