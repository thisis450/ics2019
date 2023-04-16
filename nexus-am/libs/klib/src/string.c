#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len=1;
  while (s[len-1]!='\0') ++len;
  return len-1;
}

char *strcpy(char* dst,const char* src) {
	char* pd=dst;
	while (*src!='\0') *pd=*src,++pd,++src;
	return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
	char* pd=dst;
	while (n>0&&*src!='\0') --n,*pd=*src,++pd,++src;
	while (n>0) --n,*pd='\0',++pd;
	return dst;
}

char* strcat(char* dst, const char* src) {
	char* pd=dst;
	while (*pd!='\0') ++pd;
	while (*src!='\0') *pd=*src,++pd,++src;
	*pd='\0';
	return dst;
}

int strcmp(const char* s1, const char* s2) {
	while (*s1!='\0'&&*s2!='\0'&&*s1==*s2) ++s1,++s2;
	if (*s1>*s2) return 1;
	else if (*s1<*s2) return -1;
	else return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) {
	while (n>0)
	{
		if (*s1=='\0'&&*s2=='\0') return 0;
		if (*s1!=*s2) return *s1>*s2?1:-1;
		--n;++s1;++s2;
	}
	return 0;
}

void* memset(void* v,int c,size_t n) {
	unsigned char val=c;
	char *ptr=(char *)v;
	while (n>0)
	{
		*ptr=val;
		++ptr;
		--n;
	}
	return v;
}

void* memcpy(void* out, const void* in, size_t n) {
	char *ptr=(char *)out,*pi=(char *)in;
	while (n>0)
	{
		*ptr=*pi;
		++ptr;++pi;--n;
	}
	return out;
}

int memcmp(const void* s1, const void* s2, size_t n){
	char *p1=(char *)s1,*p2=(char*)s2;
	while (n>0)
	{
		if (*p1!=*p2) return *p1>*p2?1:-1;
		--n;++p1;++p2;
	}
	return 0;
}

#endif
