#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, 
  TK_HEX,
  TK_DEC,
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
  {"0[xX][0-9A-Fa-f]+",TK_HEX},
  {"0|[1-9][0-9]*",TK_DEC},
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
            //printf("%s\n",tokens[nr_token].str);
            break;
          case TK_HEX:
            if(substr_len>=32)
            {
              printf("错误的表达式参数，超过了32字节！\n");
              return false;
            }
            strncpy(tokens[nr_token].str,substr_start+2,substr_len-2);
            *(tokens[nr_token].str+substr_len-2)='\0';
            //printf("%s\n",tokens[nr_token].str);
            break;
          case TK_REG:
            strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);
            *(tokens[nr_token].str+substr_len-1)='\0';
            //printf("%s\n",tokens[nr_token].str);
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
bool check_parentheses(int p,int q)
{
  if(p>=q)
  {
  printf("括号匹配出现了错误,p>=q的情况\n");
  return false;
  }
  if (tokens[p].type!='('||tokens[q].type!=')')
  {
    return false;
  }
  int un_left=0;
  for(int i=p;i<=q;i++)
  {
    if(tokens[i].type=='(')
    {
      un_left++;
    }
    if(tokens[i].type==')')
    {
      un_left--;
    }
    if(un_left==0&&i<q)
    {
      return false;
    }

  }
  if(un_left!=0)
  {
    return false;
  }
  return true;
}

int get_pr(int n)
{
  if(tokens[i].type=='*'||tokens[i].type=='/')
  return 3;
  if(tokens[i].type=='+'||tokens[i].type=='-')
  return 2;
  if(tokens[i].type==TK_EQ||tokens[i].type==TK_NEQ)
  return 1;
  if(tokens[i].type==TK_AND||tokens[i].type==TK_OR)
  return 0;
  return 999;
}
int find_dominant_operator(int p,int q)
{
  int i=0,left=0,pr=-100,position=p;
  for(i=p;i<=q;i++)
  {
    if(tokens[i].type=='(')
    {
      left++;
      i++;
      while(1)
      {
        if(tokens[i].type=='(')
        {
          left++;
        }
        if(tokens[i].type==')')
        {
          left--;
        }
        i++;
        if(left==0)
        {
          break;
        }
        if(i>q)
        {
          printf("括号匹配错误，错误的括号数量\n");
          assert(0);
        }
      }


    }

    if(tokens[i].type==TK_HEX||tokens[i].type==TK_DEC||tokens[i].type==TK_REG)
    {
      continue;
    }
    int temp_pr=get_pr(i);
    if(temp_pr<=pr)
    {
      position=i;
      pr=temp_Pr;
    }
    return position;
      

  }
  return 0;

}
int eval(int p,int q)
{
  if(p>q)
  {
    printf("表达式计算出现错了，p>q\n");
    assert(0);
  }
  else if(p==q)
  {int val;
    switch(tokens[p].type)
    {
      case TK_DEC:
      sscanf(tokens[p].str,"%d",&val);
      return val;
      case TK_HEX:
      sscanf(tokens[p].str,"%x",&val);
      return val;
      case TK_REG:
      for(int i=0;i<8;i++)
      {
        if(strcmp(tokens[p].str,regsl[i])==0)
        return reg_l(i);
        if(strcmp(tokens[p].str,regsw[i])==0)
        return reg_w(i);
        if(strcmp(tokens[p].str,regsb[i])==0)
        return reg_b(i);
        
      }
      if(strcmp(tokens[p].str,"pc")==0)
      {
        return cpu.pc;
      }
      else
      {
        printf("错误的寄存器名字%s\n",tokens[p].str);
        assert(0);
      }
      default:
      printf("expr计算中p=q出错,错误的值类型\n");
      assert(0);
    }
    
  }
  else if(check_parentheses(p,q)==true)
  {
    return eval(p+1,q-1);
  }
 else
 {
  int op=find_dominant_operator(p,q);
  int val1=eval(p,op-1);
  int val2=eval(op+1,q);
  switch(tokens[op].type)
  {
    case '+':
    return val1+val2;
    case '-':
    return val1-val2;
    case '*':
    return val1*val2;
    case '/':
    return val1/val2;
    case TK_AND:
    return val1&&val2;
    case TK_OR:
    return val1||Val2;
    case TK_EQ:
    return val1==val2;
    case TK_NEQ:
    return val1!=val2;
    default:
    printf("错误的运算符类型,%d\n",tokens[op].type);
    assert(0)
  }

 }
}
uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  return eval(0,nr_token-1);

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
