#include <common.h>
#include "syscall.h"

int sys_write(intptr_t *buf, size_t count){
      for (int i = 0; i < count; i++) {
    putch(*((char*)buf + i));
  }
   return count;
  
}
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1; //#define GPR1 gpr[17] // a7
  intptr_t ret=0;
 printf("执行到do_syscall,此时根据c->GPR1的值来判断属于哪个系统调用 c->GPR1=a7=%d\n",a[0]);
  switch (a[0]) {
    case SYS_exit:
        c->GPRx=0;
        halt(c->GPRx);
    case SYS_yield:
        yield(); 
    //case SYS_write:ret=sys_write(c->GPR2,c->GPR3);break;
    case SYS_brk:
        c->GPRx=0;
        break;  
    case SYS_open:
        ret = fs_open((const char *)c->GPR2, c->GPR3, c->GPR4);
        Log("fs_open(%s, %d, %d) = %d",(const char *)c->GPR2, c->GPR3, c->GPR4, ret);
        break;
    case SYS_read:
        ret = fs_read(c->GPR2, (void *)c->GPR3, (size_t)c->GPR4);
        Log("fs_read(%d, %p, %d) = %d", c->GPR2, c->GPR3, c->GPR4, ret);
        break;
    case SYS_close:
        ret = fs_close(c->GPR2);
        Log("fs_close(%d, %d, %d) = %d", c->GPR2, c->GPR3, c->GPR4, ret);
        break;
    case SYS_write:
        ret = fs_write(c->GPR2, (void *)c->GPR3, (size_t)c->GPR4);
        Log("fs_write(%d, %p, %d) = %d", c->GPR2, c->GPR3, c->GPR4, ret);
        break;
    case SYS_lseek:
        ret = fs_lseek(c->GPR2, (size_t)c->GPR3, c->GPR4);
        Log("fs_lseek(%d, %d, %d) = %d", c->GPR2, c->GPR3, c->GPR4, ret);
        break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  c->GPRx=ret;
}
