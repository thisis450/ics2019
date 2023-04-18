#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)


static char int_buf[20]; // 最大的long long十进制长度为19
char* hex_digits = "0123456789abcdef";
// base表示是几进制, 返回长度
static int fill_int_buf(unsigned long long lval, unsigned base) {
  int cur_pos = 0;
  if(lval==0) {
    int_buf[0]='0';
    int_buf[1]='\0';
    return 1;
  }
  char tmp;
  while (lval > 0) {
    int_buf[cur_pos++] = hex_digits[(lval % base)]; // 由于10进制的digits与hex的digits相同, 所以用hex_digits即可
    lval /= base;
  }
  int left = 0;
  int right = cur_pos - 1; // 现在cur_pos是右开, 因此-1
  while (left < right) {
    tmp = int_buf[right];
    int_buf[right] = int_buf[left];
    int_buf[left] = tmp;
    ++left;
    --right;
  }
  int_buf[cur_pos]='\0';
  return cur_pos;
}

int allprintf(char *out, size_t size, const char *fmt, va_list ap) {
#define out_putc(out_putc_ch) if(out!=NULL) {\
          out[cur_pos++] = out_putc_ch;\
      } else {\
        cur_pos++;\
        _putc(out_putc_ch);\
      }
#define check_out_putc(out_putc_ch) if(cur_pos==size_minus_one) {\
goto return_label;\
} else {out_putc(out_putc_ch);}
  assert(size>0);
  size_t size_minus_one = size-1;
  size_t cur_pos = 0; // cur_pos指向out, j指向fmt
  const char* p;
  int ival;
  unsigned long long lval = 0; // 为了过编
  size_t int_buf_len;
  char tmp_ch;
  
  for (p = fmt; *p; ++p) {
    if (*p != '%') {
      check_out_putc(*p);
      continue;
    }

    // 处理其它情况
    ++p;
    switch (*p) {
    case 'c': {
      char ch = va_arg(ap, int);
      check_out_putc(ch);
      break;
    }
    case 's': {
      for (char* s = va_arg(ap, char*); *s; ++s) {
        check_out_putc(*s);
      }
      break;
    }
    case 'd': {
      ival = va_arg(ap, int);
      if (ival < 0) {
        check_out_putc('-');
        lval = (-ival);
      } else {
        lval = ival;
      }
      int_buf_len = fill_int_buf(lval, 10);
      for(size_t i = 0; i<int_buf_len; i++) {
        tmp_ch = int_buf[i];
        check_out_putc(tmp_ch);
      }
      break;
    }
    case 'u': {
      lval = va_arg(ap, unsigned int);
      int_buf_len = fill_int_buf(lval, 10);
      for(size_t i = 0; i<int_buf_len; i++) {
        tmp_ch = int_buf[i];
        check_out_putc(tmp_ch);
      }
      break;
    }
    case 'x': {
      lval = va_arg(ap, unsigned int);
      int_buf_len = fill_int_buf(lval, 16);
      for(size_t i = 0; i<int_buf_len; i++) {
        tmp_ch = int_buf[i];
        check_out_putc(tmp_ch);
      }
      break;
    }
    default: {
      return -1;
    }
    }
  }
return_label:
  out_putc('\0');
  return (cur_pos-1); // 因为cur_pos包括\0
#undef out_putc
#undef check_out_putc
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  allprintf(NULL, 0xffffffffu, fmt, ap);
  va_end(ap);
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return 0;
}

int sprintf(char* out, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = allprintf(out, 0xffffffffu, fmt, ap);
  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = allprintf(out, n, fmt, ap);
  va_end(ap);
  return ret;
}

#endif