#include <common.h>
#include "syscall.h"
/*enum {
  SYS_exit,
  SYS_yield,
  SYS_open,
  SYS_read,
  SYS_write,
  SYS_kill,
  SYS_getpid,
  SYS_close,
  SYS_lseek,
  SYS_brk,
  SYS_fstat,
  SYS_time,
  SYS_signal,
  SYS_execve,
  SYS_fork,
  SYS_link,
  SYS_unlink,
  SYS_wait,
  SYS_times,
  SYS_gettimeofday
};*/
SYS_write(intptr_t *buf, size_t count,intptr_t *ret){
      for (int i = 0; i < count; i++) {
    putch(*((char*)buf + i));
  }
  ret=count;
  return count;
}
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1; //#define GPR1 gpr[17] // a7
 printf("执行到do_syscall,此时根据c->GPR1的值来判断属于哪个系统调用 c->GPR1=a7=%d\n",a[0]);
  switch (a[0]) {
    case 0:c->GPRx=0;printf("SYS_exit， do_syscall此时 c->GPRx=%d\n",c->GPRx);halt(c->GPRx);//SYS_exit系统调用
    case 1:printf("SYS_yield， do_syscall此时c->GPRx=%d\n",c->GPRx);yield(); //SYS_yield系统调用
    case 4:return SYS_write(c->GPR2,c->GPR3,c->GPRx);
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
