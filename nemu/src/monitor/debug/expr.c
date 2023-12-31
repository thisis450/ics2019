#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
const char *regsl_c[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *regsw_c[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *regsb_c[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
enum {
  TK_NOTYPE = 256, 
  TK_HEX,
  TK_DEC,
  TK_REG,
  TK_EQ,
  TK_NEQ,
  TK_AND,
  TK_OR,
  TK_DEREF


  /* TODO: Add more token types */

};
uint32_t hex_cal(char ch)
{
	return (('a'<=ch&&ch<='f')?ch-'a'+10:(('A'<=ch&&ch<='F')?ch-'A'+10:ch-'0'));
}
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
              Log("错误的表达式参数，超过了32字节！\n");
              return false;
            }
            strncpy(tokens[nr_token].str,substr_start,substr_len);
            *(tokens[nr_token].str+substr_len)='\0';
            tokens[nr_token].type=TK_DEC;
            nr_token++;
            //printf("%s\n",tokens[nr_token].str);
            break;
          case TK_HEX:
            if(substr_len>=32)
            {
              Log("错误的表达式参数，超过了32字节！\n");
              return false;
            }
            strncpy(tokens[nr_token].str,substr_start+2,substr_len-2);
            *(tokens[nr_token].str+substr_len-2)='\0';
            tokens[nr_token].type=TK_HEX;
            nr_token++;
            //printf("%s\n",tokens[nr_token].str);
            break;
          case TK_REG:
            strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);
            *(tokens[nr_token].str+substr_len-1)='\0';
            //printf("%s\n",tokens[nr_token].str);
            tokens[nr_token].type=TK_REG;
            nr_token++;
            break;
          case '(':
          tokens[nr_token++].type='(';
          break;
          case ')':
          tokens[nr_token++].type=')';
          break;
          case '+':
          tokens[nr_token++].type='+';
          break;
          case '-':
          tokens[nr_token++].type='-';
          break;
          case '*':
          tokens[nr_token++].type='*';
          break;
          case '/':
          tokens[nr_token++].type='/';
          break;
          case TK_EQ:
          tokens[nr_token++].type=TK_EQ;
          break;
          case TK_NEQ:
          tokens[nr_token++].type=TK_NEQ;
          break;
          case TK_AND:
          tokens[nr_token++].type=TK_AND;
          break;
          case TK_OR:
          tokens[nr_token++].type=TK_OR;
          break;
          
          

          //default: TODO();
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
  Log("括号匹配出现了错误,p>=q的情况\n");
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

int get_pr(int i)
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
  int i=0,left=0,pr=100,position=p;
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
          Log("括号匹配错误，错误的括号数量\n");
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
      pr=temp_pr;
    }
  }
  return position;

}
uint32_t eval(int p,int q)
{uint32_t val=0,len=strlen(tokens[p].str);
  if(p>q)
  {
    Log("表达式计算出现错了，p>q\n");
    assert(0);
  }
  else if(p==q)
  {
    switch(tokens[p].type)
    {
      case TK_DEC:
      for (int i=0;i<len;i++) val=val*10+tokens[p].str[i]-'0';
      Log("%d到%d为10进制，计算结果为%u\n",p,q,val);
      return val;
      case TK_HEX:
      for (int i=0;i<len;++i) val=val*16+hex_cal(tokens[p].str[i]);
      Log("%d到%d为16进制，计算结果为%u\n",p,q,val);
      return val;
      case TK_REG:
      for(int i=0;i<8;i++)
      {
        if(strcmp(tokens[p].str,regsl_c[i])==0)
        
        {val= reg_l(i);
        Log("%d到%d为reg_l计算结果为%u\n",p,q,val);
        return val;
        }
        if(strcmp(tokens[p].str,regsw_c[i])==0)
        {
          val= reg_w(i);
        Log("%d到%d为reg_w计算结果为%u\n",p,q,val);
        return val;
        }
        if(strcmp(tokens[p].str,regsb_c[i])==0)
        {val= reg_b(i);
        Log("%d到%d为reg_b计算结果为%u\n",p,q,val);
        return val;
        }
        
      }
      if(strcmp(tokens[p].str,"pc")==0)
      {
        Log("%d到%d为pc计算结果为%u\n",p,q,cpu.pc);
        return cpu.pc;
      }
      else
      {
        printf("错误的寄存器名字%s\n",tokens[p].str);
        assert(0);
      }
      default:
        printf("错误的tokentype%d",tokens[p].type);
        assert(0);

    }
    
  }
  else if(check_parentheses(p,q)==true)
  {uint32_t result=0;
     result=eval(p+1,q-1);
     Log("去括号，%d到%d的计算结果为%d\n",p+1,q-1,result);
     return result;
  }
 else
 {
  int op=find_dominant_operator(p,q);
  if(tokens[op].type==TK_DEREF)
  {
    uint32_t addr=eval(p+1,q);
    val=vaddr_read(addr,4);
    Log("%d到%d的计算结果为解引用结果，结果为%u\n",p,q,val);
    return val;
  }
  
  uint32_t val1=eval(p,op-1);
  Log("拆分，%d到%d的计算结果为%u\n",p,op-1,val1);
  uint32_t val2=eval(op+1,q);
  Log("拆分，%d到%d的计算结果为%u\n",op+1,q,val2);
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
    return val1||val2;
    case TK_EQ:
    return val1==val2;
    case TK_NEQ:
    return val1!=val2;
    default:
    printf("错误的运算符类型,%d\n",tokens[op].type);
    assert(0);
  }
 }
 return -1;
}
uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  if(tokens[0].type=='*')
  {
    tokens[0].type=TK_DEREF;
  }
  for(int i=1;i<nr_token;i++)
{
  if(
  tokens[i].type=='*'&&
  tokens[i-1].type!=TK_HEX&&
  tokens[i-1].type!=TK_DEC&&
  tokens[i-1].type!=TK_REG&&
  tokens[i-1].type!=')'
  )
  {
    tokens[i].type=TK_DEREF;
  }
}
 //printf("nr_token为%d\n",nr_token);
  return eval(0,nr_token-1);

  /* TODO: Insert codes to evaluate the expression. */

  return 0;
}
