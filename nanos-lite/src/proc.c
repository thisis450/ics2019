#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite for the %dth time!,%s", j,(char *)arg);
    j ++;
    _yield();
  }
}
char *arg[] = {"/bin/pal", "--skip"};
void init_proc() {

  // switch_boot_pcb();

  // Log("Initializing processes...");

  // // load program here
  // naive_uload(0, "/bin/bmptest");


  context_kload(&pcb[0], hello_fun, "kernel thread 1");
  context_kload(&pcb[1], hello_fun, "kernel thread 2");


  // context_kload(&pcb[0], hello_fun, "kernel thread 233");
  // context_uload(&pcb[1], "/bin/pal", 2, arg, NULL);
  // switch_boot_pcb();
  
naive_uload(NULL, "/bin/dummy");
}

_Context* schedule(_Context *prev) {

    current->cp = prev;
    if(current==&pcb[0])
    {
      current=&pcb[1];
    }
    else
    {
    current=&pcb[0];
    }
    //current = &pcb[0];
    return current->cp;
}
