#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
  char* arg=strtok(args," ");
  uint64_t N=0;
  if(arg==NULL)
  {
    N=1;
  }
  else{
    N=atoi(arg);

  }
cpu_exec(N);
  return 0;
}

static int cmd_info(char *args) {
  char* arg=strtok(args," ");
  if(arg==NULL)
  {
    printf("cmd_info没有参数\n");
    return 0;
  }
  if (strcmp(arg,"r")==0)
  {
        printf("eax is %x\n",cpu.eax);
        printf("ecx is %x\n",cpu.ecx);
        printf("edx is %x\n",cpu.edx);
        printf("ebx is %x\n",cpu.ebx);
        printf("esp is %x\n",cpu.esp);
        printf("ebp is %x\n",cpu.ebp); 
        printf("esi is %x\n",cpu.esi);
        printf("edi is %x\n",cpu.edi);
        printf("pc is %x\n",cpu.pc);
        return 0;
  }
  if(strcmp(arg,"w")==0)
  {
    print_wp();
    return 0;
  }
  printf("info命令未输入正确参数\n");
  return 0;
}

static int cmd_p(char *args) {
  if(args==NULL)
  {
    printf("表达式求解缺少参数\n");
    return 0;
  }
  bool success=true;
  //char*arg=strtok(args," ");
  uint32_t val=expr(args,&success);
  if(success==false)
  {
    return 0;
  }
  printf("%u\n",val);
  return 0;
}

static int cmd_x(char *args) {
  if(args==NULL)
  {
    printf("扫描内存缺少参数\n");
    return 0;
  }
  int arglen=strlen(args);
  char *argN=strtok(args," ");
  if(argN==NULL)
    {
    printf("扫描内存缺少参数\n");
    return 0;
  }
  int N=atoi(argN);
  if(N<=0)
  {
    printf("错误的参数N\n");
    return 0;
  }
  //printf("N的值为%d\n",N);
  char *exprs=args+strlen(argN)+1;
  if(exprs>=args+arglen)
  {
    printf("没有输入表达式\n");
    return 0;
  }
  //printf("expr为%s\n",expr);
  bool success=true;
  vaddr_t expr_result=expr(exprs,&success);
  //vaddr_t expr_result=atoi(expr);
  if(success==false)
  {
    printf("表达式求解过程中出现了错误\n");
    return 0;
  }
  printf("Memory\n");
  for(int i=0;i<N;i++)
  {
    
    uint32_t data = vaddr_read(expr_result + i * 4,4);
        printf("0x%08x  " , expr_result + i * 4 );
        for(int j =0 ; j < 4 ; j++){
            printf("0x%02x " , data & 0xff);
            data = data >> 8 ;
        }
        printf("\n");

    
  }
  return 0;
}

static int cmd_w(char *args) {
    if(args==NULL)
  {
    printf("插入检查点缺少参数\n");
    return 0;
  }
  WP* temp=new_wp(args);
  if(temp!=NULL)
  {
    printf("插入检查点成功，表达式为%s,目前值为%u，编号为%d\n",temp->exp,temp->old_val,temp->NO);
  }
  else
  {
    printf("检查点插入失败\n");
  }
  return 0;
}

static int cmd_d(char *args) {
      if(args==NULL)
  {
    printf("删除检查点缺少参数\n");
    return 0;
  }
  char*arg=strtok(args," ");
  int N=atoi(arg);
  free_wp(N);
  return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "格式 si [N] 使用举例 si 10 让程序单步执行N条指令后暂停执行,当N没有给出时, 缺省为1",cmd_si},
  { "info", "打印程序状态 格式 info SUBCMD 使用举例 info r打印寄存器状态info w打印监视点信息", cmd_info },
  { "p", "表达式求值 格式 p EXPR 举例：p $eax + 1 求出表达式EXPR的值, 将结果作为起始内存地址, 以十六进制形式输出连续的N个4字节", cmd_p },
  { "x", "扫描内存 格式 x N EXPR 使用举例	x 10 $esp 求出表达式EXPR的值, 将结果作为起始内存地址, 以十六进制形式输出连续的N个4字节", cmd_x },
  { "w", "设置监视点	格式 w EXPR 举例：w *0x2000 当表达式EXPR的值发生变化时, 暂停程序执行", cmd_w },
  { "d", "删除监视点  格式 d N 举例：d 2 删除序号为N的监视点备注", cmd_d },

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
