#include "proc.h"
#include <elf.h>
#include"fs.h"
#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif
#define PAGE_SIZE 0x1000
Elf_Phdr pHeaders[20];
// static uintptr_t loader(PCB *pcb, const char *filename)
// {
//   //TODO();
//   unsigned char buf[2048];
//   Elf_Ehdr elf_head;
//   ramdisk_read(&elf_head, 0, sizeof(Elf_Ehdr));
//   Elf_Phdr elf_phentry;
//   for (int i = elf_head.e_phoff; i < elf_head.e_phnum * elf_head.e_phentsize + elf_head.e_phoff; i += elf_head.e_phentsize)
//   {
//     ramdisk_read(&elf_phentry, i, sizeof(Elf_Phdr));
//     if (elf_phentry.p_type == PT_LOAD)
//     {
//       int len = 0;
//       size_t file_off = elf_phentry.p_offset;
//       unsigned char *mem_addr = (unsigned char *)elf_phentry.p_vaddr;
//       while (len < elf_phentry.p_filesz)
//       {
//         int mov_size = (elf_phentry.p_filesz - len > 2048 ? 2048 : elf_phentry.p_filesz - len);
//         ramdisk_read(buf, file_off, mov_size);
//         memcpy(mem_addr, buf, mov_size);
//         file_off += mov_size;
//         mem_addr += mov_size;
//         len += mov_size;
//       }
//       memset(mem_addr, 0, elf_phentry.p_memsz - len);
//     }
//   }
//   // fs_close(fd);
//   return elf_head.e_entry;
// }
// static uintptr_t loader(PCB *pcb, const char *filename)
// {
//   //TODO();
//   int fd = fs_open(filename, 0, 0);
//   unsigned char buf[2048];
//   Elf_Ehdr elf_head;
//   fs_read(fd, &elf_head, sizeof(Elf_Ehdr));
//   // ramdisk_read(&elf_head, 0, sizeof(Elf_Ehdr));
//   Elf_Phdr elf_phentry;
//   for (int i = 0; i < elf_head.e_phnum; i++)
//   {
//     fs_lseek(fd, elf_head.e_phoff + i * elf_head.e_phentsize, SEEK_SET);
//     fs_read(fd, &elf_phentry, elf_head.e_phentsize);
//     // ramdisk_read(&elf_phentry, i, sizeof(Elf_Phdr));
//     if (elf_phentry.p_type == PT_LOAD)
//     {
//       int len = 0;
//       size_t file_off = elf_phentry.p_offset;
//       unsigned char *mem_addr = (unsigned char *)elf_phentry.p_vaddr;
//       while (len < elf_phentry.p_filesz)
//       {
//         int mov_size = (elf_phentry.p_filesz - len > 2048 ? 2048 : elf_phentry.p_filesz - len);
//         fs_lseek(fd, file_off, SEEK_SET);
//         fs_read(fd, buf, mov_size);
//         // ramdisk_read(buf, file_off, mov_size);
//         memcpy(mem_addr, buf, mov_size);
//         file_off += mov_size;
//         mem_addr += mov_size;
//         len += mov_size;
//       }
//       memset(mem_addr, 0, elf_phentry.p_memsz - len);
//     }
//   }
//   fs_close(fd);
//   return elf_head.e_entry;
// }

#define ROUNDUP(a, sz) ((((uintptr_t)a) + (sz)-1) & ~((sz)-1))
static uintptr_t loader(PCB *pcb, const char *filename)
{
  const int page_size = 4096;
  int fd = fs_open(filename, 0, 0);
  Elf_Ehdr elf_head;
  fs_read(fd, &elf_head, sizeof(Elf_Ehdr));
  Elf_Phdr elf_phentry;
  for (int i = 0; i < elf_head.e_phnum; i++)
  {
    fs_lseek(fd, elf_head.e_phoff + i * elf_head.e_phentsize, SEEK_SET);
    fs_read(fd, &elf_phentry, elf_head.e_phentsize);
    if (elf_phentry.p_type == PT_LOAD)
    {
      int len = 0;
      size_t file_off = elf_phentry.p_offset;
      fs_lseek(fd, file_off, SEEK_SET);
      unsigned char *v_addr = (unsigned char *)elf_phentry.p_vaddr;
      void *p_addr = 0;
      while (len < elf_phentry.p_filesz)
      {
        int mov_size = (elf_phentry.p_filesz - len > page_size ? page_size
                                                               : elf_phentry.p_filesz - len);
        int gap = page_size - ((uint32_t)v_addr & 0xfff);
        if (mov_size > gap)
          mov_size = gap;
        // 原来的框架写的还挺方便
        p_addr = new_page(1);
        p_addr = (void *)((uint32_t)p_addr | ((uint32_t)v_addr & 0xfff));
        _map(&pcb->as, v_addr, p_addr, _PROT_EXEC);
        fs_read(fd, p_addr, mov_size);
        v_addr += mov_size;
        p_addr += mov_size;
        len += mov_size;
      }
      int gap = page_size - ((uint32_t)v_addr & 0xfff);
      if (gap != page_size)
      {
        memset(p_addr, 0, gap);
        v_addr += gap;
        len += gap;
      }
      while (len < elf_phentry.p_memsz)
      {
        int mov_size = (elf_phentry.p_memsz - len > page_size ? page_size
                                                              : elf_phentry.p_memsz - len);
        p_addr = new_page(1);
        _map(&pcb->as, v_addr, p_addr, _PROT_EXEC);
        memset(p_addr, 0, mov_size);
        v_addr += mov_size;
        len += mov_size;
      }
      // max_brk应该是最后一个段吧
      pcb->max_brk = pcb->max_brk > ROUNDUP(v_addr, page_size) ? pcb->max_brk : ROUNDUP(v_addr, page_size);
    }
  }
  fs_close(fd);
  return elf_head.e_entry;
}


void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %x", entry);
  ((void(*)())entry) ();//Log("4396");
}




void context_kload(PCB *pcb, void *entry,void*arg) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(stack, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, int argc, char *const argv[], char *const envp[])
{
  _protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, argc, argv, envp);

}
