#include "rtl/rtl.h"
#define IRQ_TIMER 32


void raise_intr(uint32_t NO, vaddr_t ret_addr) {
/* TODO: Trigger an interrupt/exception with ``NO''.
* That is, use ``NO'' to index the IDT.
*/
// step1
rtl_push(&cpu.eflags.val);
rtl_push(&cpu.cs);
rtl_push(&ret_addr);
cpu.eflags.IF = 0;
// step2
uint32_t gate_addr = cpu.idtr.base, len = cpu.idtr.limit;
if (len <= NO)
{
printf("the number is larger than the length of IDT!\n");
assert(0);
}
//step3
uint32_t val_l, val_h, p;
val_l = vaddr_read(gate_addr + NO * 8, 2);
val_h = vaddr_read(gate_addr + NO * 8 + 6, 2);
p = vaddr_read(gate_addr + NO * 8 + 5, 1) >> 7;
//actually no need to check p for NEMU, but we can do it.
if (!p)
{
printf("The gatedesc is not allowed!");
assert(0);
}
//step4
//using rtl api
vaddr_t goal = (val_h << 16) + val_l;
rtl_j(goal);
}

bool isa_query_intr(void) {
  if (cpu.INTR == true && cpu.eflags.IF == 1)
  {
    cpu.INTR = false;
    raise_intr(IRQ_TIMER, cpu.pc);
    return true;
  }
  return false;
}
