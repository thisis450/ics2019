#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

char *strcpy(char* dst,const char* src) {
	char *tmp = dst;

	while ((*dst++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}

char* strncpy(char* dst, const char* src, size_t n) {
	char *tmp = dst;

	while (n) {
		if ((*tmp = *src) != 0)
			src++;
		tmp++;
		n--;
	}
	return dst;
}

char* strcat(char* dst, const char* src) {
	char *tmp = dst;

	while (*dst)
		dst++;
	while ((*dst++ = *src++) != '\0')
		;
	return tmp;
}

int strcmp(const char* s1, const char* s2) {
	unsigned char c1, c2;

	while (1) {
		c1 = *s1++;
		c2 = *s2++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) {
	unsigned char c1, c2;

	while (n) {
		c1 = *s1++;
		c2 = *s2++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
		n--;
	}
	return 0;
}

void* memset(void* v,int c,size_t n) {
	char *xs = v;

	while (n--)
		*xs++ = c;
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
