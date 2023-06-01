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

static uintptr_t loader(PCB *pcb, const char *filename) {

	int fd=fs_open(filename);
	Elf_Phdr head_pro;
	Elf_Ehdr head_elf;
	fs_read(fd,&head_elf,sizeof(Elf_Ehdr));
	void *pa;
	size_t siz=head_elf.e_phentsize,cnt=head_elf.e_phnum;
	uintptr_t now_v,v_addr;
	size_t page_num;
	for (int i=0;i<cnt;++i)
	{
		fs_lseek(fd,i*siz+head_elf.e_phoff,0);
		fs_read(fd,&head_pro,siz);
		if (head_pro.p_type!=PT_LOAD) continue;
		fs_lseek(fd,head_pro.p_offset,0);
		v_addr=head_pro.p_vaddr;

		page_num=(head_pro.p_filesz-1)/PGSIZE+1;
		for (int j=0;j<page_num;++j)
		{
			pa=new_page(1);
			if ((uintptr_t)pa&0xfff) assert(0);
			_map(&(pcb->as),(void*)(head_pro.p_vaddr+j*PGSIZE),pa,0);

			if (j<page_num-1) fs_read(fd,pa,PGSIZE);
			else fs_read(fd,pa,head_pro.p_filesz-PGSIZE*j);
		}

		v_addr+=page_num*PGSIZE;
		if (head_pro.p_filesz==head_pro.p_memsz) {pcb->max_brk=v_addr;continue;}
		
		int zero_len=head_pro.p_memsz-head_pro.p_filesz;
		if (zero_len<PGSIZE*page_num-head_pro.p_filesz)
			memset((void*)(((uintptr_t)pa)+(head_pro.p_filesz-PGSIZE*(page_num-1))),0,zero_len);
		else
		{
			memset((void*)(((uintptr_t)pa)+(head_pro.p_filesz-PGSIZE*(page_num-1))),0,PGSIZE*page_num-head_pro.p_filesz);
			zero_len-=(PGSIZE*page_num-head_pro.p_filesz);
			now_v=head_pro.p_vaddr+page_num*PGSIZE;
			page_num=(zero_len-1)/PGSIZE+1;
			for (int j=0;j<page_num;++j)
			{
				pa=new_page(1);
				_map(&(pcb->as),(void*)(now_v),pa,0);
				if ((uintptr_t)pa&0xfff) assert(0);
				if (j<page_num-1) memset(pa,0,PGSIZE);
				else memset(pa,0,zero_len);
				now_v+=PGSIZE;
				zero_len-=PGSIZE;
			}
			v_addr+=page_num*PGSIZE;
		}
		pcb->max_brk=v_addr;

	}
	fs_close(fd);

	return head_elf.e_entry;

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
