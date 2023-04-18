#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
	va_list sArgv;
	int ret=0,val_d,len,tmp;
	unsigned val_u=0;
	char ds[35];
	char* val_s;
	va_start(sArgv,fmt);
	for (;;)
	{
		while (*fmt!='%'&&*fmt!='\0') _putc(*fmt++),++ret;
		if (*fmt=='\0') {_putc('\0');++ret;break;}
		switch (*(fmt+1))
		{
			case 'd':
			{
				val_d=va_arg(sArgv,int);
				if (val_d<0) _putc('-'),++ret;
				len=0;
				do
				{
					tmp=val_d%10;
					ds[++len]=(tmp<0?-tmp:tmp)+'0';
					val_d/=10;
				}while(val_d);
				for (int i=len;i>=1;--i) _putc(ds[i]);
				ret+=len;
				break;
			}
			case 's':
			{
				val_s=va_arg(sArgv,char*);
				while (*val_s!='\0') _putc(*val_s),val_s++;
				break;
			}
			case 'u':
			{
				val_u=va_arg(sArgv,unsigned);
				len=0;
				do
				{
					tmp=val_u%10;
					ds[++len]=tmp+'0';
					val_u/=10;
				}while(val_u);
				for (int i=len;i>=1;--i) _putc(ds[i]);
				ret+=len;
				break;
			}
			case 'x':
			{
				val_u=va_arg(sArgv,unsigned);
				len=0;
				do
				{
					tmp=val_u%16;
					ds[++len]=(tmp<10?tmp+'0':tmp-9+'A'-1);
					val_u/=16;
				}while(val_u);
				for (int i=len;i>=1;--i) _putc(ds[i]);
				ret+=len;
				break;
			}

		}
		fmt=fmt+2;
	}
	va_end(sArgv);
	return ret==0?-1:ret;
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
  va_list sArgv;
  int ret=0,val_d,len,tmp;
  char ds[15];
  char* val_s;
  va_start(sArgv,fmt);
  for (;;)
  {
	while (*fmt!='%'&&*fmt!='\0'&&n>0) *out=*fmt,++fmt,++out,++ret,--n;
	if (n==0) break;
	if (*fmt=='\0') {*out='\0';++ret;break;}
	if (*(fmt+1)=='d')
	{
		val_d=va_arg(sArgv,int);
		if (val_d<0) *out='-',++out,++ret,--n;
		if (n==0) break;
		len=0;
		do
		{
			tmp=val_d%10;
			ds[++len]=(tmp<0?-tmp:tmp)+'0';
			val_d/=10;
		}while (val_d!=0);
		for (int i=len;n>0&&i>=1;--i) *out=ds[i],++out,--n,++ret;
		++fmt;++fmt;
	}
	else if (*(fmt+1)=='s')
	{
		val_s=va_arg(sArgv,char*);
		while (*val_s!='\0'&&n>0) *out=*val_s,++val_s,++out,--n,++ret;
		++fmt;++fmt;
	}
  }
  va_end(sArgv);
  return ret==0?-1:ret;
}

#endif
