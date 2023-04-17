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
int printf(const char *fmt, ...) {
return  0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
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
	/*将字符逐个放到输出缓冲区中，直到遇到第一个%*/
	while((c = *fmt++) != 0){
		
		if(c != '%'){
			*out++ = c;
			continue;
		}
		/* %%, just a formatted '%' */
		if(*fmt == '%'){
			*out++ = '%';
			continue;
		}
		//遇到格式化输出后执行下面代码
		flags = 0; //判断%后面的字符，对格式运算符的标志的处理
		while(1){
			if(*fmt == '-'){ flags |= LEFT; fmt++; continue;}//flags=10000下类似
			if(*fmt == '+'){ flags |= PLUS; fmt++; continue;}
			if(*fmt == ' '){ flags |= SPACE; fmt++; continue;}
			if(*fmt == '#'){ flags |= SPECIAL; fmt++; continue;}
			if(*fmt == '0'){ flags = ZERO; fmt++; continue;}
			break;
		}
		//对字段宽度的处理
		wide = -1;
		if(isdigit(*fmt)){
			wide = get_wide((const char **)(&fmt));//得到字段宽度
		}
		else if(*fmt == '*'){//*表示可变宽度
			wide = va_arg(ap, int32_t);
			fmt++;
		}
		// 获取精度 
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
		//获取转换修饰符,即%hd、%ld、%lld、%Lf...中的h、l、L、Z
		qualifier = -1;
		if((*fmt == 'h') || (*fmt == 'l')) qualifier = *fmt++;
		/* get format */
		switch(*fmt++){
			case 'i':
			case 'd'://十进制
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
				s = va_arg(ap, char*);//char*格式获取参数
				if(!s)//如果字符串不存在，则返回(NULL)
					s="<NULL>";
					/*如果字符串中字符个数大于精度，len为精度；
	        否则len为字符个数,即精度表示了字符串输出字符的个数*/
				len = strlen(s);
				if((precision >= 0) && (len > precision)) len = precision;
				//处理'-',即printf("%-s","hello");
				if(!(flags & LEFT)) while(len < wide--) *out++ = ' ';
				for(i = 0; i < len; i++) *out++ = *s++;
				while(len < wide--) *out++ = ' ';
				break;

			case 'c':
				//如果没有有‘-’，先输出字宽-1个空格再输出字符
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

#endif
