#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define isdigit(x) (((x) >= '0') && ((x) <= '9'))
#define todigit(x) ((x) - '0')
#define LEFT 0x01
#define PLUS 0x02
#define SPACE 0x04
#define SPECIAL 0x08
#define ZERO 0x10
#define SIGN 0x20	 /* signed if set */
#define SMALL 0x40 /* 'abcdef' if set, 'ABCDEF' otherwise */

int32_t get_wide(const char **s);
void number_to_string(long num, int32_t base, int32_t flags, int32_t wide, int32_t precision, char **s);

int printf(const char *fmt, ...) {
  va_list ap;
	va_start(ap, fmt);
	char out[2000];
	int len = vsprintf(out, fmt, ap);
	for(int i = 0;i < len; i++){
		_putc(out[i]);
	}
	return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char c;
	char *start = out;
	int32_t flags;
	int32_t wide;
	int32_t precision;
	int32_t qualifier;
	char *s;
	int32_t i, len, base;
	while((c = *fmt++) != 0){
		/* normal string */
		if(c != '%'){
			*out++ = c;
			continue;
		}
		/* %%, just a formatted '%' */
		if(*fmt == '%'){
			*out++ = '%';
			continue;
		}
		/* % used to indicate a formatted string */
		flags = 0; /* get flags */
		while(1){
			if(*fmt == '-'){ flags |= LEFT; fmt++; continue;}
			if(*fmt == '+'){ flags |= PLUS; fmt++; continue;}
			if(*fmt == ' '){ flags |= SPACE; fmt++; continue;}
			if(*fmt == '#'){ flags |= SPECIAL; fmt++; continue;}
			if(*fmt == '0'){ flags = ZERO; fmt++; continue;}
			break;
		}
		/* get wide */
		wide = -1;
		if(isdigit(*fmt)){
			wide = get_wide((const char **)(&fmt)); /* read from the range of fmt which indicates the width */
		}
		else if(*fmt == '*'){
			wide = va_arg(ap, int32_t);
			fmt++;
		}
		/* get precision */
		precision = -1;
		if(*fmt == '.'){
			fmt++;
			if(isdigit(*fmt)){
				precision = get_wide((const char **)(&fmt));
			}
			else if(*fmt == '*'){
				precision = va_arg(ap, int32_t);
				fmt++;
			}
			else precision = 0;
		}
		/* get qualifier */
		qualifier = -1;
		if((*fmt == 'h') || (*fmt == 'l')) qualifier = *fmt++;
		/* get format */
		switch(*fmt++){
			case 'i':
			case 'd':
				flags |= SIGN;
				if(precision != -1) flags &= ~ZERO;
				switch(qualifier){
					case 'h':
						number_to_string((short)va_arg(ap, int32_t), 10, flags, wide, precision, &out);
						break;
					case 'l':
						number_to_string(va_arg(ap, long), 10, flags, wide, precision, &out);
						break;
					default:
						number_to_string(va_arg(ap, int32_t), 10, flags, wide, precision, &out);
						break;
				}
				break;
			case 'u':
				base = 10;
				goto num_to_str_without_sign;
			case 'o':
				base = 8;
				goto num_to_str_without_sign;
			case 'x':
				flags |= SMALL;
			case 'X':
				base = 16;

				num_to_str_without_sign:
				flags &= (~PLUS & ~SPACE);
				if(precision != -1) flags &= ~ZERO;
				switch(qualifier){
					case 'h':
						number_to_string((unsigned short)va_arg(ap, int32_t), base, flags, wide, precision, &out);
						break;
					case 'l':
						number_to_string((unsigned long)va_arg(ap, long), base, flags, wide, precision, &out);
						break;
					default:
						number_to_string((uint32_t)va_arg(ap, int32_t), base, flags, wide, precision, &out);
						break;
				}
				break;

			case 's':
				s = va_arg(ap, char*);
				len = strlen(s);
				if((precision >= 0) && (len > precision)) len = precision;
				/* right justified : pad with spaces */
				if(!(flags & LEFT)) while(len < wide--) *out++ = ' ';
				for(i = 0; i < len; i++) *out++ = *s++;
				/* left justified : pad with spaces */
				while(len < wide--) *out++ = ' ';
				break;

			case 'c':
				/* right justified : pad with spaces */
				if(!(flags & LEFT)) while(1 < wide--) *out++ = ' ';
				*out++ = (unsigned char)va_arg(ap, int32_t);
				/* left justified : pad with spaces */
				while(1 < wide--) *out++ = ' ';
				break;
			default:
				return -1;
		}
	}
	*out = 0;
	return (int32_t)(out - start);
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
	int32_t res;
	va_start(args, fmt);
	res = vsprintf(out, fmt, args);
	va_end(args);
	return res;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

int32_t get_wide(const char **s){
	int32_t res = 0;
	while(isdigit(**s)){
		res = 10 * res + todigit(*(*s++));
	}
	return res;
}

#define LONG_STRSIZE_BASE_2 32
void number_to_string(long num, int32_t base, int32_t flags, int32_t wide, int32_t precision, char **s)
{
	char sign; /* sign printed : '+', '-', ' ', or 0 (no sign) */
	int32_t num_cpy = num;
	unsigned long ul_num = (unsigned long)num; /* for unsigned format */

	/* string representation of num (reversed) */
	char tmp[LONG_STRSIZE_BASE_2];
	int32_t i = 0; /* number of figures in tmp */

	const char *digits = "0123456789ABCDEF";
	if (flags & SMALL)
		digits = "0123456789abcdef";

	if ((base < 2) || (base > 16))
		return;

	if ((flags & SIGN) && (num < 0))
	{
		sign = '-';
		num = -num;
	}
	else
		sign = (flags & PLUS) ? '+' : ((flags & SPACE) ? ' ' : 0);
	if (sign)
		wide--;

	if (flags & SPECIAL)
	{
		if ((base == 16) && (num != 0))
			wide -= 2; /* '0x' or '0X' */
		if (base == 8)
		{
			wide--;
			precision--;
		} /* '0' */
	}

	if (num == 0)
		tmp[i++] = '0';
	/* signed format */
	if (flags & SIGN)
	{
		while (num != 0)
		{
			tmp[i++] = digits[num % base];
			num = num / base;
		}
	}
	/* unsigned format */
	else
	{
		while (ul_num != 0)
		{
			tmp[i++] = digits[ul_num % base];
			ul_num = ul_num / base;
		}
	}

	if (i > precision)
		precision = i;
	wide -= precision;

	/* wide = number of padding chars */
	/* precision = number of figures after the sign and the special chars */

	/* right justified and no zeropad : pad with spaces */
	if (!(flags & (LEFT + ZERO)))
		while (wide-- > 0)
			*((*s)++) = ' ';

	if (sign)
		*((*s)++) = sign;
	if ((flags & SPECIAL) && (num_cpy != 0))
	{
		if (base == 8)
			*((*s)++) = '0';
		if (base == 16)
		{
			*((*s)++) = '0';
			if (flags & SMALL)
				*((*s)++) = 'x';
			else
				*((*s)++) = 'X';
		}
	}

	/* rigth justified and zeropad : pad with 0 */
	if (!(flags & LEFT))
		while (wide-- > 0)
			*((*s)++) = '0';

	/* print num */
	while (i < precision--)
		*((*s)++) = '0';
	while (i-- > 0)
		*((*s)++) = tmp[i];

	/* left justfied : pad with spaces */
	while (wide-- > 0)
		*((*s)++) = ' ';
}
#endif