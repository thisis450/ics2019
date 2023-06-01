#include "common.h"
#include "syscall.h"
uint32_t sys_yield(){
	_yield();
	return 0;
}

uint32_t sys_exit(int state){
	_halt(state);
}
uint32_t sys_brk(uintptr_t brk, intptr_t increment)
{	mm_brk(brk,increment);
	return 0;
}

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
	a[2] = c->GPR3;
	a[3] = c->GPR4;
uintptr_t res = 0;
  switch (a[0]) {
		case SYS_yield: 
		//Log("sys_yield \n");
		res = sys_yield(); break;
		case SYS_exit: 
		//Log("sys_exit \n");
		res = sys_exit(a[1]); break;
		case SYS_brk:
		//Log("sys_brk \n"); 
		res=sys_brk(a[1],a[2]); break;
		case SYS_open: 
		//Log("sys_open \n");
		res = fs_open((const char *)a[1], a[2]); break;
		case SYS_read: 
		//Log("sys_read \n");
		res = fs_read(a[1], (void *)a[2], a[3]); break;
		case SYS_write: 
		//Log("sys_write \n");
		res = fs_write((int)a[1],(const void *)a[2],(size_t)a[3]); break;
		case SYS_close: 
		//Log("sys_close \n");
		res = fs_close(a[1]); break;
		case SYS_lseek: 
		//Log("sys_lseek \n");
		res = fs_lseek(a[1], a[2], a[3]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
c->GPRx = res;
  return NULL;
}
