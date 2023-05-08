#include "cpu/exec.h"

make_EHelper(lidt) {
//   TODO();

    rtl_li(&t0,id_dest->addr);
    rtl_li(&cpu.idtr.limit, vaddr_read(t0,2));
    rtl_li(&cpu.idtr.base, vaddr_read(t0+2,4));
    print_asm_template1(lidt);

}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

  difftest_skip_ref();
}

make_EHelper(int) {
	raise_intr(id_dest->val, decinfo.seq_pc);
	print_asm("int %s", id_dest->str);

	difftest_skip_dut(1, 2);
}

make_EHelper(iret) {
	rtl_pop(&decinfo.jmp_pc);
	rtl_pop(&cpu.cs);
	rtl_pop(&cpu.eflags.val);

	rtl_j(decinfo.jmp_pc);
	print_asm("iret");
}

uint32_t pio_read_l(ioaddr_t);
uint32_t pio_read_w(ioaddr_t);
uint32_t pio_read_b(ioaddr_t);
void pio_write_l(ioaddr_t, uint32_t);
void pio_write_w(ioaddr_t, uint32_t);
void pio_write_b(ioaddr_t, uint32_t);

make_EHelper(in) {
  //TODO();
  if (id_dest->width==4) 
  {s0=pio_read_l(id_src->val);
  }
  else if (id_dest->width==2) 
  {s0=pio_read_w(id_src->val);
  }
  else 
  {s0=pio_read_b(id_src->val);
  }
  operand_write(id_dest,&s0);
  print_asm_template2(in);
}

make_EHelper(out) {
  //TODO();
	if (id_dest->width==4)  {pio_write_l(id_dest->val,id_src->val);}
	else if (id_dest->width==2)  {pio_write_w(id_dest->val,id_src->val);}
	else if (id_dest->width==1) {pio_write_b(id_dest->val,id_src->val);}
	else assert(0);
	print_asm_template2(out);
}
