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
    printf("arg is %s",arg);
    Log("Hello World from Nanos-lite for the %dth time!,arg is %s", j,arg);
    j ++;
    _yield();
  }
}

void init_proc() {

  // switch_boot_pcb();

  // Log("Initializing processes...");

  // // load program here
  // naive_uload(0, "/bin/bmptest");
  context_kload(&pcb[0], hello_fun, 1);
  context_kload(&pcb[1], hello_fun, 2);
  switch_boot_pcb();

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
    return current->cp;
}
