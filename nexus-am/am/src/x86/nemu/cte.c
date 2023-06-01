#include <am.h>
#include <x86.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void __am_irq0();
void __am_vecsys();
void __am_vectrap();
void __am_vecnull();

_Context* __am_irq_handle(_Context *c) {
  _Context *next = c;
  __am_get_cur_as(c);
  if (user_handler) {
    _Event ev = {0};
    switch (c->irq) {
      case 0x81:
				ev.event = _EVENT_YIELD;
			break;
      case 0x80:
				ev.event = _EVENT_SYSCALL;
				break;
      case 0x20:
				ev.event = _EVENT_IRQ_TIMER;
				break;
      default: ev.event = _EVENT_ERROR; break;
    }
    // printf("irq=%d\n", c->irq);
    // printf("eip=%x\n", c->eip);
    // printf("cs=%x\n", c->cs);
    // printf("eflags=%x\n", c->eflags);
    // printf("edi=%d\n", c->edi);
    // printf("esi=%x\n", c->esi);
    // printf("ebp=%x\n", c->ebp);
    // printf("esp=%x\n", c->esp);
    // printf("ebx=%x\n", c->ebx);
    // printf("edx=%x\n", c->edx);
    // printf("ecx=%x\n", c->ecx);
    // printf("eax=%x\n", c->eax);
    next = user_handler(ev, c);
    if (next == NULL) {
      next = c;
    }
  }
__am_switch(next);
  return next;
}

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  static GateDesc idt[NR_IRQ];

  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), __am_vecnull, DPL_KERN);
  }

  // ----------------------- interrupts ----------------------------
  idt[32]   = GATE(STS_IG32, KSEL(SEG_KCODE), __am_irq0,   DPL_KERN);
  // ---------------------- system call ----------------------------
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), __am_vecsys, DPL_USER);
  idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), __am_vectrap, DPL_KERN);

  set_idt(idt, sizeof(idt));

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {

_Context *c=(_Context*)(stack.end-sizeof(_Context)-8);
  void ** arg_stack=(void**)(stack.end-4);
  *arg_stack=arg;
// if(arg==0)
// {
//   printf("no such arg!\n");
// }
c->eip=(uintptr_t)entry;
c->eflags=0x2;
c->cs=8;
return c;
}

void _yield() {
  asm volatile("int $0x81");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
