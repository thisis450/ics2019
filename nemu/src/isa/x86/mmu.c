#include "nemu.h"
#define PTXSHFT        12      // Offset of PTX in a linear address
#define PDXSHFT        22      // Offset of PDX in a linear address
#define PDX(va)          (((uint32_t)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va)          (((uint32_t)(va) >> PTXSHFT) & 0x3ff)
#define OFF(va)          ((uint32_t)(va) & 0xfff)
#define ROUNDUP(a, sz)   ((((uintptr_t)a)+(sz)-1) & ~((sz)-1))
#define ROUNDDOWN(a, sz) ((((uintptr_t)a)) & ~((sz)-1))
#define PTE_ADDR(pte)    ((uint32_t)(pte) & ~0xfff)
#define PGADDR(d, t, o)  ((uint32_t)((d) << PDXSHFT | (t) << PTXSHFT | (o)))
uint32_t page_translate(uint32_t addr)//am.h,vme.c,mmu.c
{
	uint32_t Dir=(addr>>22),Page=((addr>>12)&1023),Offset=(addr&0xfff);
	PDE pde;
	pde.val=paddr_read(PTE_ADDR(cpu.cr3.val)|(Dir<<2),4);
	if (pde.present==0) assert(0);
	PTE pte;
	pte.val=paddr_read(PTE_ADDR(pde.val)|(Page<<2),4);
	if (pte.present==0) assert(0);
	uint32_t ret=(PTE_ADDR(pte.val)|Offset);
	return ret;
}
uint32_t isa_vaddr_read(vaddr_t addr, int len) {
	if (cpu.cr0.paging==0)
		return paddr_read(addr, len);
	int s=addr/PAGE_SIZE,t=(addr+len-1)/PAGE_SIZE;
	if (s!=t)
	{
		uint32_t ret=0;
		for (int i=len-1;i>=0;--i) ret=(ret<<8)|paddr_read(page_translate(addr+i),1);
		return ret;
	}
	else
	{
		paddr_t paddr = page_translate(addr);
        return paddr_read(paddr, len);
	}
	return 0;
}

void isa_vaddr_write(vaddr_t addr, uint32_t data, int len) {

	if (cpu.cr0.paging==0)
	{
		paddr_write(addr, data, len);
		return;
	}
	int s=addr/PAGE_SIZE,t=(addr+len-1)/PAGE_SIZE;
	if (s!=t)
	{
		uint32_t tmp;
		for (int i=len-1;i>=0;--i)
			tmp=((data>>(i*8))&0xff),
			paddr_write(page_translate(addr+i),tmp,1);

	}
	else
	{
		paddr_t paddr = page_translate(addr);
		paddr_write(paddr,data,len);
	}
}

