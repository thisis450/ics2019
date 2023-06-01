#include "nemu.h"

#define PTXSHFT 12 // Offset of PTX in a linear address
#define PDXSHFT 22 // Offset of PDX in a linear address
#define PDX(va) (((uint32_t)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va) (((uint32_t)(va) >> PTXSHFT) & 0x3ff)
#define OFF(va) ((uint32_t)(va)&0xfff)
#define VPAGE_NUM(va) ((uint32_t)(va) >> PTXSHFT)
#define PTE_ADDR(pte) ((uint32_t)(pte) & ~0xfff)

paddr_t page_translate(vaddr_t addr)
{
  uint32_t dir_index = PDX(addr);
  uint32_t tab_index = PTX(addr);
  paddr_t pgdir = cpu.cr3.val;
  PDE pde;
  pde.val = paddr_read(pgdir + dir_index * 4, 4);
  if (!pde.present) {
    Log("%x %x", addr, pde.val);
    assert(0);
  }
  PTE pte;
  pte.val = paddr_read((pde.val & ~0xfff) + tab_index * 4, 4);
  if (!pte.present)
    assert(0);
  return (PTE_ADDR(pte.val) | OFF(addr));
}

uint32_t isa_vaddr_read(vaddr_t addr, int len)
{
  if (!cpu.cr0.paging)
    return paddr_read(addr, len);
  uint32_t s_vpage_num = VPAGE_NUM(addr);
  uint32_t e_vpage_num = VPAGE_NUM(addr + len - 1);
  if (s_vpage_num == e_vpage_num)
    return paddr_read(page_translate(addr), len);
  else
  {
    uint32_t pre_len = 4096 - OFF(addr);
    uint32_t suc_len = len - pre_len;
    uint32_t res = 0;
    res |= paddr_read(page_translate(e_vpage_num << PTXSHFT), suc_len);
    res <<= (pre_len << 3);
    res |= paddr_read(page_translate(addr), pre_len);
    return res;
  }
  return paddr_read(addr, len);
}

void isa_vaddr_write(vaddr_t addr, uint32_t data, int len)
{
  if (!cpu.cr0.paging)
  {
    paddr_write(addr, data, len);
    return;
  }
  uint32_t s_vpage_num = VPAGE_NUM(addr);
  uint32_t e_vpage_num = VPAGE_NUM(addr + len - 1);
  if (s_vpage_num == e_vpage_num) // 没有出现跨页
    paddr_write(page_translate(addr), data, len);
  else
  {
    uint32_t pre_len = 4096 - OFF(addr);
    uint32_t suc_len = len - pre_len;
    paddr_write(page_translate(addr), data, pre_len);
    paddr_write(page_translate(e_vpage_num << PTXSHFT), (data >> (pre_len << 3)), suc_len);
  }
}
