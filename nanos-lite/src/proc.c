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
    Log("Hello World from Nanos-lite for the %dth time!", j);
    j ++;
    _yield();
  }
}

void init_proc() {

  // switch_boot_pcb();

  // Log("Initializing processes...");

  // // load program here
  // naive_uload(0, "/bin/bmptest");
  context_kload(&pcb[0], hello_fun, NULL);
  switch_boot_pcb();

}

_Context* schedule(_Context *prev) {

    current->cp = prev;
    current=&pcb[0];
    return current->cp;
}
