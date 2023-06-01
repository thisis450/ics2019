#include <am.h>
#include <x86.h>
#include <nemu.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN = {};
static PTE kptabs[(PMEM_SIZE + MMIO_SIZE) / PGSIZE] PG_ALIGN = {};
static void* (*pgalloc_usr)(size_t) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static _Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE},
  {.start = (void*)MMIO_BASE,  .end = (void*)(MMIO_BASE + MMIO_SIZE)}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
  vme_enable = 1;

  return 0;
}

int _protect(_AddressSpace *as) {
  PDE *updir = (PDE*)(pgalloc_usr(1));
  as->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  return 0;
}

void _unprotect(_AddressSpace *as) {
}

static _AddressSpace *cur_as = NULL;
void __am_get_cur_as(_Context *c) {
  c->as = cur_as;
}

void __am_switch(_Context *c) {
  if (vme_enable) {
    set_cr3(c->as->ptr);
    cur_as = c->as;
  }
}

int _map(_AddressSpace *as, void *va, void *pa, int prot) {
	PDE* pde=as->ptr;
	int Dir=((uintptr_t)va)>>22,Page=((((uintptr_t)va)>>12)&0x3ff);
	PTE* pte;
	if (pde[Dir]&1)
		pte=(PTE*)(pde[Dir]&0xfffff000);
	else
	{
		pte=(PTE*)(((uintptr_t)pgalloc_usr(1))&0xfffff000);

		pde[Dir]=(((uintptr_t)pte)|1);
	}
	pte[Page]=((((uintptr_t)pa)&0xfffff000)|1);

  return 0;
}

_Context *_ucontext(_AddressSpace *as, _Area ustack, _Area kstack, void *entry, int argc, char *const argv[], char *const envp[])
{
  extern void memcpy(void *, const void *, int);
  memcpy(ustack.end - 12, (void *)(&argc), 4);
  memcpy(ustack.end - 8, (void *)(&argv), 4);
  memcpy(ustack.end - 4, (void *)(&envp), 4);

  _Context *new_context = ustack.end - 16 - sizeof(_Context);
  new_context->as = as;
  new_context->eip = (uintptr_t)entry;
  new_context->cs = 8;
  new_context->eflags = 0x202;
  return new_context;
}
