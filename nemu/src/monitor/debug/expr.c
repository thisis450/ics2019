#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, 
  TK_DEC,
  TK_HEX,
  TK_REG,
  TK_EQ,
  TK_NEQ,
  TK_AND,
  TK_OR


  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"0|[1-9][0-9]*",TK_DEC},
  {"0[xX][0-9A-Fa-f]+",TK_HEX},
  {"\\$(eax|ebx|ecx|edx|esp|ebp|esi|edi|pc|ax|bx|cx|dx|sp|bp|si|di|al|cl|dl|bl|ah|ch|dh|bh)",TK_REG},
  {"==", TK_EQ},
  {"!=", TK_NEQ},
  {"&&",TK_AND},
  {"\\|\\|",TK_OR},
  {"\\+", '+'},         
  {"-", '-'},         
  {"\\*", '*'},         
  {"\\/", '/'},
  {"\\(", '('},
  {"\\)", ')'}               
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case TK_DEC:
            if(substr_len>=32)
            {
              printf("错误的表达式参数，超过了32字节！\n");
              return false;
            }
            strncpy(tokens[nr_token].str,substr_start,substr_len);
            *(tokens[nr_token].str+substr_len)='\0';
            break;
          case TK_HEX:
            if(substr_len>=32)
            {
              printf("错误的表达式参数，超过了32字节！\n");
              return false;
            }
            strncpy(tokens[nr_token].str,substr_start+2,substr_len-2);
            *(tokens[nr_token].str+substr_len-2)='\0';
            printf("%s\n",tokens[nr_token].str);
            break;
          case TK_REG:
            strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);
            *(tokens[nr_token].str+substr_len-1)='\0';
            printf("%s\n",tokens[nr_token].str);
            break;


          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
