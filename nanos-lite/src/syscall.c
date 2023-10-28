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

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1; //#define GPR1 gpr[17] // a7
 printf("执行到do_syscall c->GPR1=a7=%d\n",a[0]);
  switch (a[0]) {
    case 0:printf("do_syscall此时 c->GPRx=%d\n",c->GPRx);halt(a[0]);
    case 1:c->GPRx=0;printf("do_syscall此时c->GPRx=%d\n",c->GPRx);yield(); //SYS_yield系统调用
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
