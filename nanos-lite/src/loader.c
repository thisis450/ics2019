#include "proc.h"
#include <elf.h>

#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  //TODO();
	int fd = fs_open(filename, 0, 0);
	Elf_Ehdr elf_ehdr;
	fs_lseek(fd, 0, SEEK_SET);
	fs_read(fd, (void *)&elf_ehdr, sizeof(Elf_Ehdr));
	if (elf_ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
			elf_ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
			elf_ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
			elf_ehdr.e_ident[EI_MAG3] != ELFMAG3){
		Log("ELF format error in ramdisk\n");
		assert(0);
	}
	Elf_Phdr elf_phdr;
	for (uint16_t i = 0; i < elf_ehdr.e_phnum; i++){
		fs_lseek(fd, elf_ehdr.e_phoff + i * elf_ehdr.e_phentsize, SEEK_SET);
		fs_read(fd, (void *)&elf_phdr, sizeof(Elf_Phdr));
		Log("segment p_offset from phdr: 0x%x, from ehdr: 0x%x\n", elf_phdr.p_offset, elf_ehdr.e_phoff + i * elf_ehdr.e_phentsize);
		Log("current type: %d\n", elf_phdr.p_type);
		if (elf_phdr.p_type != PT_LOAD){
			continue;
		}
		fs_lseek(fd, elf_phdr.p_offset, SEEK_SET);
		fs_read(fd, (void *)elf_phdr.p_vaddr, elf_phdr.p_filesz);
		if (elf_phdr.p_memsz > elf_phdr.p_filesz){
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
