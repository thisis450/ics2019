#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);
// call & ret
make_EHelper(ret);
make_EHelper(call);
make_EHelper(call_rm);
// data-mov
make_EHelper(push);
make_EHelper(pop);
make_EHelper(pusha);
make_EHelper(popa);
make_EHelper(leave);
make_EHelper(lea);
make_EHelper(movzx);
make_EHelper(movsx);
make_EHelper(cwtl);
make_EHelper(cltd);
// special
make_EHelper(nop);
// logic
make_EHelper(test);
make_EHelper(and);
make_EHelper(xor);
make_EHelper(or);
make_EHelper(sar);
make_EHelper(shl);
make_EHelper(shr);
make_EHelper(setcc);
make_EHelper(not);
// arith
make_EHelper(add);
make_EHelper(sub);
make_EHelper(cmp);
make_EHelper(inc);
make_EHelper(dec);
make_EHelper(neg);
make_EHelper(adc);
make_EHelper(sbb);
make_EHelper(mul);
make_EHelper(imul1);
make_EHelper(imul2);
make_EHelper(imul3);
make_EHelper(div);
make_EHelper(idiv);
// control
make_EHelper(jcc);
make_EHelper(jmp);
make_EHelper(jmp_rm);
//system
make_EHelper(in);
make_EHelper(out);
