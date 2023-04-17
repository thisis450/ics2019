#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
return 0;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	unsigned long long num;
	int i, base;
	char * str;
	const char *s;

	int flags;		/* 用在number()函数的标志 */
	
	int field_width;	/* 输出字段的宽度 */
	//精度；用在浮点数时表示输出小数点后几位；用在字符串时表示输出字符个数 
    int precision;		
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */
	                        /* 'z' support added 23/7/1999 S.H.    */
				/* 'z' changed to 'Z' --davidm 1/25/99 */
    
    
	/*将字符逐个放到输出缓冲区中，直到遇到第一个%*/
	for (str=buf ; *fmt ; ++fmt) 
    {
		if (*fmt != '%') 
        {    //寻找%
			*str++ = *fmt;
			continue;
		}
        
		//遇到%后执行下面代码	
		/* process flags */
		flags = 0;
		repeat:
			++fmt;				//跳过第一个 '%'
			switch (*fmt) 		//判断%后面的字符，对格式运算符的标志的处理
            {		
				case '-': flags |= LEFT; goto repeat;//flags=10000(二进制，下面一样)
				case '+': flags |= PLUS; goto repeat;//flags=100
				case ' ': flags |= SPACE; goto repeat;//flags=1000
				case '#': flags |= SPECIAL; goto repeat;//flags=10 0000
				case '0': flags |= ZEROPAD; goto repeat;//flags=1
			}
		//对字段宽度的处理
		field_width = -1;
		if ('0' <= *fmt && *fmt <= '9')
			field_width = skip_atoi(&fmt);  //得到字段宽度
		else if (*fmt == '*')               //*表示可变宽度
        {             
			++fmt;
			field_width = va_arg(args, int);//获得表示字段宽度的参数，
            
			/*一般使用最后一个固定参数args来初始化这个函数，
			得到的下一个参数为第一个变参,即printf("%*d",a,b);中的a,
			这里a表示字段宽度。字符串为固定参数；变参函数至少要有一个固定参数。*/
			if (field_width < 0) //手动输入负数，左对齐，例如printf("%*d",-2,3);
            {	
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		// 获取精度 
		precision = -1;
		if (*fmt == '.') 
        {
			++fmt;	
			if ('0' <= *fmt && *fmt <= '9')
				precision = skip_atoi(&fmt);//获得精度
			else if (*fmt == '*') //可变精度，printf("%.*f",2,3.1415);-->3.14
            {
				++fmt;
				/* 获取表示精度的参数(以整数类型获取) */
				precision = va_arg(args, int);
			}
			if (precision < 0)//精度不能小于0
				precision = 0;
		}
	
		//获取转换修饰符,即%hd、%ld、%lld、%Lf...中的h、l、L、Z (ll用q代替)
		qualifier = -1;
		if (*fmt == 'l' && *(fmt + 1) == 'l') 
        {
			qualifier = 'q';//即ll
			fmt += 2;
		} else if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L'
			|| *fmt == 'Z') 
        {
			qualifier = *fmt;
			++fmt;
		}
	
		
		base = 10;//默认十进制
		//对c、s、p、n、%、o等做处理
		switch (*fmt) 
        {
		//转换格式符为%c
		case 'c':
	            //如果没有有‘-’，先输出字宽-1个空格再输出字符
			if (!(flags & LEFT))//如果没有'-'标记符
				while (--field_width > 0)
					*str++ = ' ';//根据字段输出空格' '
            /*获取字符参数时是先以int类型获取再强转为unsigned char，
                为了获取过程中保证精度不丢失。*/
			*str++ = (unsigned char) va_arg(args, int);
	        // 如果有'-'，先输出字符再填补空格,注意是先--的，所以实际空格会比输入的字段少1，在加上参数就刚好够宽度；
			while (--field_width > 0)
				*str++ = ' ';
			continue;
	
		//转换格式符为%s           
		case 's':
			s = va_arg(args, char *);//char*格式获取参数
			if (!s)                  //如果字符串不存在，则返回(NULL)
				s = "<NULL>";
                
	        /*如果字符串中字符个数大于精度，len为精度；
	        否则len为字符个数,即精度表示了字符串输出字符的个数*/
			len = strnlen(s, precision);
	
	         //处理'-',即printf("%-s","hello");
			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			continue;
	
	    //处理格式符%p       
		case 'p':
			if (field_width == -1) //如果没有设置字段宽度
            { 
                /*字宽为8或16(根据系统而定)，因为2个位表示一个直接；
                例如32位系统指针大小位4字节，oxFF FF FF FF,需要8个字宽才能存储*/
				field_width = 2*sizeof(void *);
				flags |= ZEROPAD;   //flags = 1;会在前面补0
			}
	        //转为16进制并存进缓冲区中
			str = number(str,
				(unsigned long) va_arg(args, void *), 16,
				field_width, precision, flags);
			continue;
	
		//buf为1024字节空间的输出缓冲区（静态char数组）
		case 'n':
			if (qualifier == 'l') {
				long * ip = va_arg(args, long *);
				*ip = (str - buf);//获取输出缓冲数组中的个数
			} else if (qualifier == 'Z') {
				size_t * ip = va_arg(args, size_t *);
				*ip = (str - buf);
			} else {
				int * ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;
	
		case '%':
			*str++ = '%';
			continue;
	
		/* integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;
	
		case 'X':
			flags |= LARGE;//小写转大写
		case 'x':  //十六进制
			base = 16;
			break;
	
		case 'd':	//十进制
		case 'i':
			flags |= SIGN;
		case 'u':	//无符号
			break;
	
		default:
			*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}
		if (qualifier == 'l') 
        {
			num = va_arg(args, unsigned long);
			if (flags & SIGN)
				num = (signed long) num;
		} else if (qualifier == 'q') 
        {
			num = va_arg(args, unsigned long long);
			if (flags & SIGN)
				num = (signed long long) num;
		} else if (qualifier == 'Z') 
        {
			num = va_arg(args, size_t);
		} else if (qualifier == 'h') 
        {
            //输出短整型时是先以整数来获取参数，再转为短整型输出，保证获取过程中精度不丢失
			num = (unsigned short) va_arg(args, int);
			if (flags & SIGN)
				num = (signed short) num;
		} else 
        {
            //没有特殊标志的格式符，一律先以无符号整型获取，再转为有符号整型
			num = va_arg(args, unsigned int);
			if (flags & SIGN)
				num = (signed int) num;
		}
        //转换为对应的个数再存到缓冲区中
		str = number(str, num, base, field_width, precision, flags);
	}
	*str = '\0';//最后以'\0'结束
	return str-buf;
}


int sprintf(char *buf, const char *fmt, ...) {
	va_list args;
	int i;

	va_start(args, fmt);
	i=vsprintf(buf,fmt,args);
	va_end(args);
	return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif
