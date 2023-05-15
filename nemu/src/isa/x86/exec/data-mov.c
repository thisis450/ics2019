#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push)
{
	rtl_push(&id_dest->val);
	print_asm_template1(push);
  //TODO();
}

make_EHelper(pop) {
	rtl_pop(&s0);
	operand_write(id_dest,&s0);
	print_asm_template1(pop);
  //TODO();
}

make_EHelper(pusha) {
  //TODO();
  rtl_mv(&s0,&cpu.esp);
	rtl_push(&cpu.eax);
	rtl_push(&cpu.ecx);
	rtl_push(&cpu.edx);
	rtl_push(&cpu.ebx);
	rtl_push(&s0);
	rtl_push(&cpu.ebp);
	rtl_push(&cpu.esi);
	rtl_push(&cpu.edi);
	print_asm("pusha");
}

make_EHelper(popa) {
  //TODO();
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&s0);
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);
  print_asm("popa");
}

make_EHelper(leave) {
  //TODO();
  rtl_mv(&cpu.esp,&cpu.ebp);
  rtl_pop(&cpu.ebp);
  print_asm("leave");
}

make_EHelper(cltd) {
	if (decinfo.isa.is_operand_size_16) {
		// TODO();
    rtl_sext(&s0, &reg_l(R_EAX), 2);
		rtl_sari(&s0, &s0, 16);
		rtl_sr(R_DX, &s0, 2);
  }
  else {
    // TODO();
		rtl_sari(&reg_l(R_EDX), &reg_l(R_EAX), 31);
  }
  print_asm(decinfo.isa.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
	if (decinfo.isa.is_operand_size_16) {
    // TODO();
		rtl_lr(&s0, R_AX, 1);
		rtl_sext(&s0, &s0, 1);
		rtl_sr(R_AX, &s0, 1);
  }
  else {
    // TODO();
		rtl_sext(&reg_l(R_EAX), &reg_l(R_EAX), 2);
  }
  print_asm(decinfo.isa.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decinfo.isa.is_operand_size_16 ? 2 : 4;
  rtl_sext(&s0, &id_src->val, id_src->width);
  operand_write(id_dest, &s0);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decinfo.isa.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  operand_write(id_dest, &id_src->addr);
  print_asm_template2(lea);
}
