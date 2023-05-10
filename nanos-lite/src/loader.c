#include "proc.h"
#include <elf.h>
#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename)
{
  //TODO();
  unsigned char buf[2048];
  Elf_Ehdr elf_head;
  ramdisk_read(&elf_head, 0, sizeof(Elf_Ehdr));
  Elf_Phdr elf_phentry;
  for (int i = elf_head.e_phoff; i < elf_head.e_phnum * elf_head.e_phentsize + elf_head.e_phoff; i += elf_head.e_phentsize)
  {
    ramdisk_read(&elf_phentry, i, sizeof(Elf_Phdr));
    if (elf_phentry.p_type == PT_LOAD)
    {
      int len = 0;
      size_t file_off = elf_phentry.p_offset;
      unsigned char *mem_addr = (unsigned char *)elf_phentry.p_vaddr;
      while (len < elf_phentry.p_filesz)
      {
        int mov_size = (elf_phentry.p_filesz - len > 2048 ? 2048 : elf_phentry.p_filesz - len);
        ramdisk_read(buf, file_off, mov_size);
        memcpy(mem_addr, buf, mov_size);
        file_off += mov_size;
        mem_addr += mov_size;
        len += mov_size;
      }
      memset(mem_addr, 0, elf_phentry.p_memsz - len);
    }
  }
  // fs_close(fd);
  return elf_head.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %x", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
