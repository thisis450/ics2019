#include "rtl/rtl.h"

void raise_intr(uint32_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
	/* 1. push eflags, cs, eip */
	// printf("syscall NO: 0x%x and ret-addr: 0x%x\n", NO, ret_addr);
	rtl_push(&cpu.eflags.eflags_32);
	rtl_push(&cpu.cs);
	rtl_push(&ret	_addr);

	/* 2 3. get IDT base addr and entry from IDTR */
	vaddr_t idt_entry = cpu.idtr.base + NO * IDT_ENTRY_SZ;
	// printf("IDT entry address: 0x%x\n", idt_entry);

	/* 4. generate exception entry addr */
	vaddr_t ent_hi = vaddr_read(idt_entry + IDT_ENTRY_HI_OFF, 4) & 0xffff0000;
	vaddr_t ent_lo = vaddr_read(idt_entry, 2) & 0xffff;
	// printf("Handler target address: 0x%x\n", ent_hi | ent_lo);

	/* 5. jump */
	rtl_j(ent_hi | ent_lo);
}

bool isa_query_intr(void) {
  return false;
}
