#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
PCB *front_p = NULL;
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


  // context_kload(&pcb[0], hello_fun, "kernel thread 233");
  // context_uload(&pcb[1], "/bin/pal", 2, arg, NULL);


  // context_kload(&pcb[0], hello_fun, "kernel thread 233");
  // context_uload(&pcb[1], "/bin/pal", 2, arg, NULL);
  // switch_boot_pcb();
  context_uload(&pcb[0], "/bin/pal", 2, arg, NULL);
  context_uload(&pcb[1], "/bin/pal", 2, arg, NULL);
  context_uload(&pcb[2], "/bin/pal", 2, arg, NULL);
  context_uload(&pcb[3], "/bin/hello", 1, arg, NULL);
  // context_kload(&pcb[1], hello_fun, "kernel thread 2");
  front_p = &pcb[0];
  switch_boot_pcb();
}

_Context* schedule(_Context *prev) {

    // current->cp = prev;
    // if(current==&pcb[0])
    // {
    //   current=&pcb[1];
    // }
    // else
    // {
    // current=&pcb[0];
    // }
    // // current = &pcb[0];
    // return current->cp;

  static int pcb1_times = 0;
  current->cp = prev;
  current = front_p;
  if (pcb1_times == 50)
  {
    current = &pcb[3];
    pcb1_times = 0;
  }
  pcb1_times++;
  return current->cp;



}
void change_front_program(int key_code)
{
  if (key_code >= 2 && key_code <= 4)
  {
    front_p = &pcb[key_code - 2];
  }
  return;
}