#include "nemu.h"
#define PTXSHFT 12 // Offset of PTX in a linear address
#define PDXSHFT 22 // Offset of PDX in a linear address
#define PDX(va) (((uint32_t)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va) (((uint32_t)(va) >> PTXSHFT) & 0x3ff)
#define OFF(va) ((uint32_t)(va)&0xfff)
#define VPAGE_NUM(va) ((uint32_t)(va) >> PTXSHFT)
#define PTE_ADDR(pte) ((uint32_t)(pte) & ~0xfff)

uint32_t page_translate(uint32_t addr)
{
// printf("addr:%x\n",addr);
uint32_t pdir = PDX(addr);
uint32_t ptab = PTX(addr);
uint32_t offset = OFF(addr);
// note that PDE is a struct, rather than a uint32_t in the nexus-am
PDE pde;
pde.val = paddr_read(PTE_ADDR(cpu.cr3.val) | (pdir << 2), 4);
// guarantee that the page table is present
assert(pde.present == 1);
PTE pte;
pte.val = paddr_read(PTE_ADDR(pde.val) | (ptab << 2), 4);
// guarantee that the page is present
if (pte.present == 0)
{
printf("addr:%x\n", addr);
}
assert(pte.present == 1);
uint32_t paddr = (PTE_ADDR(pte.val) | offset);
// printf("paddr:%x\n",paddr);
return paddr;
}
uint32_t isa_vaddr_read(vaddr_t addr, int len)
{
// return paddr_read(addr, len);
// now we need to handle the page mapping
if (cpu.cr0.paging == 0)
{
// no paging
return paddr_read(addr, len);
}
int start_page_num = addr / PAGE_SIZE;
int end_page_num = (addr + len - 1) / PAGE_SIZE;
if (start_page_num == end_page_num)
{
// the data is in one page
paddr_t paddr = page_translate(addr);
return paddr_read(paddr, len);
}
else
{
printf("the data is in two pages\n");
assert(0);
}
}
void isa_vaddr_write(vaddr_t addr, uint32_t data, int len)
{
if (cpu.cr0.paging == 0)
{
// no paging
paddr_write(addr, data, len);
return;
}
int start_page_num = addr / PAGE_SIZE;
int end_page_num = (addr + len - 1) / PAGE_SIZE;
if (start_page_num == end_page_num)
{
// the data is in one page
paddr_t paddr = page_translate(addr);
paddr_write(paddr, data, len);
}
else
{
printf("the data is in two pages\n");
assert(0);
}
}