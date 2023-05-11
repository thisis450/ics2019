#include "proc.h"
#include <elf.h>
#include<fs.h>
#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

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
static uintptr_t loader(PCB *pcb, const char *filename)
{
	int fd = fs_open(filename, 0, 0);
	Elf_Ehdr elf_ehdr;
	/* init the file open_offset */
	fs_lseek(fd, 0, SEEK_SET);
	/* read elf-header from ramdisk */
	fs_read(fd, (void *)&elf_ehdr, sizeof(Elf_Ehdr));
	/* check format */
	if (elf_ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
			elf_ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
			elf_ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
			elf_ehdr.e_ident[EI_MAG3] != ELFMAG3){
		Log("ELF format error in ramdisk\n");
		assert(0);
	}
	/* read elf-program header from ramdisk */
	/* The program header table tells the system how to create a process image.
	 * It is found at file offset e_phoff, and consists of e_phnum entries, each with size e_phentsize.
	 */
	Elf_Phdr elf_phdr;
	/* find load-type program headers */
	for (uint16_t i = 0; i < elf_ehdr.e_phnum; i++){
		fs_lseek(fd, elf_ehdr.e_phoff + i * elf_ehdr.e_phentsize, SEEK_SET);
		fs_read(fd, (void *)&elf_phdr, sizeof(Elf_Phdr));
		Log("segment p_offset from phdr: 0x%x, from ehdr: 0x%x\n", elf_phdr.p_offset, elf_ehdr.e_phoff + i * elf_ehdr.e_phentsize);
		Log("current type: %d\n", elf_phdr.p_type);
		if (elf_phdr.p_type != PT_LOAD){
			continue;
		}
		/* load to memory */
		fs_lseek(fd, elf_phdr.p_offset, SEEK_SET);
		fs_read(fd, (void *)elf_phdr.p_vaddr, elf_phdr.p_filesz);
		if (elf_phdr.p_memsz > elf_phdr.p_filesz){
			/* set 0 */
			memset((void *)elf_phdr.p_vaddr + elf_phdr.p_filesz, 0, elf_phdr.p_memsz - elf_phdr.p_filesz);
		}
	}
	return elf_ehdr.e_entry;
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
